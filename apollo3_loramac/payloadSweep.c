/*!
 * \file      payloadSweep.c
 *
 * \brief     Payload-size energy sweep.
 *
 *            For every application payload size from 0 to SWEEP_MAX_PAYLOAD_SIZE
 *            (EU868 DR3/SF9 limit = 115 bytes), SWEEP_REPS = 15 unconfirmed
 *            LoRaWAN uplinks are sent with dummy data.  The INA219 is sampled
 *            continuously at 10 ms intervals into a ring buffer.  For each
 *            transmission the buffer (PRE_SAMPLES of history + POST_SAMPLES
 *            covering TX + RX1 + RX2) is dumped as a JSON object over SWO.
 *
 *            IMPORTANT: dummy payload generation (memset) is intentionally done
 *            BEFORE arming the INA219 capture, so the CPU load of filling the
 *            buffer does not contaminate the current measurement.
 *
 *            LoRaWAN duty-cycle enforcement is disabled for this lab-only sweep.
 *            Do not use in a deployed device.
 */

#include "payloadSweep.h"
#include "ina219.h"
#include "deviceConfig.h"
#include "am_util_stdio.h"
#include "am_util_delay.h"
#include <string.h>

/* -------------------------------------------------------------------------- */
/* Configuration                                                               */
/* -------------------------------------------------------------------------- */

#ifndef LORAWAN_DEFAULT_CLASS
#define LORAWAN_DEFAULT_CLASS CLASS_A
#endif

/* EU868 DR3 (SF9 / BW125 kHz) application-payload maximum (bytes) */
#define SWEEP_MAX_PAYLOAD_SIZE  115
/* Number of repetitions per payload size */
#define SWEEP_REPS              10
/* Silent warm-up transmissions (size 0) discarded before the sweep begins */
#define SWEEP_WARMUP_REPS       3
/* LoRaWAN port used for dummy uplinks */
#define SWEEP_APP_PORT          2

/* -------------------------------------------------------------------------- */
/* INA219 ring-buffer capture                                                  */
/* -------------------------------------------------------------------------- */

/* Sampling period in milliseconds */
#define SAMPLE_INTERVAL_MS  1

/*
 * PRE_SAMPLES: ring-buffer history kept before each trigger.
 * 50 samples = 50 ms of baseline current before the TX starts.
 */
#define PRE_SAMPLES  50

/*
 * POST_SAMPLES: samples collected after arming the capture.
 * Only the TX air-time needs to be covered (RX windows are excluded from
 * energy and ToA calculation).
 *
 * Capture runs as fast as INA219 I2C allows (~200 µs/sample at 400 kHz).
 * Worst case TX (115-byte, SF9, BW125): ~680 ms → ~3400 samples.
 * 3500 samples gives a safe margin.
 */
#define POST_SAMPLES  3500

#define CAPTURE_MAX  (PRE_SAMPLES + POST_SAMPLES)

/* 32.768 kHz STIMER used for high-resolution timestamps */
#define STIMER_HZ  32768UL

/* Ring buffer */
static uint32_t CapTicks[CAPTURE_MAX];
static int32_t  CapCurrent_uA[CAPTURE_MAX];
static int32_t  CapShunt_uV[CAPTURE_MAX];
static int32_t  CapBus_mV[CAPTURE_MAX];
static uint16_t CapHead = 0;
static uint16_t CapFill = 0;

/* Capture control */
static bool             CaptureActive = false;
static uint16_t         PostRoll      = 0;
static volatile uint8_t IsDumpPending = 0;

/* Sweep state — written in main loop, read in DumpCaptureJson */
static uint8_t SweepPayloadSize = 0;
static uint8_t SweepRep        = 0;

/* Dummy payload buffer; filled before each capture window opens */
static uint8_t SweepDummyBuf[SWEEP_MAX_PAYLOAD_SIZE];

/* -------------------------------------------------------------------------- */
/* Sampling timer                                                              */
/* -------------------------------------------------------------------------- */

static TimerEvent_t     SampleTimer;
static volatile uint8_t IsSamplePending = 0;
static bool             Ina219Ready     = false;

static void OnSampleTimerEvent(void *context)
{
    TimerStop(&SampleTimer);
    IsSamplePending = 1;
    TimerSetValue(&SampleTimer, SAMPLE_INTERVAL_MS);
    TimerStart(&SampleTimer);
}

