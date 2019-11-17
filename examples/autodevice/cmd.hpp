#ifndef AUTODEVICE_CMD__H
#define AUTODEVICE_CMD__H

#include "../../lib/libjetbeep.hpp"
#include <string>
#include <vector>

class Cmd {
public:
  Cmd();
  void process(const std::string& cmd, const std::vector<std::string>& params);  
private:
  JetBeep::Logger m_log;
  JetBeep::AutoDevice m_autoDevice;
  void start();
  void stop();
  void openSession();
  void closeSession();
  void requestBarcodes();
  void cancelBarcodes();
  void createPayment(const std::vector<std::string>& params);
  void createPaymentToken(const std::vector<std::string>& params);
  void confirmPayment();
  void cancelPayment();
};

#endif