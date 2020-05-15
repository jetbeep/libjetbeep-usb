#include "../../lib/libjetbeep.hpp"
#include "./include/com_jetbeep_EasyPayBackend.h"
#include "jni-utils.hpp"
#include <functional>

using namespace std;
using namespace JetBeep;
using namespace std::placeholders;

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

void onPaymentResult(EasyPayResult result, EasyPayBackend* backend) {
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
  //string,long,string
  auto onNativePaymentResult = env->GetMethodID(easyPayBackendClass, "onNativePaymentResult", "(Ljava/lang/String;JLjava/lang/String;)V");
  if (onNativePaymentResult == nullptr) {
    EasyPayBackendJni::log.e() << "unable to get onNativePaymentResult method" << Logger::endl;
    return JniUtils::detachCurrentThread();
  }

  jstring jerrorString = nullptr;
  jlong jeasyPayTransactionId = 0;
  jstring jeasyPayPaymentRequestUid = nullptr;
  if (result.isError()) {
    jerrorString = env->NewStringUTF(result.primaryErrorMsg.c_str());
    if (jerrorString == nullptr) {
      EasyPayBackendJni::log.e() << "unable to create jString" << Logger::endl;
      return JniUtils::detachCurrentThread();
    }
  } else {
    jeasyPayTransactionId = result.TransactionId;
    jeasyPayPaymentRequestUid = env->NewStringUTF(result.PaymentRequestUid.c_str());
    if (jeasyPayPaymentRequestUid == nullptr) {
      EasyPayBackendJni::log.e() << "unable to create jString" << Logger::endl;
      return JniUtils::detachCurrentThread();
    }
  }

  env->CallVoidMethod(object, onNativePaymentResult, jerrorString, jeasyPayTransactionId, jeasyPayPaymentRequestUid);
  JniUtils::detachCurrentThread();
}

void onPaymentCatch(exception_ptr error, EasyPayBackend* backend) {
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
  //string
  auto onNativePaymentError = env->GetMethodID(easyPayBackendClass, "onNativePaymentError", "(Ljava/lang/String;)V");
  if (onNativePaymentError == nullptr) {
    EasyPayBackendJni::log.e() << "unable to get onNativePaymentError method" << Logger::endl;
    return JniUtils::detachCurrentThread();
  }

  string errorMessage = "Невідома системна помилка";
  try {
    rethrow_exception(error);
  } catch (const HttpErrors::RequestError &e) {
    errorMessage = e.what();
  } catch (const HttpErrors::APIError &) {
    errorMessage = "Помилка роботи API серверу";
  } catch (const HttpErrors::ServerError &) {
    errorMessage = "Помилка роботи серверу";
  } catch (const HttpErrors::NetworkError &) {
    errorMessage = "Мережеве з'єднання недоступне";
  } catch (...) {
    errorMessage = "Невідома системна помилка";
  }

  jstring jerrorString = env->NewStringUTF(errorMessage.c_str());
  if (jerrorString == nullptr) {
    EasyPayBackendJni::log.e() << "unable to create jString" << Logger::endl;
    return JniUtils::detachCurrentThread();
  }

  env->CallVoidMethod(object, onNativePaymentError, jerrorString);
  JniUtils::detachCurrentThread();
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
      .then(std::bind(onPaymentResult, _1, backend))
      .catchError(std::bind(onPaymentCatch, _1, backend));
  } catch (...) {
    JniUtils::throwIOException(env, "unable to request payment");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_EasyPayBackend_makePaymentPartials(JNIEnv* env,
                                                                           jobject object,
                                                                           jlong ptr,
                                                                           jstring jtransactionId,
                                                                           jstring jtoken,
                                                                           jint jamountInCoins,
                                                                           jlong jdeviceId,
                                                                           jobjectArray jmetadataKeys,
                                                                           jobjectArray jmetadataValues,
                                                                           jstring jcashierId) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  EasyPayBackend* backend = nullptr;
  if (!JniUtils::getEasyPayBackendPointer(env, ptr, &backend)) {
    return;
  }
  PaymentMetadata metadata;
  auto transactionId = JniUtils::getString(env, jtransactionId);
  auto token = JniUtils::getString(env, jtoken);
  uint32_t amountInCoints = (uint32_t)jamountInCoins;
  uint32_t deviceId = (uint32_t)jdeviceId;
  auto cashierId = JniUtils::getString(env, jcashierId);

  auto metadataSize = env->GetArrayLength(jmetadataKeys);

  for (int i = 0; i < metadataSize; ++i) {
    jstring jkey = (jstring)env->GetObjectArrayElement(jmetadataKeys, i);
    jstring jvalue = (jstring)env->GetObjectArrayElement(jmetadataValues, i);
    string key = JniUtils::getString(env, jkey);
    string value = JniUtils::getString(env, jvalue);

    metadata[key] = value;
  }

  try {
    backend->makePaymentPartials(transactionId, token, amountInCoints, deviceId, metadata, cashierId)
      .then(std::bind(onPaymentResult, _1, backend))
      .catchError(std::bind(onPaymentCatch, _1, backend));
  } catch (...) {
    JniUtils::throwIOException(env, "unable to request payment");
  }
}

