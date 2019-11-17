#ifndef PROMISE__TEST_HPP
#define PROMISE__TEST_HPP

#include "../../lib/libjetbeep.h"

class PromiseTest {
  public:
    PromiseTest();

    void run();
  private:
    JetBeep::Logger m_log;
};


#endif