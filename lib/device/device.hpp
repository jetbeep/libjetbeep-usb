#ifndef JETBEEP_DEVICE__H
#define JETBEEP_DEVICE__H

#include <memory>
#include <string>

namespace JetBeep {
  enum DeviceEvent {
    WRITE_ERROR,
    READ_ERROR
  };

  union DeviceEventParameters {
    int errorCode;
  };

  typedef void (*DeviceCallback)(const DeviceEvent &, const DeviceEventParameters&);

  class Device {
  public:
    Device(DeviceCallback callback = nullptr);
    virtual ~Device();

    void open(const std::string& path);
    void close();

    void openSession();
    void closeSession();
    void requestBarcodes();
    void createPayment();
    
    DeviceCallback callback;
  private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
  };
}

#endif