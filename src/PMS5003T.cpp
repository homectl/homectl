#include "homectl/PMS5003T.h"

#include "homectl/Print.h"
#include "homectl/UART.h"

constexpr byte INIT_BYTE1 = 0x42;
constexpr byte INIT_BYTE2 = 0x4d;

template <int N>
static uint16_t getPms5003Checksum(byte const (&payload)[N]) {
  uint16_t checksum = 0;
  for (uint8_t i = 0; i < N - 2; ++i) {
    checksum += payload[i];
  }
  return checksum;
}

PMS5003T::PMS5003T(HardwareSerial &io) : io_(io) {
  io_.begin(9600, SERIAL_8N1, 32, 33);
}

template <int SendSize, int RecvSize>
static int sendCommand(Stream &io, byte (&cmd)[SendSize],
                       byte (&response)[RecvSize], byte startByte) {
  LOG(F("  >>"), Bytes(cmd));
  uint16_t checksum = getPms5003Checksum(cmd);
  cmd[SendSize - 2] = (checksum >> 8) & 0xff;
  cmd[SendSize - 1] = (checksum & 0xff);
  io.write(cmd, SendSize);

  int const ret = readResponse(io, response, startByte);
  if (ret != 0) {
    LOG(F("error reading response: "), ret);
  }

  return ret;
}

void PMS5003T::sleep(bool enabled) {
  byte cmd[] = {INIT_BYTE1, INIT_BYTE2, 0xe4, 0x00, !enabled, 0x00, 0x00};
  LOG("Setting PMS5003T sensor sleep mode to ", enabled);

  byte response[8];
  sendCommand(io_, cmd, response, INIT_BYTE1);
}

void PMS5003T::loop() {
  if (io_.available() == 0) return;

  byte const b1 = io_.read();
  if (b1 != INIT_BYTE1) {
    LOGF(F("Incorrect start byte 1 of PMS5003T reading: %02X"), b1);
    return;
  }
  byte const b2 = io_.read();
  if (b2 != INIT_BYTE2) {
    LOGF(F("Incorrect start byte 2 of PMS5003T reading: %02X"), b2);
    return;
  }

  byte lenHi = io_.read();
  byte lenLo = io_.read();
  uint16_t len = (uint16_t(lenHi) << 8) | lenLo;
  if (len != 28) {
    LOG("Got unexpected payload length: ", len);
    while (io_.available() != 0) Serial.printf("%02x ", io_.read());
    return;
  }

  byte payload[28];
  for (uint16_t i = 0; i < len; ++i) {
    payload[i] = io_.read();
  }

  Reading const reading(payload);
  uint16_t const checksumRef =
      getPms5003Checksum(payload) + INIT_BYTE1 + INIT_BYTE2 + sizeof payload;
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

  while (io_.available() != 0) {
    LOG("Discarding garbage");
    while (io_.available() != 0) Serial.printf("%02x ", io_.read());
  }

  return newReading(reading);
}

PMS5003T::Reading const &PMS5003T::read() const { return reading_; }

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
