#include "../utils/platform.hpp"

#ifdef PLATFORM_OSX
#include "../io/iocontext_impl.hpp"
#include "../utils/logger.hpp"
#include "detection.hpp"

#include <thread>
#include <unordered_map>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/usb/IOUSBLib.h>

using namespace JetBeep;
using namespace std;

class DeviceDetection::Impl {
public:
  Impl(DeviceDetectionCallback* callback, IOContext context);

  void start();
  void stop();

  virtual ~Impl();

private:
  bool m_started;
  IOContext m_context;
  DeviceDetectionCallback* m_callback;
  Logger m_log;
  CFRunLoopRef m_loop;
  io_iterator_t m_iterator;
  std::thread m_thread;
  IONotificationPortRef m_notifyPort;
  std::unordered_map<std::string, std::pair<DeviceCandidate, io_service_t>> m_trackedDevices;

  VidPid getVidPid(const io_service_t& service);
  std::string getDevicePath(const io_service_t& service);

  static void deviceAdded(void* refCon, io_iterator_t iterator);
  static void deviceRemoved(void* refCon, io_service_t service, natural_t messageType, void* messageArgument);

  void runLoop();
};

DeviceDetection::Impl::Impl(DeviceDetectionCallback* callback, IOContext context)
  : m_callback(callback), m_loop(NULL), m_iterator(0), m_notifyPort(NULL), m_log("detection"), m_context(context), m_started(false) {
}

DeviceDetection::Impl::~Impl() {
  stop();
}

void DeviceDetection::Impl::stop() {
  if (!m_started) {
    return;
  }

  if (m_loop != nullptr) {
    CFRunLoopStop(m_loop);
    m_loop = nullptr;
  }

  if (m_thread.joinable()) {
    m_thread.join();
  }

  for (auto it = m_trackedDevices.begin(); it != m_trackedDevices.end(); ++it) {
    auto service = it->second.second;

    IOObjectRelease(service);
  }

  m_started = false;
}

void DeviceDetection::Impl::deviceAdded(void* refCon, io_iterator_t iterator) {
  io_service_t service;
  kern_return_t kr;
  DeviceDetection::Impl* detection = reinterpret_cast<DeviceDetection::Impl*>(refCon);

  while ((service = IOIteratorNext(iterator))) {
    auto vidPid = detection->getVidPid(service);

    if (DeviceDetection::isValidVidPid(vidPid)) {
      DeviceCandidate device = {vidPid.vid, vidPid.pid, detection->getDevicePath(service)};
      io_service_t remove_service;

      kr = IOServiceAddInterestNotification(detection->m_notifyPort, // notifyPort
                                            service,                 // service
                                            kIOGeneralInterest,      // interestType
                                            deviceRemoved,           // callback
                                            detection,               // refCon
                                            &remove_service          // notification
      );

      if (KERN_SUCCESS != kr) {
        detection->m_log.e() << "unable to add device remove callback: " << kr << Logger::endl;
        IOObjectRelease(service);
        continue;
      }

      detection->m_trackedDevices[device.path] = make_pair(device, remove_service);

      detection->m_context.m_impl->ioService.post([&, device] {
        auto callback = *detection->m_callback;
        if (callback != nullptr) {
          callback(DeviceDetectionEvent::added, device);
        }
      });
    }

    IOObjectRelease(service); // yes, you have to release this
  }
}

void DeviceDetection::Impl::deviceRemoved(void* refCon, io_service_t service, natural_t messageType, void* messageArgument) {
  kern_return_t kr;
  DeviceDetection::Impl* detection = reinterpret_cast<DeviceDetection::Impl*>(refCon);

  if (messageType != kIOMessageServiceIsTerminated) {
    return;
  }

  auto path = detection->getDevicePath(service);
  auto devicePair = detection->m_trackedDevices[path];
  auto device = devicePair.first;
  auto notification = devicePair.second;

  detection->m_trackedDevices.erase(path);

  kr = IOObjectRelease(notification);

  if (KERN_SUCCESS != kr) {
    detection->m_log.e() << "unable to relase service in deviceRemoved: " << kr << Logger::endl;
    return;
  }

  detection->m_context.m_impl->ioService.post([&, device] {
    auto callback = *detection->m_callback;

    if (callback != nullptr) {
      callback(DeviceDetectionEvent::removed, device);
    }
  });
}

