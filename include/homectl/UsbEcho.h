#pragma once

class UsbEcho {
  char recv_[32];
  char *it_ = recv_;

 public:
  void loop();
};
