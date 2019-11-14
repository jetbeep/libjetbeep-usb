#include "auto_device_impl.hpp"
#include "device_errors.hpp"

#include <functional>

using namespace boost;
using namespace std;
using namespace JetBeep;

AutoDevice::Impl::Impl(AutoDeviceStateChangeCallback *callback)
: m_callback(callback), m_state(AutoDeviceState::invalid), m_log("autodevice"), m_thread(&AutoDevice::Impl::runLoop, this),
  m_work(m_io_service), m_timer(m_io_service) {
  m_detection.callback = std::bind(&AutoDevice::Impl::onDeviceEvent, this, std::placeholders::_1, std::placeholders::_2);
  m_device.barcodesCallback = std::bind(&AutoDevice::Impl::onBarcodes, this, std::placeholders::_1);
  m_device.paymentErrorCallback = std::bind(&AutoDevice::Impl::onPaymentError, this, std::placeholders::_1);
  m_device.paymentSuccessCallback = std::bind(&AutoDevice::Impl::onPaymentSuccess, this);
  m_device.paymentTokenCallback = std::bind(&AutoDevice::Impl::onPaymentToken, this, std::placeholders::_1);
}

AutoDevice::Impl::~Impl() {
  m_io_service.stop();
  m_thread.join();
}

void AutoDevice::Impl::onDeviceEvent(const DeviceDetectionEvent& event, const DeviceCandidate& candidate) {
  std::lock_guard<recursive_mutex> guard(m_mutex);
  auto callback = *m_callback;

  switch (event) {
    case DeviceDetectionEvent::added:
        if (m_state != AutoDeviceState::invalid) {
          m_log.w() << "detected additional device in the system, ignoring.." << Logger::endl;
          return;
        }

        try {
          m_device.open(candidate.path);
          m_candidate = candidate;
          resetState();
        } catch (...) {
          m_log.e() << "unable to open device!" << Logger::endl;
        }
      break;
    case DeviceDetectionEvent::removed:
      if (m_candidate != candidate) {
        return;
      }

      try {
        m_device.close();          
      } catch (...) {
        m_log.e() << "unable to close device!" << Logger::endl;
      }
      m_timer.cancel(); // if there is pending reset state attempt, we have to cancel it
      m_state = AutoDeviceState::invalid;
      notifyStateChange(m_state, make_exception_ptr(Errors::DeviceLost()));    
      break;
  }
}