void DeviceDetection::Impl::start() {
  if (m_started) {
    return;
  }
  m_started = true;

  kern_return_t kr;
  CFMutableDictionaryRef matchingDict;
  CFRunLoopRef gRunLoop;

  matchingDict = IOServiceMatching(kIOSerialBSDServiceValue /*kIOUSBDeviceClassName*/);

  if (matchingDict == NULL) {
    throw runtime_error("unable to find USB class name");
  }

  m_notifyPort = IONotificationPortCreate(kIOMasterPortDefault);

  kr = IOServiceAddMatchingNotification(m_notifyPort,              // notifyPort
                                        kIOFirstMatchNotification, // notificationType
                                        matchingDict,              // matching
                                        deviceAdded,               // callback
                                        this,                      // refCon
                                        &m_iterator                // notification
  );
  if (kr != KERN_SUCCESS) {
    throw runtime_error("unable to create matching notification");
  }

  m_thread = thread(&DeviceDetection::Impl::runLoop, this);
}

void DeviceDetection::Impl::runLoop() {
  deviceAdded(this, m_iterator);
  CFRunLoopSourceRef runLoopSource = nullptr;

  runLoopSource = IONotificationPortGetRunLoopSource(m_notifyPort);

  m_loop = CFRunLoopGetCurrent();
  CFRunLoopAddSource(m_loop, runLoopSource, kCFRunLoopDefaultMode);

  CFRunLoopRun();
}

VidPid DeviceDetection::Impl::getVidPid(const io_service_t& service) {
  int vid = 0, pid = 0;
  CFTypeRef cf_vendor, cf_product;
  VidPid return_value = {0, 0};

  // Search properties among parents of the current modemService
  cf_vendor = (CFTypeRef)IORegistryEntrySearchCFProperty(service, kIOServicePlane, CFSTR("idVendor"), kCFAllocatorDefault,
                                                         kIORegistryIterateRecursively | kIORegistryIterateParents);

  cf_product = (CFTypeRef)IORegistryEntrySearchCFProperty(service, kIOServicePlane, CFSTR("idProduct"), kCFAllocatorDefault,
                                                          kIORegistryIterateRecursively | kIORegistryIterateParents);

  // Decode & print VID & PID
  if (cf_vendor && cf_product && CFNumberGetValue((CFNumberRef)cf_vendor, kCFNumberIntType, &vid) &&
      CFNumberGetValue((CFNumberRef)cf_product, kCFNumberIntType, &pid)) {
  }

  if (cf_vendor)
    CFRelease(cf_vendor);
  if (cf_product)
    CFRelease(cf_product);

  return_value.vid = vid;
  return_value.pid = pid;

  return return_value;
}

string DeviceDetection::Impl::getDevicePath(const io_service_t& service) {
  string result;

  CFStringRef device_path = (CFStringRef)IORegistryEntrySearchCFProperty(
    service, kIOServicePlane, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, kIORegistryIterateRecursively);

  if (!device_path) {
    return string();
  }

  const char* ptr = CFStringGetCStringPtr(device_path, kCFStringEncodingUTF8);
  size_t len = CFStringGetLength(device_path);

  if (ptr != NULL && len != 0) {
    result.assign(ptr, len);
  }

  if (device_path) {
    CFRelease(device_path);
  }

  return result;
}

// DeviceDetection

DeviceDetection::DeviceDetection(IOContext context) : m_impl(new Impl(&this->callback, context)) {
}

DeviceDetection::~DeviceDetection() {
}

void DeviceDetection::start() {
  m_impl->start();
}
void DeviceDetection::stop() {
  m_impl->stop();
}

#endif