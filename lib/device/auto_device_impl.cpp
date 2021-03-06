#include "../utils/platform.hpp"
#include "auto_device_impl.hpp"
#include "../io/iocontext_impl.hpp"
#include "../utils/utils.hpp"
#include "device_errors.hpp"
#include "./nfc/mifare-classic/mfc-provider.hpp"

#include <functional>

using namespace boost;
using namespace std;
using namespace JetBeep;

AutoDevice::Impl::Impl(AutoDeviceStateCallback* stateCallback,
                       AutoDevicePaymentErrorCallback* paymentErrorCallback,
                       AutoDeviceMobileCallback* mobileCallback,
                       AutoDeviceNFCEventCallback*  nfcEventCallback,
                       AutoDeviceNFCDetectionErrorCallback * nfcDetectionErrorCallback,
                       IOContext context)
  : m_context(context),
    m_stateCallback(stateCallback),
    m_paymentErrorCallback(paymentErrorCallback),
    m_mobileCallback(mobileCallback),
    m_nfcEventCallback(nfcEventCallback),
    m_nfcDetectionErrorCallback(nfcDetectionErrorCallback),
    m_state(AutoDeviceState::invalid),
    m_log("autodevice"),
    m_timer(context.m_impl->ioService),
    m_mobileConnected(false),
    m_started(false),
    m_deviceId(0) {
  m_device_sp = std::shared_ptr<SerialDevice>(new SerialDevice());
  m_detection.callback = std::bind(&AutoDevice::Impl::onDeviceEvent, this, std::placeholders::_1, std::placeholders::_2);
  m_device_sp->barcodesCallback = std::bind(&AutoDevice::Impl::onBarcodes, this, std::placeholders::_1);
  m_device_sp->paymentErrorCallback = std::bind(&AutoDevice::Impl::onPaymentError, this, std::placeholders::_1);
  m_device_sp->paymentSuccessCallback = std::bind(&AutoDevice::Impl::onPaymentSuccess, this);
  m_device_sp->paymentTokenCallback = std::bind(&AutoDevice::Impl::onPaymentToken, this, std::placeholders::_1);
  m_device_sp->mobileCallback = std::bind(&AutoDevice::Impl::onMobileConnectionChange, this, std::placeholders::_1);
  m_device_sp->nfcEventCallback = std::bind(&AutoDevice::Impl::onNFCEvent, this, std::placeholders::_1, std::placeholders::_2);
  m_device_sp->nfcDetectionErrorCallback = std::bind(&AutoDevice::Impl::onNFCDetectionError, this, std::placeholders::_1);

}

AutoDevice::Impl::~Impl() {
  try {
    stop();
  } catch (...) {
  }
}

void AutoDevice::Impl::start() {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_started) {
    throw Errors::InvalidState();
  }
  m_detection.start();
  m_started = true;
}

void AutoDevice::Impl::stop() {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (!m_started) {
    throw Errors::InvalidState();
  }

  m_detection.stop();
  try {
    m_device_sp->close();
  } catch (...) {
  }
  m_candidate = DeviceCandidate();
  rejectPendingOperations();
  changeState(AutoDeviceState::invalid);
  m_started = false;
}

void AutoDevice::Impl::onDeviceEvent(DeviceDetectionEvent event, DeviceCandidate candidate) {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  switch (event) {
  case DeviceDetectionEvent::added:
    if (m_state != AutoDeviceState::invalid && m_state != AutoDeviceState::firmwareVersionNotSupported) {
      m_log.w() << "detected additional device in the system, ignoring.." << Logger::endl;
      return;
    }

    try {
      m_log.d() << "Opening device path: " << candidate.path << Logger::endl;
      m_device_sp->open(candidate.path);
      m_candidate = candidate;
      initDevice();
    } catch (std::exception &error) {
      m_log.e() << "unable to open device! " << error.what() << Logger::endl;
    }
    break;
  case DeviceDetectionEvent::removed:
    if (m_candidate != candidate) {
      return;
    }

    try {
      m_device_sp->close();
    } catch (...) {
      m_log.e() << "unable to close device!" << Logger::endl;
    }
    rejectPendingOperations();
    changeState(AutoDeviceState::invalid, make_exception_ptr(Errors::DeviceLost()));
    break;
  }
}

