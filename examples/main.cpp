#include <iostream>
#include "../lib/libjetbeep.h"

using namespace std;
using namespace JetBeep;

int main() {
Logger::cout_enabled = true;
Logger::level = VERBOSE;

Logger log("main");
DeviceDetection d;

log.i() << "starting device detection.." << Logger::endl;

d.setup();
cin.get();
  return 0;
}
