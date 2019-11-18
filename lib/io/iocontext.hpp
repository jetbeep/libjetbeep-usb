#ifndef JETBEEP_IO_CONTEXT__H
#define JETBEEP_IO_CONTEXT__H

#include <memory>

namespace JetBeep {
  class IOContext {
  public:
    IOContext();

    static IOContext context;      
  private:
    class Impl;
    std::unique_ptr<Impl> m_impl;

    friend class AutoDevice;
    friend class SerialDevice;
    friend class DeviceDetection;
  };
}

#endif