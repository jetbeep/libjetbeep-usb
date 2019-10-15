#include "cmd.h"
#include "../lib/libjetbeep.h"

using namespace std;
using namespace JetBeep;

Cmd::Cmd()
:_l("Cmd") {

}

void Cmd::process(const string& cmd, const vector<string>& params) {
  if (cmd == "exit") {
    exit(0);
  } else if (cmd == "open") {
    open(params);
  } else if (cmd == "close") {
    close();
  } else if (cmd == "open_session" || cmd == "opensession") {
    openSession();
  } else if (cmd == "close_session" || cmd == "closesession") {
    closeSession();
  } else {
    _l.e() << "invalid command: " << cmd << Logger::endl;
  }
}

void Cmd::open(const vector<string>& params) {
  if (params.size() != 1) {
    _l.e() << "please select correct device path. E.g. 'open /dev/tty1'" << Logger::endl;
    return;
  }

  auto path = params.at(0);    
  try {
    _device.open(path);
    _l.i() << "device opened" << Logger::endl;
  } catch (const exception& e) {
    _l.e() << "unable to open device: "<< e.what() << Logger::endl;
  } catch (...) {
    _l.e() << "unable to open device" << Logger::endl;
  }
}

void Cmd::close() {
  try {
    _device.close();
    _l.i() << "device closed" << Logger::endl;
  } catch (const exception& e) {
    _l.e() << "unable to close device: "<< e.what() << Logger::endl;
  } catch (...) {
    _l.e() << "unable to close device" << Logger::endl;
  }
}

void Cmd::openSession() {
  try {
    _device.openSession();
    _l.i() << "session opened" << Logger::endl;
  } catch (const exception& e) {
    _l.e() << "unable to open session: "<< e.what() << Logger::endl;
  } catch (...) {
    _l.e() << "unable to open session" << Logger::endl;
  }  
}

void Cmd::closeSession() {
    try {
    _device.closeSession();
    _l.i() << "session closed" << Logger::endl;
  } catch (const exception& e) {
    _l.e() << "unable to close session: "<< e.what() << Logger::endl;
  } catch (...) {
    _l.e() << "unable to close session" << Logger::endl;
  } 
}