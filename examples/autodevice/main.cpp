#include "../../lib/libjetbeep.hpp"

using namespace JetBeep;
using namespace std;

int main() {
  Logger::coutEnabled = true;
	Logger::level = LoggerLevel::verbose;  

  return 0;
}