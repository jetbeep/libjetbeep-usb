#ifndef JETBEEP_DEVICE__H
#define JETBEEP_DEVICE__H

#include "device_types.hpp"

#include <memory>
#include <string>
#include <vector>

namespace JetBeep {  
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
    const std::vector<Barcode>& barcodes();
    const std::string& paymentToken();
    int errorCode();
  private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
  };
}

#endif