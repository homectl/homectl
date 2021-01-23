#include "homectl/CO2.h"

#include "homectl/Matrix.h"
#include "homectl/Print.h"
#include "homectl/UART.h"

// The sensor delivers temperature readings in Celcius, but they are offset by
// some amount. We subtract this amount when displaying the actual temperature.
constexpr byte TEMPERATURE_OFFSET = 49;

// Calibration values {Temp (Sensor), CO2 (Sensor), CO2 (Temtop)}:
constexpr LinearFunction<2> correction{{
    // Probably stable (measured after leaving the room with a closed
    // door and windows).
    {15, 481, 546},

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

    {21, 560, 739},

    // Might not be stable (measured while in the room, door/windows
    // closed).
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

// Check that we didn't accidentally put more data in here than strictly
// necessary for runtime computation.
static_assert(sizeof correction == sizeof(double) * 3,
              "extra data unaccounted for in LinearFunction");

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

static void sendCommand(Stream &io, byte (&cmd)[9]) {
  LOG(F("  >>"), Bytes(cmd));
  cmd[8] = getCheckSum(cmd);
  io.write(cmd, 9);
}

void CO2::setABC(bool enabled) const {
  byte cmd[9] = {0xFF, 0x01, 0x79, byte(enabled * 0xA0), 0x00, 0x00,
                 0x00, 0x00, 0x00};
  sendCommand(input_, cmd);
}

void CO2::calibrateZeroPoint() const {
  byte cmd[9] = {0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  sendCommand(input_, cmd);
}

void CO2::calibrateSpanPoint(uint16_t ppm) const {
  byte cmd[9] = {0xFF, 0x01, 0x88, 0x00, byte(ppm / 256), byte(ppm % 256),
                 0x00, 0x00, 0x00};
  sendCommand(input_, cmd);
}

void CO2::requestReading() {
  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  sendCommand(input_, cmd);
}

void CO2::handleReading(byte (&response)[9]) const {
  LOG(F("applying linear correction: "), correction);
  int const ppm_raw = 256 * (int)response[2] + response[3];
  int const temperature = response[4] - TEMPERATURE_OFFSET;
  int const ppm_corrected = correction(temperature, ppm_raw);
  int const unknown = 256 * (int)response[6] + response[7];

  byte const status = response[5];

  // Is always 0 for version 19b.
  if (status != 0) {
    LOGF(F("status not OK: %02X"), status);
  }

  input_.flush();

  return newReading(Reading{ppm_raw, ppm_corrected, temperature, unknown});
}

void CO2::loop() {
  if (input_.available() == 0) return;

  byte response[9];

  int const ret = readResponse(input_, response, 0xFF);
  if (ret != 0) {
    LOG(F("error reading response: "), ret);
    return;
  }

  byte const check = getCheckSum(response);
  if (response[8] != check) {
    LOG(F("checksum failed"));
    LOGF(F("received: %02X"), response[8]);
    LOGF(F("should be: %02X"), check);
    input_.flush();
    // STATUS_CHECKSUM_MISMATCH
    return;
  }

  switch (response[1]) {
    case 0x79:
      // setABC
      break;
    case 0x86:
      return handleReading(response);
    case 0x87:
      // calibrateZeroPoint
      break;
    case 0x88:
      // calibrateSpanPoint
      break;
    default:
      LOGF(F("got response to unknown command: %02X"), response[1]);
      break;
  }
}

Print &operator<<(Print &out, CO2::Reading const &reading) {
  out << F("Temp: x1=") << reading.temperature << F(", CO2: x2=")
      << reading.ppm_raw << F(", Corrected: ") << reading.ppm_corrected
      << F(", Unknown: ") << reading.unknown;
  return out;
}

size_t CO2::Reading::printTo(Print &out) const {
  size_t sz = 0;
  sz += out.print(F("CO2: "));
  sz += out.print(ppm_raw);
  sz += out.print(F(" ("));
  sz += out.print(ppm_corrected);
  sz += out.print(F(") @"));
  sz += out.print(temperature);
  sz += out.print('C');
  return sz;
}