void AutoDevice::Impl::initDevice() {
  m_pendingOperations.clear();
  rejectPendingOperations();
  m_device_sp->get(DeviceParameter::version)
    .thenPromise<std::string, Promise>([&](std::string version) {
      if (Utils::deviceFWVerToNumber(version) < Utils::deviceFWVerToNumber(JETBEEP_DEVICE_MIN_FW_VER)) {
        throw Errors::FirmwareVersionNotSupported();
      }
      m_version = version;
      return m_device_sp->get(DeviceParameter::deviceId);
    })
    .thenPromise([&](std::string strDeviceId) {
      m_deviceId = std::strtoul(strDeviceId.c_str(), nullptr, 16);
      return m_device_sp->resetState();
    })
    .then([&](...) { changeState(AutoDeviceState::sessionClosed, nullptr); })
    .catchError([&](const std::exception_ptr error) {
      try {
        std::rethrow_exception(error);
      } catch (const Errors::FirmwareVersionNotSupported& fwError) {
        m_log.w() << "Device firmware version too low" << Logger::endl;
        if (m_state != AutoDeviceState::firmwareVersionNotSupported) {
          changeState(AutoDeviceState::firmwareVersionNotSupported, make_exception_ptr(fwError));
        }
        try {
          m_device_sp->close();
        } catch (...) {
        }
      } catch (...) {
        if (m_state != AutoDeviceState::invalid) {
          changeState(AutoDeviceState::invalid, error);
        }
        m_log.e() << "unable to init device" << Logger::endl;
        m_timer.expires_from_now(boost::posix_time::millisec(2000));
        m_timer.async_wait(boost::bind(&AutoDevice::Impl::handleInitError, this, asio::placeholders::error));
      }
    });
}

void AutoDevice::Impl::resetState() {
  m_pendingOperations.clear();
  rejectPendingOperations();

  m_device_sp->resetState().then([&]() { changeState(AutoDeviceState::sessionClosed, nullptr); }).catchError([&](exception_ptr exception) {
    m_log.e() << "unable to reset state!" << Logger::endl;
    if (m_state != AutoDeviceState::invalid) {
      changeState(AutoDeviceState::invalid, exception);
    }
    m_timer.expires_from_now(boost::posix_time::millisec(2000));
    m_timer.async_wait(boost::bind(&AutoDevice::Impl::handleTimeout, this, asio::placeholders::error));
  });
}

void AutoDevice::Impl::handleTimeout(const boost::system::error_code& err) {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (err == boost::asio::error::operation_aborted) {
    return;
  }
  m_log.i() << "trying to issue reset state one more time..." << Logger::endl;
  resetState();
}

void AutoDevice::Impl::handleInitError(const boost::system::error_code& err) {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (err == boost::asio::error::operation_aborted) {
    return;
  }
  m_log.i() << "trying to init device one more time..." << Logger::endl;
  initDevice();
}

void AutoDevice::Impl::openSession() {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state != AutoDeviceState::sessionClosed) {
    throw Errors::InvalidState();
  }
  changeState(AutoDeviceState::sessionOpened);
  auto lambda = [&] {
    m_device_sp->openSession().then([&] { executeNextOperation(); }).catchError([&](exception_ptr) {
      m_log.e() << "open session error" << Logger::endl;
      resetState();
    });
  };

  enqueueOperation(lambda);
}

