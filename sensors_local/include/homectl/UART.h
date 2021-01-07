#pragma once

#include <Arduino.h>

/**
 * The sensor didn't respond within 1 second.
 *
 * Retries probably won't help. Most likely the sensor isn't wired up correctly.
 */
constexpr int STATUS_NO_RESPONSE = -2;
/**
 * The data received from the sensor was of the correct length but was garbled.
 *
 * Retries are likely to help. If retries don't help, there is probably a lot of
 * interference on the RX wire to the sensor's TX, or the voltage is too low
 * (note that the sensor takes around 130mA while performing a measurement).
 */
constexpr int STATUS_CHECKSUM_MISMATCH = -3;
/**
 * The data received from the sensor was truncated.
 *
 * Retries might help. If not, either the wiring is broken or the sensor is
 * borked. In the latter case, try a different sensor.
 */
constexpr int STATUS_INCOMPLETE = -4;

int readResponse(Stream &io, byte *response, size_t recvSize, byte startByte);

template <int N>
static int readResponse(Stream &io, byte (&response)[N], byte startByte) {
  return readResponse(io, response, N, startByte);
}
