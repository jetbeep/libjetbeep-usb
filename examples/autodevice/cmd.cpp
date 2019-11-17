#include "cmd.hpp"

using namespace std;
using namespace JetBeep;

Cmd::Cmd():m_log("cmd") {

}

void Cmd::process(const std::string& cmd, const std::vector<std::string>& params) {
  if (cmd == "start") {
    start();
  } else if (cmd == "stop") {
    stop();
  }
}

void Cmd::start() {
  try {
    m_autoDevice.start();
  } catch (...) {
    m_log.e() << "unable to start" << Logger::endl;
  }
}

void Cmd::stop() {
  try {
    m_autoDevice.start();
  } catch (...) {
    m_log.e() << "unable to stop" << Logger::endl;
  }
}