/* Stores one INA219 measurement into the ring buffer. */
static void StoreSample(float shuntVoltage_mV, float busVoltage_V, float current_uA)
{
    CapTicks[CapHead]      = am_hal_stimer_counter_get();
    CapCurrent_uA[CapHead] = (int32_t)(current_uA);
    CapShunt_uV[CapHead]   = (int32_t)(shuntVoltage_mV * 1000.0f);
    CapBus_mV[CapHead]     = (int32_t)(busVoltage_V * 1000.0f);
    CapHead = (CapHead + 1) % CAPTURE_MAX;
    if (CapFill < CAPTURE_MAX) CapFill++;
}

/* 1 ms timer-based sampling — used during idle to fill the PRE_SAMPLES buffer. */
static void SampleProcess(void)
{
    if (!IsSamplePending) return;
    IsSamplePending = 0;
    if (!Ina219Ready) return;

    float sh, bus, cur;
    if (INA219_GetMeasurements(&sh, &bus, &cur) != INA219_NO_ERROR) return;
    StoreSample(sh, bus, cur);
}

/*
 * Fast capture — called directly in the main loop without any delay.
 * Speed is limited by the INA219 I2C transaction time (~200 µs at 400 kHz),
 * giving ~5 kHz effective sample rate and ~0.2 ms ToA resolution.
 * Used only while CaptureActive is true.
 */
static void SampleFast(void)
{
    if (!Ina219Ready) return;

    float sh, bus, cur;
    if (INA219_GetMeasurements(&sh, &bus, &cur) != INA219_NO_ERROR) return;
    StoreSample(sh, bus, cur);

    if (PostRoll > 0) PostRoll--;
    if (PostRoll == 0) {
        CaptureActive = false;
        IsDumpPending = 1;
    }
}

/* -------------------------------------------------------------------------- */
/* Summary output (one line per transmission)                                  */
/* -------------------------------------------------------------------------- */

/* Current threshold (µA) that separates TX (~46 mA peak) from RX/idle (~4 mA) */
#define TX_DETECT_THRESHOLD_uA  25000

/*
 * Integrates I×V over the POST_SAMPLES window and prints one summary line:
 *   {"size":<N>,"rep":<R>,"energy_uJ":<E>,"toa_ms":<T>}
 *
 * ToA is measured from the current trace: first and last sample above
 * TX_DETECT_THRESHOLD_uA, resolution = SAMPLE_INTERVAL_MS.
 */
static void PrintSummary(void)
{
    uint16_t n      = CapFill;
    uint16_t oldest = (uint16_t)((CapHead + CAPTURE_MAX - n) % CAPTURE_MAX);

    int16_t  tx_start      = -1;
    int16_t  tx_end        = -1;
    uint32_t tx_start_tick = 0;
    uint32_t tx_end_tick   = 0;

    for (uint16_t k = PRE_SAMPLES; k < n; k++) {
        uint16_t idx = (uint16_t)((oldest + k) % CAPTURE_MAX);
        if (CapCurrent_uA[idx] >= TX_DETECT_THRESHOLD_uA) {
            if (tx_start < 0) { tx_start = (int16_t)k; tx_start_tick = CapTicks[idx]; }
            tx_end      = (int16_t)k;
            tx_end_tick = CapTicks[idx];
        }
    }

    /*
     * Integrate I×V over the TX window using the actual STIMER Δt between
     * consecutive samples — correct even with variable polling intervals.
     *   unit: µA × mV × ticks = nW × ticks
     *   ÷ STIMER_HZ → nJ
     *   ÷ 1000      → µJ
     */
    int64_t energy_sum = 0;
    if (tx_start >= 0) {
        for (int16_t k = tx_start; k < tx_end; k++) {
            uint16_t idx  = (uint16_t)((oldest + (uint16_t)k)      % CAPTURE_MAX);
            uint16_t idx1 = (uint16_t)((oldest + (uint16_t)(k + 1)) % CAPTURE_MAX);
            uint32_t dt   = CapTicks[idx1] - CapTicks[idx];
            energy_sum   += (int64_t)CapCurrent_uA[idx] * CapBus_mV[idx] * dt;
        }
    }
    int64_t  energy_uJ = energy_sum / ((int64_t)STIMER_HZ * 1000LL);
    uint32_t toa_ms    = (tx_start >= 0)
                         ? (uint32_t)((uint64_t)(tx_end_tick - tx_start_tick) * 1000ULL / STIMER_HZ)
                         : 0;

    am_util_stdio_printf(
        "{\"size\":%u,\"rep\":%u,\"energy_uJ\":%d,\"toa_ms\":%u}\n",
        (unsigned)SweepPayloadSize,
        (unsigned)SweepRep,
        (int)energy_uJ,
        (unsigned)toa_ms);

    IsDumpPending = 0;
}

