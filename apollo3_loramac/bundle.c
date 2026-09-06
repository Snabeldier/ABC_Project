/*!
 * \file      bundle.c
 *
 * \brief     SHT31 measurement bundling for LoRaWAN uplinks.
 *
 * State machine:
 *   COLLECTING (TxReady=false, BundleCount < N_BUNDLE) ──AddSample fills──► READY
 *   READY      (TxReady=true)  ──LmHandlerSend called──────────────────────► IN_FLIGHT
 *   IN_FLIGHT  ──MCPS-Confirm (OnTxDone)─────────────────────────────────── COLLECTING
 *
 * All state is in the main-loop context; no ISR accesses bundle state.
 */

#include "bundle.h"
#include "am_util_stdio.h"

/* -------------------------------------------------------------------------
 * Buffer types
 * ---------------------------------------------------------------------- */

typedef struct {
    uint16_t temp_raw;
    uint16_t hum_raw;
} bundle_sample_t;

/* -------------------------------------------------------------------------
 * Module state  (volatile RAM — all samples are lost on power failure)
 * ---------------------------------------------------------------------- */

static bundle_sample_t BundleBuffer[N_BUNDLE_MAX];
static uint8_t  BundleCount = 0;    /* number of valid samples in BundleBuffer */
static bool     TxReady     = false; /* true when buffer holds N_BUNDLE samples */
static uint16_t BundleSeqNo = 0;    /* monotonically increasing; wraps at 65535 */

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

void Bundle_Init(void)
{
    BundleCount = 0;
    TxReady     = false;
    BundleSeqNo = 0;
}

bool Bundle_AddSample(uint16_t temp_raw, uint16_t hum_raw)
{
    if (TxReady) {
        /* Previous bundle not yet confirmed; cannot accept new samples. */
        return false;
    }

    if (BundleCount < N_BUNDLE) {
        BundleBuffer[BundleCount].temp_raw = temp_raw;
        BundleBuffer[BundleCount].hum_raw  = hum_raw;
        BundleCount++;
    }

    if (BundleCount >= N_BUNDLE) {
        TxReady = true;
        return true;
    }
    return false;
}

bool Bundle_IsTxReady(void)
{
    return TxReady;
}

uint8_t Bundle_SerializeInFlight(uint8_t *dst)
{
    if (!TxReady) return 0;

    uint8_t n = BundleCount;

    /* Bytes 0-1: bundle counter, big-endian. */
    dst[0] = (uint8_t)(BundleSeqNo >> 8);
    dst[1] = (uint8_t)(BundleSeqNo);

    for (uint8_t i = 0; i < n; i++) {
        uint16_t t    = BundleBuffer[i].temp_raw;
        uint16_t h    = BundleBuffer[i].hum_raw;
        uint8_t  base = (uint8_t)(2u + i * 4u);
        dst[base + 0u] = (uint8_t)(t >> 8);
        dst[base + 1u] = (uint8_t)(t);
        dst[base + 2u] = (uint8_t)(h >> 8);
        dst[base + 3u] = (uint8_t)(h);
    }

    return (uint8_t)(2u + n * 4u);
}

void Bundle_OnTxDone(bool ack_received)
{
    uint8_t n_payload = (uint8_t)(2u + BundleCount * 4u);
    am_util_stdio_printf("[BUNDLE] seq=%u n=%u payload=%u B ack=%s\n",
                         (unsigned)BundleSeqNo,
                         (unsigned)BundleCount,
                         (unsigned)n_payload,
                         ack_received ? "OK" : "FAIL");

    /*
     * Always reset after MCPS-Confirm, regardless of ACK status.
     * Rationale: retaining samples on a missing ACK would let the payload
     * grow beyond the DR3 limit across retry cycles. The bundle counter
     * allows the backend to detect gaps and reorder frames correctly.
     */
    BundleSeqNo++;   /* wraps naturally at uint16 rollover */
    BundleCount = 0;
    TxReady     = false;
}
