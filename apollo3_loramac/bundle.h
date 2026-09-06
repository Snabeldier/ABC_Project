/*!
 * \file      bundle.h
 *
 * \brief     SHT31 measurement bundling for LoRaWAN uplinks.
 *
 *            Collects raw SHT31 samples into a statically-allocated RAM buffer
 *            and serialises complete bundles as confirmed uplinks.
 *
 * Payload format (own encoding, NOT JalapenosLPP per-channel headers):
 *   Byte 0-1 : bundle counter, uint16, big-endian, monotonically increasing,
 *              wraps from 65535 back to 0.
 *   Byte 2.. : n_bundle blocks of 4 bytes each, chronological order:
 *              [temp_raw MSB][temp_raw LSB][hum_raw MSB][hum_raw LSB]
 *   Total    : n_payload = 2 + 4 * n_bundle bytes.
 *
 * Fixed-point encoding of raw SHT31 values (established in sht3x.c):
 *   Temperature : T_celsius  = 175.0 * temp_raw / 65535.0 - 45.0
 *                 Range -45 .. +130 deg C, resolution ~2.7 mK.
 *                 temp_raw is unsigned uint16; no sign extension required.
 *   Humidity    : RH_percent = 100.0 * hum_raw  / 65535.0
 *                 Range 0 .. 100 %, resolution ~1.5 mRH.
 *                 hum_raw is unsigned uint16, sensor clamps to [0, 65535].
 *
 * Buffer memory:
 *   The buffer resides in RAM and is volatile. All buffered samples are lost
 *   on power failure or MCU reset before transmission completes.
 *
 * N_BUNDLE_MAX derivation:
 *   DR3 (SF9) max payload per RP002, EU868 : 115 bytes
 *   3 bytes reserved for MAC commands (FOpts): 115 - 3 = 112 bytes usable
 *   2 bytes for bundle counter header        : 112 - 2 = 110 bytes for data
 *   Bytes per sample                         : 4
 *   floor(110 / 4)                           = 27
 *   => N_BUNDLE_MAX = 27.
 */

#ifndef BUNDLE_H
#define BUNDLE_H

#include <stdint.h>
#include <stdbool.h>

/* Maximum samples per bundle (see derivation in file header). */
#define N_BUNDLE_MAX 27

/*
 * Active bundle size at compile time (1..N_BUNDLE_MAX).
 * Override on the compiler command line: -DN_BUNDLE=<value>.
 */
#ifndef N_BUNDLE
#define N_BUNDLE N_BUNDLE_MAX
#endif

#if N_BUNDLE < 1 || N_BUNDLE > N_BUNDLE_MAX
#error "N_BUNDLE must be in the range 1..N_BUNDLE_MAX"
#endif

/* Verify that the payload fits within the usable DR3 frame body. */
#if (2 + 4 * N_BUNDLE) > 112
#error "N_BUNDLE too large: 2 + 4*N_BUNDLE exceeds the 112-byte LoRaWAN payload limit at DR3"
#endif

/* Initialise the module; call once after LmHandlerInit() succeeds. */
void    Bundle_Init(void);

/*
 * Offer one raw SHT31 sample.
 * Returns true when the buffer just reached N_BUNDLE samples and is ready for
 * transmission. The caller must immediately trigger LmHandlerSend.
 * Returns false while the buffer is still filling or while a previous bundle
 * awaits its MCPS-Confirm.
 */
bool    Bundle_AddSample(uint16_t temp_raw, uint16_t hum_raw);

/* Returns true when the buffer is full and a TX should be triggered. */
bool    Bundle_IsTxReady(void);

/*
 * Serialise the ready bundle into dst.
 * dst must be at least 2 + 4 * N_BUNDLE_MAX bytes.
 * Returns the serialised byte count (2 + 4 * n_bundle), or 0 if not ready.
 */
uint8_t Bundle_SerializeInFlight(uint8_t *dst);

/*
 * Call from OnTxData after every MCPS-Confirm.
 * Resets the buffer regardless of ACK status and prints the per-TX debug line:
 *   [BUNDLE] seq=<n> n=<samples> payload=<bytes> B ack=<OK|FAIL>
 */
void    Bundle_OnTxDone(bool ack_received);

#endif /* BUNDLE_H */
