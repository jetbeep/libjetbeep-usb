#ifndef JETBEEP_DEVICE__H
#define JETBEEP_DEVICE__H

#include "device_types.hpp"
#include "device_parameter.hpp"
#include "../utils/promise.hpp"

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace JetBeep {      
  class SerialDevice {
  public:
    SerialDevice();
    virtual ~SerialDevice();

    void open(const std::string& path);
    void close();

    Promise<void> openSession();
    void closeSession();
    void requestBarcodes();
    void cancelBarcodes();
    void createPayment(uint32_t amount, const std::string& transactionId, const std::string& cashierId = "", 
      const PaymentMetadata& metadata = PaymentMetadata());
    void createPaymentToken(uint32_t amount, const std::string& transactionId, const std::string& cashierId = "", 
      const PaymentMetadata& metadata = PaymentMetadata());
    void cancelPayment();
    void resetState();
    void get(const DeviceParameter& parameter);
    void set(const DeviceParameter& parameter, const std::string& value);
    void beginPrivate();
    void commit(const std::string& signature);
    void getState();
    
    SerialErrorCallback errorCallback;
    SerialBarcodesCallback barcodesCallback;
    SerialPaymentErrorCallback paymentErrorCallback;
    SerialPaymentSuccessCallback paymentSuccessCallback;
    SerialPaymentTokenCallback paymentTokenCallback;
    SerialMobileCallback mobileCallback;
    SerialGetCallback getCallback;
    SerialGetStateCallback getStateCallback;
  private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
  };
}

#endif