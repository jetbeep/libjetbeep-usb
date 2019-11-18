#ifndef JETBEEP_IO_CONTEXT__H
#define JETBEEP_IO_CONTEXT__H

#include <memory>

namespace JetBeep {
  class IOContext {
  public:
    IOContext();
    IOContext(const IOContext& other);

    static IOContext context;
    IOContext& operator=(const IOContext &other);
  private:
    class Impl;
    std::shared_ptr<Impl> m_impl;

    friend class AutoDevice;
    friend class SerialDevice;
    friend class DeviceDetection;
  };
} // namespace JetBeep

#endif