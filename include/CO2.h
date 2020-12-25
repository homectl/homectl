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

/**
 * MH-Z19b sensor interface using a user-supplied UART stream.
 */
class CO2 {
  Stream &input_;
  unsigned long lastRequest_ = 0;

 public:
  /**
   * The result of receiving a reading from the sensor. If the reading failed
   * for some reason, the error code is stored in @c ppm_raw.
   */
  struct Reading {
    /**
     * The CO2 ppm value received from the sensor.
     *
     * In case of failure, this contains the error code.
     */
    int ppm_raw;
    /**
     * The CO2 ppm value with a linear correction applied.
     *
     * The calibration values are hard-coded in @c read().
     */
    int ppm_corrected;
    /**
     * The temperature measured by the CO2 sensor.
     *
     * Since the sensor is based on infrared absorption, it needs a thermometer
     * to counter environmental effects of temperature shifts. The UART protocol
     * is nice enough to give us this value, so we're nice enough to give it to
     * our client code.
     */
    int temperature;
    /**
     * Some other value that seems to always be 0.
     *
     * Maybe it contains nothing, but the sensor sends this to us, so here it
     * is.
     */
    int unknown;
  };

  /**
   * Set up communication at baud 9600 with the sensor.
   */
  explicit CO2(HardwareSerial &input);
  /**
   * Enable or disable Automatic Baseline Calibration.
   *
   * This requires that at least once every 24 hours the sensor is exposed to a
   * minimum CO2 environment of 450ppm. Indoors this is unlikely to happen
   * unless you heavily ventilate every room every day. With ABC enabled, your
   * sensor will drift upwards and no longer be able to measure lower values of
   * CO2. We highly recommend doing a manual zero point calibration before
   * starting the sensor, and then perhaps once a year.
   */
  void setABC(bool enabled) const;
  /**
   * Invoke zero point calibration, setting the current value to 450ppm.
   *
   * This tells the sensor that whatever it's measuring right now is in fact
   * 450ppm. After exposing the sensor to the outside air for 20 minutes, you
   * can be reasonably sure that this is the case.
   */
  void calibrateZeroPoint() const;
  /**
   * Set the second (high) point in the 2-point sensor calibration.
   *
   * As far as we know, this does nothing useful when invoked in a 1500ppm
   * environment. Perhaps a higher concentration of CO2 is needed for this to be
   * useful, but so far all tests have shown this to have an adverse effect on
   * precision (accuracy is not affected).
   */
  void calibrateSpanPoint(uint16_t ppm) const;

  /**
   * Retrieve the latest reading from the sensor.
   *
   * This doesn't necessarily perform a reading. The sensor performs readings
   * once every 5 seconds, so fetching readings more often than that gives you
   * no new information.
   */
  Reading read();
};
