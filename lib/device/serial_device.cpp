#include "serial_device.hpp"
#include "device_utils.hpp"
#include "serial_device_impl.hpp"

using namespace JetBeep;
using namespace std;

// Device

SerialDevice::SerialDevice() {
  SerialDeviceCallbacks callbacks = {&errorCallback, &barcodesCallback, &paymentErrorCallback, 
    &paymentSuccessCallback, &paymentTokenCallback, &mobileCallback, &getCallback, &getStateCallback};

  m_impl.reset(new Impl(callbacks));
}

SerialDevice::~SerialDevice() {}

void SerialDevice::open(const string& path) { m_impl->open(path); }
void SerialDevice::close() { m_impl->close(); }
void SerialDevice::openSession(function<void (const SerialError &)> callback) { m_impl->executeCallback = callback; m_impl->execute("OPEN_SESSION\r\n"); }
void SerialDevice::closeSession() { m_impl->execute("CLOSE_SESSION\r\n"); }
void SerialDevice::requestBarcodes() { m_impl->execute("REQUEST_BARCODES\r\n"); }
void SerialDevice::cancelBarcodes() { m_impl->execute("CANCEL_BARCODES\r\n"); }

void SerialDevice::createPayment(uint32_t amount, const std::string& transactionId, const std::string& cashierId, 
  const PaymentMetadata& metadata) {
    ostringstream ss;

    ss << "CREATE_PAYMENT " << amount << " " << transactionId;

    if (cashierId != "") {
      ss << " " << cashierId;

      if (!metadata.empty()) {
        ss << " ";

        for (auto it = metadata.begin(); it != metadata.end(); ++it) {
          ss << (*it).first << ":" << (*it).second;           
        }

        ss << ";";  
      }
    }
    ss << "\r\n";    
    m_impl->execute(ss.str());
}

void SerialDevice::createPaymentToken(uint32_t amount, const std::string& transactionId, const std::string& cashierId, 
  const PaymentMetadata &metadata) {
    ostringstream ss;

    ss << "CREATE_PAYMENT_TOKEN " << amount << " " << transactionId;

    if (cashierId != "") {
      ss << " " << cashierId;

      if (!metadata.empty()) {
        ss << " ";

        for (auto it = metadata.begin(); it != metadata.end(); ++it) {
          ss << (*it).first << ":" << (*it).second;           
        }

        ss << ";";    
      }
    }
    ss << "\r\n";    
    m_impl->execute(ss.str());
}

void SerialDevice::cancelPayment() { m_impl->execute("CANCEL_PAYMENT\r\n"); }
void SerialDevice::resetState() { m_impl->execute("RESET_STATE\r\n"); }
void SerialDevice::get(const DeviceParameter& parameter) { m_impl->execute("GET " + DeviceUtils::parameterToString(parameter) + "\r\n"); }
void SerialDevice::set(const DeviceParameter& parameter, const std::string &value) { m_impl->execute("SET " + DeviceUtils::parameterToString(parameter) + " " + value + "\r\n"); }
void SerialDevice::beginPrivate() { m_impl->execute("BEGIN_PRIVATE\r\n"); }
void SerialDevice::commit(const string& signature) { m_impl->execute("COMMIT " + signature + "\r\n"); }
void SerialDevice::getState() { m_impl->execute("GETSTATE \r\n"); }