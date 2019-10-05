#include <iostream>
#include "../lib/libjetbeep.h"

using namespace std;
using namespace jetbeep;

int main() {
logger::cout_enabled = true;
logger::level = VERBOSE;

logger log("main");
DeviceDetection d;

log.i() << "starting device detection.." << logger::endl;

d.setup();
cin.get();
  return 0;
}
