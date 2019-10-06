#include <iostream>
#include "../lib/libjetbeep.h"

using namespace std;
using namespace JetBeep;

Logger l("main");
Device d;

static void deviceEvent(const DeviceDetectionEvent& event, const DeviceCandidate &candidate) {
	string event_type;

	if (event == ADDED) {
		event_type = "added: ";
		d.open(candidate.path);
		d.openSession();
	} else {
		event_type = "removed: ";
	}
	l.i() << event_type << candidate.path << " vid: " << candidate.vid << " pid: " << candidate.pid << Logger::endl;
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
