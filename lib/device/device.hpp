#ifndef JETBEEP_DEVICE__H
#define JETBEEP_DEVICE__H

#include <string>

namespace JetBeep {
  class Device {
    void open(const std::string& path);
    void close();
  };
}

#endif