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
    void resetState();
    void openSession();
    void closeSession();
    void close();
    void requestBarcodes();
    void cancelBarcodes();
    void createPayment(const std::vector<std::string> &params);
    void cancelPayment();
    void createPaymentToken(const std::vector<std::string> &params);
    void get(const std::vector<std::string> &params);
    void set(const std::vector<std::string> &params);
    void beginPrivate();
    void commit(const std::vector<std::string> &params);
    void getState();
    
    void errorHandler(const JetBeep::SerialError &error);
    void barcodeHandler(const std::vector<JetBeep::Barcode> &barcodes);
    void paymentErrorHandler(const JetBeep::PaymentError &error);
    void paymentSuccessHandler();
    void paymentTokenHandler(const std::string &token);
    void mobileHandler(const JetBeep::SerialMobileEvent &event);
    void getHandler(const std::string& result);
    void getStateHandler(const JetBeep::SerialGetStateResult& );
};

#endif