void AutoDevice::Impl::closeSession() {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state == AutoDeviceState::sessionClosed || m_state == AutoDeviceState::invalid) {
    throw Errors::InvalidState();
  }
  changeState(AutoDeviceState::sessionClosed);
  auto lambda = [&] {
    m_device_sp->closeSession().then([&] { executeNextOperation(); }).catchError([&](exception_ptr) {
      m_log.e() << "close session error" << Logger::endl;
      resetState();
    });
  };

  enqueueOperation(lambda);
}

void AutoDevice::Impl::enableBluetooth() {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state == AutoDeviceState::sessionOpened || m_state == AutoDeviceState::invalid) {
    throw Errors::InvalidState();
  }
  auto operation = [&] {
    m_device_sp->set(DeviceParameter::bluetooth, INTERFACE_ENABLED).then([&] { executeNextOperation(); }).catchError([&](exception_ptr) {
      m_log.e() << "bluetooth enabling error" << Logger::endl;
    });
  };

  enqueueOperation(operation);
}

void AutoDevice::Impl::disableBluetooth() {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state == AutoDeviceState::sessionOpened || m_state == AutoDeviceState::invalid) {
    throw Errors::InvalidState();
  }
  auto operation = [&] {
    m_device_sp->set(DeviceParameter::bluetooth, INTERFACE_DISABLED).then([&] { executeNextOperation(); }).catchError([&](exception_ptr) {
      m_log.e() << "bluetooth disabling error" << Logger::endl;
    });
  };

  enqueueOperation(operation);
}

void AutoDevice::Impl::enableNFC() {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state == AutoDeviceState::sessionOpened || m_state == AutoDeviceState::invalid) {
    throw Errors::InvalidState();
  }
  auto operation = [&] {
    m_device_sp->set(DeviceParameter::nfc, INTERFACE_ENABLED).then([&] { executeNextOperation(); }).catchError([&](exception_ptr) {
      m_log.e() << "NFC enabling error" << Logger::endl;
    });
  };
  //TODO pass error to application, to handle cases when NFC is not available

  enqueueOperation(operation);
}

void AutoDevice::Impl::disableNFC() {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state == AutoDeviceState::sessionOpened || m_state == AutoDeviceState::invalid) {
    throw Errors::InvalidState();
  }
  auto operation = [&] {
    m_device_sp->set(DeviceParameter::nfc, INTERFACE_DISABLED).then([&] { executeNextOperation(); }).catchError([&](exception_ptr) {
      m_log.e() << "NFC disabling error" << Logger::endl;
    });
  };

  enqueueOperation(operation);
}

Promise<std::vector<Barcode>> AutoDevice::Impl::requestBarcodes() {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state != AutoDeviceState::sessionOpened) {
    throw Errors::InvalidState();
  }
  changeState(AutoDeviceState::waitingForBarcodes);
  m_barcodesPromise = Promise<std::vector<Barcode>>();

  auto lambda = [&] {
    m_device_sp->requestBarcodes().then([&] { executeNextOperation(); }).catchError([&](exception_ptr) {
      m_log.e() << "close session error" << Logger::endl;
      resetState();
    });
  };

  enqueueOperation(lambda);
  return m_barcodesPromise;
}

void AutoDevice::Impl::cancelBarcodes() {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state != AutoDeviceState::waitingForBarcodes) {
    throw Errors::InvalidState();
  }
  changeState(AutoDeviceState::sessionOpened);
  auto lambda = [&] {
    m_device_sp->cancelBarcodes().then([&] { executeNextOperation(); }).catchError([&](exception_ptr) {
      m_log.e() << "cancel barcodes error" << Logger::endl;
      resetState();
    });
  };

  enqueueOperation(lambda);
}

