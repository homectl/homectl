#include "homectl/PMS5003T.h"

#include "homectl/Logger.h"
#include "homectl/UART.h"

constexpr byte INIT_BYTE1 = 0x42;
constexpr byte INIT_BYTE2 = 0x4d;
constexpr byte RESPONSE_LEN = 28;

template <int N>
static uint16_t getPms5003Checksum(byte const (&payload)[N]) {
  uint16_t checksum = 0;
  for (uint8_t i = 0; i < N - 2; ++i) {
    checksum += payload[i];
  }
  return checksum;
}

PMS5003T::PMS5003T(HardwareSerial &io) : io_(io) {
  io_.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  timer_ = xTimerCreate(
      "PMS5003", pdMS_TO_TICKS(10000), pdTRUE, this, [](TimerHandle_t timer) {
        PMS5003T &self = *static_cast<PMS5003T *>(pvTimerGetTimerID(timer));
        self.sleep(false);
      });

  sleep(false);
}

template <int SendSize>
static void sendCommand(Stream &io, byte (&cmd)[SendSize], byte startByte) {
  LOG(F("  >>"), Bytes(cmd));
  uint16_t checksum = getPms5003Checksum(cmd);
  cmd[SendSize - 2] = (checksum >> 8) & 0xff;
  cmd[SendSize - 1] = (checksum & 0xff);
  io.write(cmd, SendSize);
}

void PMS5003T::sendSleep(bool enabled) {
  byte cmd[] = {INIT_BYTE1, INIT_BYTE2, 0xe4, 0x00, !enabled, 0x00, 0x00};
  LOG("Setting PMS5003T sensor sleep mode to ", enabled);

  sendCommand(io_, cmd, INIT_BYTE1);
}

void PMS5003T::sleep(bool enabled) {
  sleepCommand_ = enabled ? SLEEP_ENABLE : SLEEP_DISABLE;
  if (enabled) {
    // Sleep mode on; start the timer to wake the sensor back up.
    xTimerStart(timer_, 0);
  } else {
    // Sensor is going to wake up, so stop telling it to.
    xTimerStop(timer_, 0);
  }
}

static void skipGarbage(Stream &io, Print &&logger) {
  while (io.available() != 0) {
    logger.printf(" %02x", io.read());
  }
}

void PMS5003T::processOutput() {
  switch (sleepCommand_) {
    case SLEEP_NONE:
      // No change.
      break;
    case SLEEP_ENABLE:
      sendSleep(true);
      break;
    case SLEEP_DISABLE:
      sendSleep(false);
      break;
  }

  sleepCommand_ = SLEEP_NONE;
}

void PMS5003T::processInput() {
  if (io_.available() == 0) return;

  byte const b1 = io_.read();
  if (b1 != INIT_BYTE1) {
    return skipGarbage(
        io_, LOGF(F("Incorrect start byte 1 of PMS5003T reading: %02X"), b1));
  }
  byte const b2 = io_.read();
  if (b2 != INIT_BYTE2) {
    return skipGarbage(
        io_, LOGF(F("Incorrect start byte 2 of PMS5003T reading: %02X"), b2));
  }

  byte const lenHi = io_.read();
  byte const lenLo = io_.read();
  uint16_t const len = (uint16_t(lenHi) << 8) | lenLo;

  if (len == 4) {
    // This is the response to sendSleep().
    byte payload[4];
    size_t const actualRead = io_.readBytes(payload, len);
    if (actualRead != len) {
      LOG(F("ERROR: couldn't read desired number of bytes: "), actualRead,
          F(" but wanted "), len);
    }
    return;
  }

  if (len != RESPONSE_LEN) {
    return skipGarbage(io_, LOG("Got unexpected payload length: ", len));
  }

  byte payload[RESPONSE_LEN];
  size_t const actualRead = io_.readBytes(payload, len);
  if (actualRead != len) {
    LOG(F("ERROR: couldn't read desired number of bytes: "), actualRead,
        F(" but wanted "), len);
    return;
  }

  Reading const reading(payload);
  uint16_t const checksumRef =
      getPms5003Checksum(payload) + INIT_BYTE1 + INIT_BYTE2 + RESPONSE_LEN;
  if (reading.checksum != checksumRef) {
    LOGF(F("ERROR: Checksum from PMS5003T didn't match: %d vs. %d (computed)"),
         reading.checksum, checksumRef);
    return;
  }

  LOG("\n  STD: PM1.0: ", reading.pm1_0_std, ", PM2.5: ", reading.pm2_5_std,
      ", PM10: ", reading.pm10_std, "\n  ATM: PM1.0: ", reading.pm1_0_atm,
      ", PM2.5: ", reading.pm2_5_atm, ", PM10: ", reading.pm10_atm,
      "\n  CNT: PM0.3: ", reading.pm0_3_cnt, ", PM0.5: ", reading.pm0_5_cnt,
      ", PM1.0: ", reading.pm1_0_cnt, ", PM2.5: ", reading.pm2_5_cnt,
      "\n  Temp: ", float(reading.temp) / 10,
      "C, Hum: ", float(reading.hum) / 10, '%');

  if (reading.pm2_5_atm == 0) {
    LOGF(F("WARNING: skipping zero reading from PM sensor"));
    return;
  }

  // Put the sensor to sleep, and start the timer for waking it up.
  sleep(true);
  return newReading(reading);
}

void PMS5003T::loop() {
  processOutput();
  processInput();
}

size_t PMS5003T::Reading::printTo(Print &out) const {
  size_t sz = 0;
  if (pm2_5_atm == UINT16_MAX) {
    sz += out.print(F("PM2.5/10: Unknown"));
  } else {
    sz += out.print("PM2.5: ");
    sz += out.print(pm2_5_atm);
    sz += out.print(", PM10: ");
    sz += out.print(pm10_atm);
  }
  return sz;
}
