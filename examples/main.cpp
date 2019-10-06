#include <iostream>
#include "../lib/libjetbeep.h"

using namespace std;
using namespace JetBeep;

Logger l("main");

static void deviceEvent(const DeviceEvent& event, const DeviceCandidate &device) {
	string event_type;

	if (event == ADDED) {
		event_type = "added: ";
	} else {
		event_type = "removed: ";
	}
	l.i() << event_type << device.path << " vid: " << device.vid << " pid: " << device.pid << Logger::endl;
}

int main() {
	Logger::coutEnabled = true;
	Logger::level = VERBOSE;

	DeviceDetection d(deviceEvent);

	l.i() << "starting device detection.." << Logger::endl;
	try {
		d.start();
	} catch (const exception& e) {
		l.e() << e.what() << Logger::endl;
	}

	cin.get();
  return 0;
}
