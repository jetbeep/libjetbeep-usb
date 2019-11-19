#ifndef JETBEEP__PAYMENT_ERROR_H
#define JETBEEP__PAYMENT_ERROR_H

namespace JetBeep {
  enum class PaymentError { network, timeout, server, security, withdrawal, discarded, unknown, invalidPin };
}

#endif