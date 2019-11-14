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