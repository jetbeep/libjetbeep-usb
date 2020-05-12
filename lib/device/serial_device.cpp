#include "../utils/platform.hpp"
#include "serial_device.hpp"
#include "device_utils.hpp"
#include "serial_device_impl.hpp"

using namespace JetBeep;
using namespace std;

// Device

SerialDevice::SerialDevice(IOContext context) {
  SerialDeviceCallbacks callbacks = {&errorCallback,          &barcodesCallback,     &paymentErrorCallback,
                                     &paymentSuccessCallback, &paymentTokenCallback, &mobileCallback, &nfcEventCallback, &nfcDetectionErrorCallback};

  m_impl.reset(new Impl(callbacks, context));
}

SerialDevice::~SerialDevice() {
}

void SerialDevice::open(const string& path) {
  m_impl->open(path);
}
void SerialDevice::close() {
  m_impl->close();
}

Promise<void> SerialDevice::openSession() {
  return m_impl->execute(DeviceResponses::openSession);
}
Promise<void> SerialDevice::closeSession() {
  return m_impl->execute(DeviceResponses::closeSession, "", 6000);
}
Promise<void> SerialDevice::requestBarcodes() {
  return m_impl->execute(DeviceResponses::requestBarcodes);
}
Promise<void> SerialDevice::cancelBarcodes() {
  return m_impl->execute(DeviceResponses::cancelBarcodes);
}

Promise<void> SerialDevice::createPayment(uint32_t amount,
                                          const std::string& transactionId,
                                          const std::string& cashierId,
                                          const PaymentMetadata& metadata) {
  ostringstream ss;

// NOTE: to_string(amount) is used to overcome very strange bug in Linux 32-bit and Java JNI calls
  ss << to_string(amount) << " " << transactionId;

  if (cashierId != "") {
    ss << " " << cashierId;

    if (!metadata.empty()) {
      ss << " ";

      for (auto it = metadata.begin(); it != metadata.end();) {
        ss << (*it).first << ":" << (*it).second;

        it++;
        if (it != metadata.end()) {
          ss << ";";
        }
      }
    }
  }
  return m_impl->execute(DeviceResponses::createPayment, ss.str());
}

Promise<void> SerialDevice::createPaymentToken(uint32_t amount,
                                               const std::string& transactionId,
                                               const std::string& cashierId,
                                               const PaymentMetadata& metadata) {
  ostringstream ss;

// NOTE: to_string(amount) is used to overcome very strange bug in Linux 32-bit and Java JNI calls
  ss << to_string(amount) << " " << transactionId;

  if (cashierId != "") {
    ss << " " << cashierId;

    if (!metadata.empty()) {
      ss << " ";

      for (auto it = metadata.begin(); it != metadata.end();) {
        ss << (*it).first << ":" << (*it).second;

        it++;
        if (it != metadata.end()) {
          ss << ";";
        }
      }
    }
  }
  return m_impl->execute(DeviceResponses::createPaymentToken, ss.str());
}

Promise<void> SerialDevice::confirmPayment() {
  return m_impl->execute(DeviceResponses::confirmPayment);
}
Promise<void> SerialDevice::cancelPayment() {
  return m_impl->execute(DeviceResponses::cancelPayment);
}
Promise<void> SerialDevice::resetState() {
  return m_impl->execute(DeviceResponses::resetState);
}
Promise<string> SerialDevice::get(const DeviceParameter& parameter) {
  return m_impl->executeString(DeviceResponses::get, DeviceUtils::parameterToString(parameter));
}
Promise<void> SerialDevice::set(const DeviceParameter& parameter, const std::string& value) {
  return m_impl->execute(DeviceResponses::set, DeviceUtils::parameterToString(parameter) + " " + value);
}
Promise<void> SerialDevice::commit(const string& signature) {
  return m_impl->execute(DeviceResponses::commit, signature);
}
Promise<SerialGetStateResult> SerialDevice::getState() {
  return m_impl->executeGetState(DeviceResponses::getState);
}

Promise<void> SerialDevice::beginPrivate(const SerialBeginPrivateMode& mode) {
  string param;
  switch (mode) {
  case SerialBeginPrivateMode::setup:
    param = "setup";
    break;
  case SerialBeginPrivateMode::config:
    param = "config";
    break;
  default:
    throw std::invalid_argument("invalid argument provided");
  }

  return m_impl->execute(DeviceResponses::beginPrivate, param);
}

Promise<std::string> SerialDevice::nfcReadMFC(uint8_t blockNo) {
  return m_impl->executeString(DeviceResponses::nfcReadMFC, std::to_string(blockNo));
}

Promise<std::string> SerialDevice::nfcSecureReadMFC(uint8_t blockNo,
                                                    const std::string& keyBase64,
                                                    const std::string& keyType) {
  std::stringstream ss;
  ss << std::to_string(blockNo) << " " << keyBase64 << " " << keyType;
  return m_impl->executeString(DeviceResponses::nfcSecureReadMFC, ss.str());
}

Promise<void> SerialDevice::nfcWriteMFC(uint8_t blockNo, const std::string& contentBase64) {
  std::stringstream ss;
  ss << std::to_string(blockNo) << " " << contentBase64;
  return m_impl->execute(DeviceResponses::nfcWriteMFC, ss.str());
}

Promise<void> SerialDevice::nfcSecureWriteMFC(uint8_t blockNo,
                                const std::string& contentBase64,
                                const std::string& keyBase64,
                                const std::string& keyType) {
  std::stringstream ss;
  ss << std::to_string(blockNo) << " " << contentBase64 << " " << keyBase64 << " " << keyType;
  return m_impl->execute(DeviceResponses::nfcSecureWriteMFC, ss.str());
}