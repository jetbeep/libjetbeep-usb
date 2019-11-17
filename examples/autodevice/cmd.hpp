#ifndef AUTODEVICE_CMD__H
#define AUTODEVICE_CMD__H

#include "../../lib/libjetbeep.hpp"
#include <string>
#include <vector>

class Cmd {
public:
  void process(const std::string& cmd, const std::vector<std::string>& params);
private:
};

#endif