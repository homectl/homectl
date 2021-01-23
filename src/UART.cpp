#include "homectl/UART.h"

#include "homectl/Print.h"

static bool hasGarbage(Stream &io, byte startByte) {
  return io.available() > 0 && (byte)io.peek() != startByte;
}

int readResponse(Stream &io, byte *response, size_t recvSize, byte startByte) {
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
  if (hasGarbage(io, startByte)) {
    auto logger = LOG(F("skipping unexpected readings:"));
    while (hasGarbage(io, startByte)) {
      logger.printf(" %02X", io.read());
    }
  }

  if (io.available() > 0) {
    int const count = io.readBytes(response, recvSize);
    if (count < recvSize) {
      io.flush();
      return STATUS_INCOMPLETE;
    }
  } else {
    io.flush();
    return STATUS_INCOMPLETE;
  }

  LOG(F("  <<"), Bytes(response, recvSize));
  return 0;
}
