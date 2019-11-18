#include "auto_device.hpp"
#include "auto_device_impl.hpp"

using namespace std;
using namespace JetBeep;

AutoDevice::AutoDevice()
:m_impl(new AutoDevice::Impl(&stateCallback, &paymentErrorCallback, &mobileCallback)) {

}
AutoDevice::~AutoDevice() {}

void AutoDevice::start() {
  m_impl->start();
}

void AutoDevice::stop() {
  m_impl->stop();
}

void AutoDevice::openSession() {
  m_impl->openSession();
}

void AutoDevice::closeSession() {
  m_impl->closeSession();
}

Promise<std::vector<Barcode>> AutoDevice::requestBarcodes() {
  return m_impl->requestBarcodes();
}

void AutoDevice::cancelBarcodes() {
  m_impl->cancelBarcodes();
}

Promise<void> AutoDevice::createPayment(uint32_t amount, const std::string& transactionId, const std::string& cashierId, 
  const PaymentMetadata& metadata) {
  return m_impl->createPayment(amount, transactionId, cashierId, metadata);
}

void AutoDevice::confirmPayment() {
  m_impl->confirmPayment();
}

Promise<std::string> AutoDevice::createPaymentToken(uint32_t amount, const std::string& transactionId, const std::string& cashierId, 
  const PaymentMetadata& metadata) {
  return m_impl->createPaymentToken(amount, transactionId, cashierId, metadata);
}

void AutoDevice::cancelPayment() {
  m_impl->cancelPayment();
}

AutoDeviceState AutoDevice::state() {
  return m_impl->state();
}

bool AutoDevice::isMobileConnected() {
  return m_impl->isMobileConnected();
}