/* -------------------------------------------------------------------------- */
/* LoRaWAN MAC boilerplate                                                     */
/* -------------------------------------------------------------------------- */

static volatile uint8_t IsMacProcessPending = 0;

static void OnMacProcessNotify(void) { IsMacProcessPending = 1; }

static void OnNvmDataChange(LmHandlerNvmContextStates_t state, uint16_t size) {}

static void OnNetworkParametersChange(CommissioningParams_t *params) {}

static void OnMacMcpsRequest(LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxIn) {}

static void OnMacMlmeRequest(LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxIn) {}

static void OnJoinRequest(LmHandlerJoinParams_t *params)
{
    if (params->Status == LORAMAC_HANDLER_ERROR) {
        LmHandlerJoin();
    } else {
        LmHandlerRequestClass(LORAWAN_DEFAULT_CLASS);
    }
}

static void OnTxData(LmHandlerTxParams_t *params)   {}
static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params) {}
static void OnClassChange(DeviceClass_t deviceClass) {}
static void OnBeaconStatusChange(LoRaMacHandlerBeaconParams_t *params) {}

#if(LMH_SYS_TIME_UPDATE_NEW_API == 1)
static void OnSysTimeUpdate(bool isSynchronized, int32_t timeCorrection) {}
#else
static void OnSysTimeUpdate(void) {}
#endif

/* Compliance callbacks — stubs; not relevant for a lab sweep */
static void OnTxPeriodicityChanged(uint32_t periodicity)             {}
static void OnTxFrameCtrlChanged(LmHandlerMsgTypes_t isTxConfirmed)  {}
static void OnPingSlotPeriodicityChanged(uint8_t pingSlotPeriodicity) {}

static LmHandlerCallbacks_t LmHandlerCallbacks = {
    .GetBatteryLevel          = BoardGetBatteryLevel,
    .GetTemperature           = NULL,
    .GetRandomSeed            = BoardGetRandomSeed,
    .OnMacProcess             = OnMacProcessNotify,
    .OnNvmDataChange          = OnNvmDataChange,
    .OnNetworkParametersChange= OnNetworkParametersChange,
    .OnMacMcpsRequest         = OnMacMcpsRequest,
    .OnMacMlmeRequest         = OnMacMlmeRequest,
    .OnJoinRequest            = OnJoinRequest,
    .OnTxData                 = OnTxData,
    .OnRxData                 = OnRxData,
    .OnClassChange            = OnClassChange,
    .OnBeaconStatusChange     = OnBeaconStatusChange,
    .OnSysTimeUpdate          = OnSysTimeUpdate,
};

#define SWEEP_APP_DATA_BUFFER_MAX_SIZE 242
static uint8_t AppDataBuffer[SWEEP_APP_DATA_BUFFER_MAX_SIZE];

static LmHandlerParams_t LmHandlerParams = {
    .Region               = ACTIVE_REGION,
    .AdrEnable            = LORAMAC_HANDLER_ADR_OFF,
    .IsTxConfirmed        = LORAMAC_HANDLER_UNCONFIRMED_MSG,
    .TxDatarate           = DR_3,
    .PublicNetworkEnable  = LORAWAN_PUBLIC_NETWORK,
    /* Duty-cycle check is disabled so back-to-back captures are possible in
     * the lab without hour-long ETSI wait times.  Re-enable for field use. */
    .DutyCycleEnabled     = false,
    .DataBufferMaxSize    = SWEEP_APP_DATA_BUFFER_MAX_SIZE,
    .DataBuffer           = AppDataBuffer,
    .PingSlotPeriodicity  = REGION_COMMON_DEFAULT_PING_SLOT_PERIODICITY,
};

static LmhpComplianceParams_t LmhpComplianceParams = {
    .FwVersion.Value             = 0x1300,
    .OnTxPeriodicityChanged      = OnTxPeriodicityChanged,
    .OnTxFrameCtrlChanged        = OnTxFrameCtrlChanged,
    .OnPingSlotPeriodicityChanged= OnPingSlotPeriodicityChanged,
};

