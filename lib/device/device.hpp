#ifndef JETBEEP_DEVICE__H
#define JETBEEP_DEVICE__H

#include "device_types.hpp"

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

namespace JetBeep {
  typedef std::unordered_map<std::string, std::string> PaymentMetadata;  
  
  class Device {
  public:
    Device(DeviceCallback callback = nullptr);
    virtual ~Device();

    void open(const std::string& path);
    void close();

    void openSession();
    void closeSession();
    void requestBarcodes();
    void cancelBarcodes();
    void createPayment(uint32_t amount, const std::string& transactionId, const std::string& cashierId = "", 
      const PaymentMetadata& metadata = PaymentMetadata());
    void createPaymentToken(uint32_t amount, const std::string& transactionId, const std::string& cashierId = "", 
      const PaymentMetadata& metadata = PaymentMetadata());
    void cancelPayment();
    void resetState();      
    
    DeviceCallback callback;
    const std::vector<Barcode>& barcodes();
    const std::string& paymentToken();
    const std::string& paymentError();
    int errorCode();
  private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
  };
}

#endif