Promise<void> AutoDevice::Impl::createPayment(uint32_t amount,
                                              const std::string& transactionId,
                                              const std::string& cashierId,
                                              const PaymentMetadata& metadata) {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state != AutoDeviceState::sessionOpened) {
    throw Errors::InvalidState();
  }

  changeState(AutoDeviceState::waitingForPaymentResult);
  m_paymentPromise = Promise<void>();

  auto lambda = [&, amount, transactionId, cashierId, metadata] {
    m_device_sp->createPayment(amount, transactionId, cashierId, metadata).then([&] { executeNextOperation(); }).catchError([&](exception_ptr) {
      m_log.e() << "create payment error" << Logger::endl;
      resetState();
    });
  };

  enqueueOperation(lambda);
  return m_paymentPromise;
}

Promise<std::string> AutoDevice::Impl::createPaymentToken(uint32_t amount,
                                                          const std::string& transactionId,
                                                          const std::string& cashierId,
                                                          const PaymentMetadata& metadata) {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state != AutoDeviceState::sessionOpened) {
    throw Errors::InvalidState();
  }

  changeState(AutoDeviceState::waitingForPaymentToken);
  m_paymentTokenPromise = Promise<string>();

  auto lambda = [&, amount, transactionId, cashierId, metadata] {
    m_device_sp->createPaymentToken(amount, transactionId, cashierId, metadata).then([&] { executeNextOperation(); }).catchError([&](exception_ptr) {
      m_log.e() << "create payment token error" << Logger::endl;
      resetState();
    });
  };

  enqueueOperation(lambda);
  return m_paymentTokenPromise;
}

void AutoDevice::Impl::confirmPayment() {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state != AutoDeviceState::waitingForConfirmation) {
    throw Errors::InvalidState();
  }

  changeState(AutoDeviceState::sessionClosed);
  auto lambda = [&] {
    m_device_sp->confirmPayment().then([&] { executeNextOperation(); }).catchError([&](exception_ptr) {
      m_log.e() << "confirm payment error" << Logger::endl;
      resetState();
    });
  };

  enqueueOperation(lambda);
}

void AutoDevice::Impl::cancelPayment() {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state != AutoDeviceState::waitingForConfirmation && m_state != AutoDeviceState::waitingForPaymentResult &&
      m_state != AutoDeviceState::waitingForPaymentToken) {
    throw Errors::InvalidState();
  }

  changeState(AutoDeviceState::sessionOpened);
  auto lambda = [&] {
    m_device_sp->cancelPayment().then([&] { executeNextOperation(); }).catchError([&](exception_ptr) {
      m_log.e() << "cancel payment error" << Logger::endl;
      resetState();
    });
  };

  enqueueOperation(lambda);
}

void AutoDevice::Impl::enqueueOperation(const std::function<void()>& callback) {
  if (m_pendingOperations.empty()) {
    callback();
  }
  m_pendingOperations.push_back(callback);
}

void AutoDevice::Impl::executeNextOperation() {
  m_pendingOperations.erase(m_pendingOperations.begin());
  if (!m_pendingOperations.empty()) {
    m_pendingOperations.front()();
  }
}

void AutoDevice::Impl::onBarcodes(const std::vector<Barcode>& barcodes) {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state != AutoDeviceState::waitingForBarcodes) {
    m_log.e() << "invalid state while received barcodes" << Logger::endl;
    return;
  }

  changeState(AutoDeviceState::sessionOpened);

  if (m_barcodesPromise.state() != PromiseState::undefined) {
    m_log.e() << "invalid promise state while received barcodes" << Logger::endl;
    return;
  }

  m_barcodesPromise.resolve(barcodes);
}

void AutoDevice::Impl::onPaymentError(const PaymentError& error) {
  auto paymentErrorCallback = *m_paymentErrorCallback;
  if (paymentErrorCallback) {
    paymentErrorCallback(error);
  }
}

void AutoDevice::Impl::onPaymentSuccess() {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state != AutoDeviceState::waitingForPaymentResult) {
    m_log.e() << "invalid state while received payment result" << Logger::endl;
    return;
  }

  changeState(AutoDeviceState::waitingForConfirmation);

  if (m_paymentPromise.state() != PromiseState::undefined) {
    m_log.e() << "invalid promise state while received payment result" << Logger::endl;
  }

  m_paymentPromise.resolve();
}

