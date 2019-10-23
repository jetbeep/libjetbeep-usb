#include "../utils/platform.hpp"

#ifdef PLATFORM_LINUX
#include "detection.hpp"
#include "../utils/logger.hpp"

#include <thread>
#include <unordered_map>


using namespace JetBeep;
using namespace std;

class DeviceDetection::Impl {
  public:
		Impl(DeviceDetectionCallback *callback);

		void start();
		void stop();

		virtual ~Impl();
	private:
		DeviceDetectionCallback *m_callback;
		Logger m_log;
		

		//VidPid getVidPid(const io_service_t &service);
		//std::string getDevicePath(const io_service_t &service);

};


DeviceDetection::Impl::Impl(DeviceDetectionCallback *callback) 
	:m_callback(callback), m_log("detection") {
}

DeviceDetection::Impl::~Impl() {
	stop();
}

void DeviceDetection::Impl::stop() {
	
}

void DeviceDetection::Impl::start() {
	
}


// DeviceDetection

DeviceDetection::DeviceDetection(DeviceDetectionCallback callback)
: callback(callback), m_impl(new Impl(&this->callback)) {}

DeviceDetection::~DeviceDetection() {}

void DeviceDetection::start() { m_impl->start(); }
void DeviceDetection::stop() { m_impl->stop(); }

#endif