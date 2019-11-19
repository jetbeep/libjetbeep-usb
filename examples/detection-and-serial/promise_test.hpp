#ifndef PROMISE__TEST_HPP
#define PROMISE__TEST_HPP

#include "../../lib/libjetbeep.hpp"

class PromiseTest {
public:
  PromiseTest();

  void run();

private:
  JetBeep::Logger m_log;
};

#endif