void AutoDevice::Impl::onPaymentToken(const std::string& token) {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state != AutoDeviceState::waitingForPaymentToken) {
    m_log.e() << "invalid state while received payment result" << Logger::endl;
    return;
  }

  changeState(AutoDeviceState::sessionOpened);

  if (m_paymentTokenPromise.state() != PromiseState::undefined) {
    m_log.e() << "invalid promise state while received payment result" << Logger::endl;
  }

  m_paymentTokenPromise.resolve(token);
}

void AutoDevice::Impl::onMobileConnectionChange(const SerialMobileEvent& event) {
  auto mobileCallback = *m_mobileCallback;

  if (event == SerialMobileEvent::connected) {
    m_mobileConnected = true;
  } else {
    m_mobileConnected = false;
  }

  if (mobileCallback) {
    mobileCallback(event);
  }
}

void AutoDevice::Impl::onNFCEvent(const SerialNFCEvent& event, const NFC::DetectionEventData &data) {
  auto callback = *m_nfcEventCallback;

  if (event == SerialNFCEvent::detected) {
    m_nfcDetected = true;
    m_nfcCardInfo = data;
  } else {
    m_nfcDetected = false;
  }

  if (callback) {
    callback(event, data);
  }
}

void AutoDevice::Impl::onNFCDetectionError(const NFC::DetectionErrorReason& reason) {
  auto callback = *m_nfcDetectionErrorCallback;

  m_nfcDetected = false;

  if (callback) {
    callback(reason);
  }
}

NFC::DetectionEventData AutoDevice::Impl::getNFCCardInfo() {
  if (!m_nfcDetected) {
    throw Errors::InvalidState();
  }

  return m_nfcCardInfo;
}

void AutoDevice::Impl::rejectPendingOperations() {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  m_timer.cancel();
  m_mobileConnected = false;

  if (m_paymentPromise.state() == PromiseState::undefined) {
    m_paymentPromise.reject(make_exception_ptr(Errors::OperationCancelled()));
  }
  if (m_paymentTokenPromise.state() == PromiseState::undefined) {
    m_paymentTokenPromise.reject(make_exception_ptr(Errors::OperationCancelled()));
  }
  if (m_barcodesPromise.state() == PromiseState::undefined) {
    m_barcodesPromise.reject(make_exception_ptr(Errors::OperationCancelled()));
  }
}

void AutoDevice::Impl::changeState(AutoDeviceState state, exception_ptr exception) {
  std::lock_guard<recursive_mutex> guard(m_mutex);
  m_state = state;

  m_context.m_impl->ioService.post([&, state, exception] {
    auto stateCallback = *m_stateCallback;
    if (stateCallback) {
      stateCallback(state, exception);
    }
  });
}

AutoDeviceState AutoDevice::Impl::state() {
  return m_state;
}

bool AutoDevice::Impl::isMobileConnected() {
  return m_mobileConnected;
}

bool AutoDevice::Impl::isNFCDetected() {
  return m_nfcDetected;
}

std::string AutoDevice::Impl::version() {
  return m_version;
}

unsigned long AutoDevice::Impl::deviceId() {
  return m_deviceId;
}

NFC::MifareClassic::MifareClassicProvider AutoDevice::Impl::getNFCMifareApiProvider() {
  if (!m_nfcDetected) {
    throw Errors::InvalidState();
  }
  if (m_nfcCardInfo.cardType != NFC::CardType::MIFARE_CLASSIC_1K
      && m_nfcCardInfo.cardType != NFC::CardType::MIFARE_CLASSIC_4K) {
    throw Errors::InvalidState();
  }

  return NFC::MifareClassic::MifareClassicProvider(m_device_sp, m_nfcCardInfo);
}