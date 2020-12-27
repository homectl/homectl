#include "../include/CO2.h"

#include "../include/linreg.h"
#include "../include/matrix.h"
#include "../include/print.h"

constexpr bool debug = true;

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

static int readResponse(Stream &io, byte (&response)[9]) {
  int waited = 0;
  while (io.available() == 0) {
    delay(100);  // wait a short moment to avoid false reading
    if (waited++ > 10) {
      if (debug) {
        Serial.println(F("CO2: no response after 1 second"));
      }
      io.flush();
      return STATUS_NO_RESPONSE;
    }
  }

  // The response starts with 0xff. If we get anything else, it's garbage.
  boolean skip = false;
  while (io.available() > 0 && (byte)io.peek() != 0xFF) {
    if (!skip) {
      Serial.print(F("CO2: skipping unexpected readings:"));
      skip = true;
    }
    Serial.printf(F(" %02X"), io.read());
  }
  if (skip) Serial.println();

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

  if (debug) {
    print(Serial, F("  << "), Bytes(response));
  }

  return 0;
}

static int sendCommand(Stream &io, byte (&cmd)[9], byte (*response)[9]) {
  if (debug) {
    print(Serial, F("  >> "), Bytes(cmd));
  }
  cmd[8] = getCheckSum(cmd);
  io.write(cmd, 9);

  byte buf[9];  // in case no buffer supplied
  if (response == nullptr) {
    response = &buf;
  }

  int ret = readResponse(io, *response);
  if (ret != 0) {
    print(Serial, F("CO2: error in "), __func__, F(": "), ret);
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
    Serial.println(F("CO2: checksum failed"));
    Serial.print(F("CO2: received: "));
    Serial.println(response[8], HEX);
    Serial.print(F("CO2: should be: "));
    Serial.println(check, HEX);
    input_.flush();
    return {STATUS_CHECKSUM_MISMATCH};
  }

  /*
    constexpr LinearRegression::LinearFunction tempCorrection{{
        {13, 15},
        {14, 15},
        {15, 16},
        {16, 17},
        {17, 18},
        {20, 20},
        {21, 21},
    }};
    */

  // Calibration values {Temp (Sensor), CO2 (Sensor), CO2 (Temtop)}:
  constexpr LinearFunction<2> correction{{
      {13, 540, 571},  {13, 563, 631},

      {14, 580, 676},

      {15, 482, 413},  {15, 571, 561}, {15, 582, 663}, {15, 621, 726},
      {15, 631, 704},  {15, 686, 859}, {15, 704, 891},

      {16, 621, 752},  {16, 600, 670}, {16, 631, 696}, {16, 644, 751},
      {16, 723, 933},

      {17, 609, 755},  {17, 618, 767}, {17, 637, 834}, {17, 642, 838},
      {17, 734, 1009},

      {18, 565, 681},

      {20, 530, 630},

      {21, 527, 700},  {21, 529, 703}, {21, 553, 723},
  }};

  if (debug) {
    print(Serial, F("CO2: applying linear correction: "), correction);
  }
  int const ppm_raw = 256 * (int)response[2] + response[3];
  int const temperature = response[4] - 34;
  int const ppm_corrected = correction(temperature, ppm_raw);
  int const unknown = 256 * (int)response[6] + response[7];

  byte const status = response[5];

  // Is always 0 for version 19b.
  if (status != 0) {
    Serial.print(F("CO2: status not OK: "));
    Serial.println(status, HEX);
  }

  input_.flush();

  return {ppm_raw, ppm_corrected, temperature, unknown};
}