/* -------------------------------------------------------------------------- */
/* Main entry point                                                            */
/* -------------------------------------------------------------------------- */

void payloadSweep(void)
{
    BoardInitMcu();
    BoardInitPeriph();

    if (LmHandlerInit(&LmHandlerCallbacks, &LmHandlerParams) != LORAMAC_HANDLER_SUCCESS) {
        while (1) {}
    }

    /* 50 ms RX-error margin covers the Apollo3 RTC 10 ms tick granularity */
    LmHandlerSetSystemMaxRxError(50);
    LmHandlerPackageRegister(PACKAGE_ID_COMPLIANCE, &LmhpComplianceParams);
    LmHandlerJoin();

    /* Free-running 32.768 kHz STIMER for ~30 µs-resolution timestamps */
    am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
    am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ);

    /* Start INA219 continuous sampling */
    if (INA219_Init() == INA219_NO_ERROR) {
        Ina219Ready = true;
        TimerInit(&SampleTimer, OnSampleTimerEvent);
        TimerSetValue(&SampleTimer, SAMPLE_INTERVAL_MS);
        TimerStart(&SampleTimer);
    }

    SweepPayloadSize = 0;
    SweepRep         = 0;
    bool txPending   = false;
    bool firstTx     = true;
    int  warmupRem   = SWEEP_WARMUP_REPS;

    while (1) {
        LmHandlerProcess();

        /* Fast-poll INA219 during capture; use 1 ms timer-based sampling otherwise */
        if (CaptureActive) {
            SampleFast();
        } else {
            SampleProcess();
        }

        /* ------------------------------------------------------------------ */
        /* Dump the completed capture window and advance the sweep counter     */
        /* ------------------------------------------------------------------ */
        if (IsDumpPending) {
            if (warmupRem > 0) {
                /* Warm-up capture: discard silently, do not advance counters */
                IsDumpPending = 0;
                warmupRem--;
            } else {
                PrintSummary();
            }
            txPending = false;
            TimerStart(&SampleTimer);  /* resume 1 ms idle sampling */

            /* Advance repetition / size counters (skipped during warm-up) */
            if (warmupRem == 0 && ++SweepRep >= SWEEP_REPS) {
                SweepRep = 0;
                SweepPayloadSize++;
                if (SweepPayloadSize > SWEEP_MAX_PAYLOAD_SIZE) {
                    am_util_stdio_printf("\n\n\n\n\n\n\n\n\n\n");
                    am_util_stdio_printf("{\"sweep_complete\":true}\n");
                    while (1) {}
                }
            }
        }

        /* ------------------------------------------------------------------ */
        /* Submit next transmission when MAC is free and no capture is open    */
        /* ------------------------------------------------------------------ */
        if (!txPending && !CaptureActive && !IsDumpPending && !LmHandlerIsBusy()) {
            if (firstTx) {
                am_util_delay_ms(15000);
                firstTx = false;
            }

            memset(SweepDummyBuf,
                   (uint8_t)(SweepPayloadSize ^ (SweepRep * 0x1F)),
                   SweepPayloadSize);

            /* Stop 1 ms timer — capture phase uses direct fast-poll instead */
            TimerStop(&SampleTimer);
            PostRoll      = POST_SAMPLES;
            CaptureActive = true;

            LmHandlerAppData_t appData = {
                .Buffer     = SweepDummyBuf,
                .BufferSize = SweepPayloadSize,
                .Port       = SWEEP_APP_PORT,
            };

            if (LmHandlerSend(&appData, LORAMAC_HANDLER_UNCONFIRMED_MSG) == LORAMAC_HANDLER_SUCCESS) {
                txPending = true;
            } else {
                CaptureActive = false;
                PostRoll      = 0;
                TimerStart(&SampleTimer);
            }
        }

        /* ------------------------------------------------------------------ */
        /* Low-power handler — skipped during capture to maximise sample rate  */
        /* ------------------------------------------------------------------ */
        if (!CaptureActive) {
            CRITICAL_SECTION_BEGIN();
            if (IsMacProcessPending == 1) {
                IsMacProcessPending = 0;
            } else {
                BoardLowPowerHandler();
            }
            CRITICAL_SECTION_END();
        }
    }
}