void onRefundResult(EasyPayResult result, EasyPayBackend* backend) {
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

  auto onNativeRefundResult = env->GetMethodID(easyPayBackendClass, "onNativeRefundResult", "(Ljava/lang/String;)V");
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
}

void onRefundCatch(exception_ptr error, EasyPayBackend* backend) {
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

  auto onNativeRefundResult = env->GetMethodID(easyPayBackendClass, "onNativeRefundResult", "(Ljava/lang/String;)V");
  if (onNativeRefundResult == nullptr) {
    EasyPayBackendJni::log.e() << "unable to get onNativeRefundResult method" << Logger::endl;
    return JniUtils::detachCurrentThread();
  }

  string errorMessage = "Невідома системна помилка";
  try {
    rethrow_exception(error);
  } catch (const HttpErrors::RequestError &e) {
    errorMessage = e.what();
  } catch (const HttpErrors::APIError &) {
    errorMessage = "Помилка роботи API серверу";
  } catch (const HttpErrors::ServerError &) {
    errorMessage = "Помилка роботи серверу";
  } catch (const HttpErrors::NetworkError &) {
    errorMessage = "Мережеве з'єднання недоступне";
  } catch (...) {
    errorMessage = "Невідома системна помилка";
  }

  jstring jerrorString = env->NewStringUTF(errorMessage.c_str());
  if (jerrorString == nullptr) {
    EasyPayBackendJni::log.e() << "unable to create jString" << Logger::endl;
    return JniUtils::detachCurrentThread();
  }

  env->CallVoidMethod(object, onNativeRefundResult, jerrorString);
  JniUtils::detachCurrentThread();
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
      .then(std::bind(onRefundResult, _1, backend))
      .catchError(std::bind(onRefundCatch, _1, backend));
  } catch (...) {
    JniUtils::throwIOException(env, "unable to make refund");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_EasyPayBackend_makeRefundPartials(
  JNIEnv* env, jobject object, jlong ptr, jstring jeasyPayPaymentRequestUid, jint jamountInCoins, jlong jdeviceId) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  EasyPayBackend* backend = nullptr;
  if (!JniUtils::getEasyPayBackendPointer(env, ptr, &backend)) {
    return;
  }

  string easyPayPaymentRequestUid = JniUtils::getString(env, jeasyPayPaymentRequestUid);
  ;
  uint32_t amountInCoins = (uint32_t)jamountInCoins;
  uint32_t deviceId = (uint32_t)jdeviceId;

  try {
    backend->makeRefundPartials(easyPayPaymentRequestUid, amountInCoins, deviceId)
      .then(std::bind(onRefundResult, _1, backend))
      .catchError(std::bind(onRefundCatch, _1, backend));
  } catch (...) {
    JniUtils::throwIOException(env, "unable to make refund");
  }
}