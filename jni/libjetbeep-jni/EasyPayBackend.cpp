#include "../../lib/libjetbeep.hpp"
#include "com_jetbeep_EasyPayBackend.h"
#include "jni-utils.hpp"

using namespace std;
using namespace JetBeep;

class EasyPayBackendJni {
public:
  static Logger log;
};

Logger EasyPayBackendJni::log = Logger("easypaybackend-jni");

JNIEXPORT jlong JNICALL Java_com_jetbeep_EasyPayBackend_init(JNIEnv* env, jobject object, jint jenvironment, jstring jmerchantToken) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  JniUtils::storeJvm(env);

  EasyPayHostEnv environment = (EasyPayHostEnv)jenvironment;
  auto merchantToken = JniUtils::getString(env, jmerchantToken);

  auto backend = new EasyPayBackend(environment, merchantToken);
  JniUtils::storeEasyPayBackendJObject(env, object, backend);

  return (jlong)(backend);
}

JNIEXPORT void JNICALL Java_com_jetbeep_EasyPayBackend_free(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  EasyPayBackend* backend = nullptr;
  if (!JniUtils::getEasyPayBackendPointer(env, ptr, &backend)) {
    return;
  }

  JniUtils::releaseEasyPayBackendJObject(env, backend);
  delete backend;
}

