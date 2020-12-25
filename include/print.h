#include <Arduino.h>

void printTime(Stream &out, unsigned long currTime);

class Bytes {
  byte *data_;
  size_t size_;

 public:
  template <int N>
  explicit Bytes(byte (&data)[N]) : data_(data), size_(N) {}

  void print(Stream &out) const {
    // print out the response in hex
    for (size_t i = 0; i < size_; i++) {
      out.printf("%02X ", data_[i]);
    }
  }
};

static inline Stream &operator<<(Stream &out, struct Bytes const bytes) {
  bytes.print(out);
  return out;
}

template <typename Arg>
Stream &operator<<(Stream &out, Arg const &arg) {
  out.print(arg);
  return out;
}

static inline void print(Stream &out) { out.println(); }

template <typename Arg, typename... Args>
void print(Stream &out, Arg const &arg, Args &&... args) {
  out << arg;
  print(out, args...);
}