void AutoDevice::Impl::resetState() {
  m_pendingOperations.clear();

  if (m_barcodesPromise.state() == PromiseState::undefined) {
    m_barcodesPromise.reject(make_exception_ptr(Errors::InvalidResponse()));
  }  

  m_device.resetState()
    .then([&] {
    m_state = AutoDeviceState::sessionClosed;
    notifyStateChange(m_state, nullptr);
  }).catchError([&] (exception_ptr exception) {
    m_log.e() << "unable to reset state!" << Logger::endl;
    if (m_state != AutoDeviceState::invalid) {
      m_state = AutoDeviceState::invalid;
      notifyStateChange(m_state, exception);      
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

void AutoDevice::Impl::openSession() {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state != AutoDeviceState::sessionClosed) {
    throw Errors::InvalidState();
  } 

  m_state = AutoDeviceState::sessionOpened;
  auto lambda = [&] {
    m_device.openSession()
      .then([&] {
        executeNextOperation();
      }).catchError([&] (exception_ptr) {
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

  m_state = AutoDeviceState::sessionClosed;
  auto lambda = [&] {
    m_device.closeSession()
      .then([&] {
        executeNextOperation();
      }).catchError([&] (exception_ptr) {
        m_log.e() << "close session error" << Logger::endl;
        resetState();
      });
  };

  enqueueOperation(lambda);
}

Promise<std::vector<Barcode> > AutoDevice::Impl::requestBarcodes() {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state != AutoDeviceState::sessionOpened) {
    throw Errors::InvalidState();
  }

  m_state = AutoDeviceState::waitingForBarcodes;
  m_barcodesPromise = Promise<std::vector<Barcode> >();

  auto lambda = [&] {
      m_device.closeSession()
      .then([&] {
        executeNextOperation();
      }).catchError([&] (exception_ptr) {
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

  m_state = AutoDeviceState::sessionOpened;
  auto lambda = [&] {
    m_device.cancelBarcodes()
      .then([&] {
        executeNextOperation();
      }).catchError([&] (exception_ptr) {
        m_log.e() << "cancel barcodes error" << Logger::endl;
        resetState();
      });
  };

  enqueueOperation(lambda);
}

Promise<void> AutoDevice::Impl::createPayment(uint32_t amount, const std::string& transactionId, const std::string& cashierId, 
  const PaymentMetadata& metadata) {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state != AutoDeviceState::sessionOpened) {
    throw Errors::InvalidState();    
  }

  m_state = AutoDeviceState::waitingForPaymentResult;
  m_paymentPromise = Promise<void>();

  auto lambda = [&, amount, transactionId, cashierId, metadata] {
    m_device.createPayment(amount, transactionId, cashierId, metadata)
      .then([&] {
        executeNextOperation();
      }).catchError([&] (exception_ptr) {
        m_log.e() << "create payment error" << Logger::endl;
        resetState();
      });
  };

  enqueueOperation(lambda);
  return m_paymentPromise;  
}

Promise<std::string> AutoDevice::Impl::createPaymentToken(uint32_t amount, const std::string& transactionId, const std::string& cashierId, 
  const PaymentMetadata& metadata) {
  std::lock_guard<recursive_mutex> guard(m_mutex);

if (m_state != AutoDeviceState::sessionOpened) {
    throw Errors::InvalidState();    
  }

  m_state = AutoDeviceState::waitingForPaymentToken;
  m_paymentTokenPromise = Promise<string>();

  auto lambda = [&, amount, transactionId, cashierId, metadata] {
    m_device.createPaymentToken(amount, transactionId, cashierId, metadata)
      .then([&] {
        executeNextOperation();
      }).catchError([&] (exception_ptr) {
        m_log.e() << "cancel barcodes error" << Logger::endl;
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

  m_state = AutoDeviceState::sessionClosed;
  auto lambda = [&] {
      m_device.confirmPayment()
      .then([&] {
        executeNextOperation();
      }).catchError([&] (exception_ptr) {
        m_log.e() << "confirm payment error" << Logger::endl;
        resetState();
      });
  };

  enqueueOperation(lambda);  
}

void AutoDevice::Impl::cancelPayment() {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state != AutoDeviceState::waitingForConfirmation && 
    m_state != AutoDeviceState::waitingForPaymentResult && 
    m_state != AutoDeviceState::waitingForPaymentToken) {
    throw Errors::InvalidState();
  }

  m_state = AutoDeviceState::sessionClosed;
  auto lambda = [&] {
      m_device.cancelPayment()
      .then([&] {
        executeNextOperation();
      }).catchError([&] (exception_ptr) {
        m_log.e() << "cancel payment error" << Logger::endl;
        resetState();
      });
  };

  enqueueOperation(lambda);    
}

void AutoDevice::Impl::enqueueOperation(const std::function<void ()>& callback) {
  if (m_pendingOperations.empty()) {
    callback();
  }
  m_pendingOperations.push_back(callback);
}

void AutoDevice::Impl::executeNextOperation() {
  m_pendingOperations.pop_back();
  if (!m_pendingOperations.empty()) {
    m_pendingOperations.front()();
  }  
}

void AutoDevice::Impl::onBarcodes(const std::vector<Barcode> &barcodes) {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state != AutoDeviceState::waitingForBarcodes) {
    m_log.e() << "invalid state while received barcodes" << Logger::endl;
    return;
  }

  m_state = AutoDeviceState::sessionClosed;

  if (m_barcodesPromise.state() != PromiseState::undefined) {
    m_log.e() << "invalid promise state while received barcodes" << Logger::endl;
    return;
  }

  m_barcodesPromise.resolve(barcodes);
}

void AutoDevice::Impl::onPaymentError(const PaymentError &error) {
  // TODO: have to properly handle payment errors.
}

void AutoDevice::Impl::onPaymentSuccess() {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state != AutoDeviceState::waitingForPaymentResult) {
    m_log.e() << "invalid state while received payment result" << Logger::endl;
    return;
  }

  m_state = AutoDeviceState::waitingForConfirmation;

  if (m_paymentPromise.state() != PromiseState::undefined) {
    m_log.e() << "invalid promise state while received payment result" << Logger::endl;
  }

  m_paymentPromise.resolve();
}

void AutoDevice::Impl::onPaymentToken(const std::string &token) {
  std::lock_guard<recursive_mutex> guard(m_mutex);

  if (m_state != AutoDeviceState::waitingForPaymentToken) {
    m_log.e() << "invalid state while received payment result" << Logger::endl;
    return;
  }

  m_state = AutoDeviceState::sessionClosed;

  if (m_paymentTokenPromise.state() != PromiseState::undefined) {
    m_log.e() << "invalid promise state while received payment result" << Logger::endl;
  }

  m_paymentTokenPromise.resolve(token);  
}

void AutoDevice::Impl::runLoop() {
  m_io_service.run();
}

void AutoDevice::Impl::notifyStateChange(AutoDeviceState state, exception_ptr exception) {
  auto callback = *m_callback;

  if (callback) {
    callback(m_state, exception);
  }
}

AutoDeviceState AutoDevice::Impl::state() {
  return m_state;
}