JNIEXPORT void JNICALL Java_com_jetbeep_EasyPayBackend_makePayment(
  JNIEnv* env, jobject object, jlong ptr, jstring jtransactionId, jstring jtoken, jint jamountInCoins, jlong jdeviceId, jstring jcashierId) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  EasyPayBackend* backend = nullptr;
  if (!JniUtils::getEasyPayBackendPointer(env, ptr, &backend)) {
    return;
  }

  auto transactionId = JniUtils::getString(env, jtransactionId);
  auto token = JniUtils::getString(env, jtoken);
  uint32_t amountInCoints = (uint32_t)jamountInCoins;
  uint32_t deviceId = (uint32_t)jdeviceId;
  auto cashierId = JniUtils::getString(env, jcashierId);

  try {
    backend->makePayment(transactionId, token, amountInCoints, deviceId, cashierId)
      .then([backend](EasyPayResult result) {
        std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
        auto env = JniUtils::attachCurrentThread();
        if (env == nullptr) {
          EasyPayBackendJni::log.e() << "unable to get env" << Logger::endl;
          return;
        }

        auto object = JniUtils::getEasyPayBackendJObject(backend);
        if (object == nullptr) {
          EasyPayBackendJni::log.e() << "unable to get jobject" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto easyPayBackendClass = env->GetObjectClass(object);
        if (easyPayBackendClass == nullptr) {
          EasyPayBackendJni::log.e() << "unable to get AutoDevice class" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto onNativePaymentResult =
          env->GetMethodID(easyPayBackendClass, "onNativePaymentResult", "(Ljava/lang/String;J)V");
        if (onNativePaymentResult == nullptr) {
          EasyPayBackendJni::log.e() << "unable to get onNativePaymentResult method" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        jstring jerrorString = nullptr;
        jlong jeasyPayTransactionId = 0;
        if (result.isError()) {
          jerrorString = env->NewStringUTF(result.primaryErrorMsg.c_str());
          if (jerrorString == nullptr) {
            EasyPayBackendJni::log.e() << "unable to create jString" << Logger::endl;
            return JniUtils::detachCurrentThread();
          }
        } else {
          jeasyPayTransactionId = result.TransactionId;
        }

        env->CallVoidMethod(object, onNativePaymentResult, jerrorString, jeasyPayTransactionId);
        JniUtils::detachCurrentThread();
      })
      .catchError([backend](exception_ptr error) {
        std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
        auto env = JniUtils::attachCurrentThread();
        if (env == nullptr) {
          EasyPayBackendJni::log.e() << "unable to get env" << Logger::endl;
          return;
        }

        auto object = JniUtils::getEasyPayBackendJObject(backend);
        if (object == nullptr) {
          EasyPayBackendJni::log.e() << "unable to get jobject" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto easyPayBackendClass = env->GetObjectClass(object);
        if (easyPayBackendClass == nullptr) {
          EasyPayBackendJni::log.e() << "unable to get AutoDevice class" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto onNativePaymentResult =
          env->GetMethodID(easyPayBackendClass, "onNativePaymentResult", "(Ljava/lang/String;J)V");
        if (onNativePaymentResult == nullptr) {
          EasyPayBackendJni::log.e() << "unable to get onNativePaymentResult method" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        jstring jerrorString = env->NewStringUTF("io exception");
        if (jerrorString == nullptr) {
          EasyPayBackendJni::log.e() << "unable to create jString" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        JniUtils::detachCurrentThread();
      });
  } catch (...) {
    JniUtils::throwIOException(env, "unable to request payment");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_EasyPayBackend_makeRefund(
  JNIEnv* env, jobject object, jlong ptr, jlong jeasyPayTransactionId, jint jamountInCoins, jlong jdeviceId) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  EasyPayBackend* backend = nullptr;
  if (!JniUtils::getEasyPayBackendPointer(env, ptr, &backend)) {
    return;
  }

  long easyPayTransactionId = (long)jeasyPayTransactionId;
  uint32_t amountInCoins = (uint32_t)jamountInCoins;
  uint32_t deviceId = (uint32_t)jdeviceId;

  try {
    backend->makeRefund(easyPayTransactionId, amountInCoins, deviceId)
      .then([backend](EasyPayResult result) {
        std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
        auto env = JniUtils::attachCurrentThread();
        if (env == nullptr) {
          EasyPayBackendJni::log.e() << "unable to get env" << Logger::endl;
          return;
        }

        auto object = JniUtils::getEasyPayBackendJObject(backend);
        if (object == nullptr) {
          EasyPayBackendJni::log.e() << "unable to get jobject" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto easyPayBackendClass = env->GetObjectClass(object);
        if (easyPayBackendClass == nullptr) {
          EasyPayBackendJni::log.e() << "unable to get AutoDevice class" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto onNativeRefundResult =
          env->GetMethodID(easyPayBackendClass, "onNativeRefundResult", "(Ljava/lang/String;)V");
        if (onNativeRefundResult == nullptr) {
          EasyPayBackendJni::log.e() << "unable to get onNativeRefundResult method" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        jstring jerrorString = nullptr;
        if (result.isError()) {
          jerrorString = env->NewStringUTF(result.primaryErrorMsg.c_str());
          if (jerrorString == nullptr) {
            EasyPayBackendJni::log.e() << "unable to create jString" << Logger::endl;
            return JniUtils::detachCurrentThread();
          }
        }

        env->CallVoidMethod(object, onNativeRefundResult, jerrorString);
        JniUtils::detachCurrentThread();
      })
      .catchError([backend](exception_ptr error) {
        std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
        auto env = JniUtils::attachCurrentThread();
        if (env == nullptr) {
          EasyPayBackendJni::log.e() << "unable to get env" << Logger::endl;
          return;
        }

        auto object = JniUtils::getEasyPayBackendJObject(backend);
        if (object == nullptr) {
          EasyPayBackendJni::log.e() << "unable to get jobject" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto easyPayBackendClass = env->GetObjectClass(object);
        if (easyPayBackendClass == nullptr) {
          EasyPayBackendJni::log.e() << "unable to get AutoDevice class" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto onNativeRefundResult =
          env->GetMethodID(easyPayBackendClass, "onNativeRefundResult", "(Ljava/lang/String;)V");
        if (onNativeRefundResult == nullptr) {
          EasyPayBackendJni::log.e() << "unable to get onNativeRefundResult method" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        jstring jerrorString = env->NewStringUTF("io error");
        if (jerrorString == nullptr) {
          EasyPayBackendJni::log.e() << "unable to create jString" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        env->CallVoidMethod(object, onNativeRefundResult, jerrorString);
        JniUtils::detachCurrentThread();
      });
  } catch (...) {
    JniUtils::throwIOException(env, "unable to make refund");
  }
}