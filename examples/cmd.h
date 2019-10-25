#ifndef JETBEEP_CMD__H
#define JETBEEP_CMD__H

#include <string>
#include <vector>
#include "../lib/libjetbeep.h"

class Cmd {
  public:
    Cmd();

    void process(const std::string& cmd, const std::vector<std::string>& params);
  private:
    JetBeep::Logger m_log;
    JetBeep::SerialDevice m_device;

    void open(const std::vector<std::string> &params);
    void openSession();
    void closeSession();
    void close();
    void requestBarcodes();
    void createPayment(const std::vector<std::string> &params);
    void createPaymentToken(const std::vector<std::string> &params);
};

#endif