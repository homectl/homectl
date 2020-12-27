#include "../include/CO2.h"

#include "../include/matrix.h"
#include "../include/print.h"

// Calibration values {Temp (Sensor), CO2 (Sensor), CO2 (Temtop)}:
constexpr LinearFunction<2> correction{{
    // Probably stable (measured after leaving the room with a closed
    // door and windows).
    {16, 441, 464},
    {16, 491, 568},

    {17, 468, 533},
    {17, 535, 635},
    {17, 544, 698},

    {18, 416, 399},
    {18, 749, 1099},
    {18, 773, 1133},
    {18, 814, 1200},

    {20, 539, 699},
    {20, 577, 776},

    // Might not be stable (measured while in the room, door/windows closed).
    {13, 540, 571},

    {14, 580, 676},

    {15, 482, 413},
    {15, 571, 561},
    {15, 582, 663},
    {15, 621, 726},
    {15, 631, 704},
    {15, 686, 859},
    {15, 704, 891},

    {16, 621, 752},
    {16, 600, 670},
    {16, 631, 696},
    {16, 644, 751},
    {16, 723, 933},

    {17, 609, 755},
    {17, 618, 767},
    {17, 637, 834},
    {17, 642, 838},
    {17, 734, 1009},

    {18, 565, 681},

    {19, 554, 736},

    {20, 552, 725},
}};

CO2::CO2(HardwareSerial &io) : input_(io) { io.begin(9600); }

static constexpr byte getCheckSum(byte const (&packet)[9]) {
  byte checksum = 0;
  for (byte i = 1; i < 8; i++) {
    checksum += packet[i];
  }
  checksum = 0xff - checksum;
  checksum += 1;
  return checksum;
}

static_assert(getCheckSum({0, 1, 2, 3, 4, 5, 6, 7, 8}) == 0xE4,
              "getCheckSum is implemented wrong");

static bool hasGarbage(Stream &io) {
  return io.available() > 0 && (byte)io.peek() != 0xFF;
}

static int readResponse(Stream &io, byte (&response)[9]) {
  int waited = 0;
  while (io.available() == 0) {
    delay(100);  // wait a short moment to avoid false reading
    if (waited++ > 10) {
      LOG(F("no response after 1 second"));
      io.flush();
      return STATUS_NO_RESPONSE;
    }
  }

  // The response starts with 0xff. If we get anything else, it's garbage.
  if (hasGarbage(io)) {
    LOGGER(logger);
    logger << F("skipping unexpected readings:");
    while (hasGarbage(io)) {
      logger.printf(F(" %02X"), io.read());
    }
  }

  if (io.available() > 0) {
    int const count = io.readBytes(response, 9);
    if (count < 9) {
      io.flush();
      return STATUS_INCOMPLETE;
    }
  } else {
    io.flush();
    return STATUS_INCOMPLETE;
  }

  LOG(F("  <<"), Bytes(response));

  return 0;
}

static int sendCommand(Stream &io, byte (&cmd)[9], byte (*response)[9]) {
  LOG(F("  >>"), Bytes(cmd));
  cmd[8] = getCheckSum(cmd);
  io.write(cmd, 9);

  byte buf[9];  // in case no buffer supplied
  if (response == nullptr) {
    response = &buf;
  }

  int ret = readResponse(io, *response);
  if (ret != 0) {
    LOG(F("error reading response: "), ret);
  }

  return ret;
}

void CO2::setABC(bool enabled) const {
  byte cmd[9] = {0xFF, 0x01, 0x79, byte(enabled * 0xA0), 0x00, 0x00,
                 0x00, 0x00, 0x00};
  sendCommand(input_, cmd, nullptr);
}

void CO2::calibrateZeroPoint() const {
  byte cmd[9] = {0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  sendCommand(input_, cmd, nullptr);
}

void CO2::calibrateSpanPoint(uint16_t ppm) const {
  byte cmd[9] = {0xFF, 0x01, 0x88, 0x00, byte(ppm / 256), byte(ppm % 256),
                 0x00, 0x00, 0x00};
  sendCommand(input_, cmd, nullptr);
}

CO2::Reading CO2::read() {
  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  byte response[9];  // for answer

  int ret = sendCommand(input_, cmd, &response);  // request PPM CO2
  if (ret != 0) {
    return {ret};
  }
  lastRequest_ = millis();

  byte const check = getCheckSum(response);
  if (response[8] != check) {
    LOG(F("checksum failed"));
    LOGF(F("received: %02X"), response[8]);
    LOGF(F("should be: %02X"), check);
    input_.flush();
    return {STATUS_CHECKSUM_MISMATCH};
  }

  LOG(F("applying linear correction: "), correction);
  int const ppm_raw = 256 * (int)response[2] + response[3];
  int const temperature = response[4] - 34;
  int const ppm_corrected = correction(temperature, ppm_raw);
  int const unknown = 256 * (int)response[6] + response[7];

  byte const status = response[5];

  // Is always 0 for version 19b.
  if (status != 0) {
    LOGF(F("status not OK: %02X"), status);
  }

  input_.flush();

  return {ppm_raw, ppm_corrected, temperature, unknown};
}

Print &operator<<(Print &out, CO2::Reading const &reading) {
  out << F("Temp: x1=") << reading.temperature << F(", CO2: x2=")
      << reading.ppm_raw << F(", Corrected: ") << reading.ppm_corrected
      << F(", Unknown: ") << reading.unknown;
  return out;
}
