#include <iostream>
#include <algorithm>
#include <cctype>
#include "cmd.h"
#include "../lib/libjetbeep.h"


using namespace std;
using namespace JetBeep;

Logger l("main");
Device d;

string toLowerCase(const string& input) {
	string result = input;

	transform(result.begin(), result.end(), result.begin(), (int (*)(int))std::tolower);

	return result;
}

static void deviceEvent(const DeviceDetectionEvent& event, const DeviceCandidate &candidate) {
	string event_type;

	if (event == DeviceDetectionEvent::added) {
		event_type = "added: ";
	} else {
		event_type = "removed: ";
	}
	l.i() << event_type << candidate.path << " vid: " << candidate.vid << " pid: " << candidate.pid << Logger::endl;
}

int main() {
	Logger::coutEnabled = true;
	Logger::level = LoggerLevel::verbose;
	Cmd cmd;

	DeviceDetection d(deviceEvent);

	l.i() << "starting device detection.." << Logger::endl;
	try {
		d.start();
	} catch (const exception& e) {
		l.e() << e.what() << Logger::endl;
	}

	while (true) {
		string input;

		getline(cin, input);
		input = toLowerCase(input);
		auto splitted = splitString(input);

		if (splitted.empty())	{
			l.e() << "invalid input" << Logger::endl;
			continue;
		}

		string command = splitted.at(0);
		splitted.erase(splitted.begin());
		
		cmd.process(command, splitted);		
	}	
  return 0;
}
