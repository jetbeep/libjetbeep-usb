#include "../../lib/libjetbeep.hpp"
#include "cmd.hpp"

using namespace JetBeep;
using namespace std;

Logger l("main");

int main() {
  Logger::coutEnabled = true;
	Logger::level = LoggerLevel::verbose;
  Cmd cmd;

  while (true) {
		string input;

		getline(cin, input);
		input = Utils::toLowerCase(input);
		auto splitted = Utils::splitString(input);

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