#include "../../lib/libjetbeep.hpp"
#include "com_jetbeep_AutoDevice.h"
#include "jni-utils.hpp"
#include <unordered_map>

using namespace std;
using namespace JetBeep;

class AutoDeviceJni {
public:
  static Logger log;
};

Logger AutoDeviceJni::log = Logger("autodevice-jni");

JNIEXPORT jlong JNICALL Java_com_jetbeep_AutoDevice_init(JNIEnv* env, jobject object) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  JniUtils::storeJvm(env);
  auto device = new AutoDevice();
  JniUtils::storeJObject(env, object, device);
  device->stateCallback = [device](AutoDeviceState state, exception_ptr ptr) {
    std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
    auto env = JniUtils::attachCurrentThread();
    if (env == nullptr) {
      AutoDeviceJni::log.e() << "unable to get env" << Logger::endl;
      return;
    }

    auto object = JniUtils::getJObject(device);
    if (object == nullptr) {
      AutoDeviceJni::log.e() << "unable to get jobject" << Logger::endl;
      return JniUtils::detachCurrentThread();
    }

    auto autoDeviceClass = env->GetObjectClass(object);
    if (autoDeviceClass == nullptr) {
      AutoDeviceJni::log.e() << "unable to get AutoDevice class" << Logger::endl;
      return JniUtils::detachCurrentThread();
    }

    auto onStateChange = env->GetMethodID(autoDeviceClass, "onStateChange", "(Lcom/jetbeep/AutoDevice$State;)V");
    if (onStateChange == nullptr) {
      AutoDeviceJni::log.e() << "unable to get onStateChange method" << Logger::endl;
      return JniUtils::detachCurrentThread();
    }

    auto jState = JniUtils::convertAutoDeviceState(env, state);
    if (jState == nullptr) {
      AutoDeviceJni::log.e() << "unable to get jState" << Logger::endl;
      return JniUtils::detachCurrentThread();
    }

    env->CallVoidMethod(object, onStateChange, jState);

    JniUtils::detachCurrentThread();
  };

  device->mobileCallback = [device](SerialMobileEvent event) {
    std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
    auto env = JniUtils::attachCurrentThread();
    if (env == nullptr) {
      AutoDeviceJni::log.e() << "unable to get env" << Logger::endl;
      return;
    }

    auto object = JniUtils::getJObject(device);
    if (object == nullptr) {
      AutoDeviceJni::log.e() << "unable to get jobject" << Logger::endl;
      return JniUtils::detachCurrentThread();
    }

    auto autoDeviceClass = env->GetObjectClass(object);
    if (autoDeviceClass == nullptr) {
      AutoDeviceJni::log.e() << "unable to get AutoDevice class" << Logger::endl;
      return JniUtils::detachCurrentThread();
    }

    auto onMobileConnectionChange = env->GetMethodID(autoDeviceClass, "onMobileConnectionChange", "(Z)V");
    if (onMobileConnectionChange == nullptr) {
      AutoDeviceJni::log.e() << "unable to get onMobileConnectionChange method" << Logger::endl;
      return JniUtils::detachCurrentThread();
    }

    jboolean isConnected = (jboolean)(event == SerialMobileEvent::connected);
    env->CallVoidMethod(object, onMobileConnectionChange, isConnected);

    JniUtils::detachCurrentThread();
  };

  return (jlong)(device);
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_free(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return;
  }

  JniUtils::releaseJObject(env, device);
  delete device;
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_start(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return;
  }

  try {
    device->start();
  } catch (...) {
    // TODO: handle all exceptions separetely
    JniUtils::throwIllegalStateException(env, "unable to start device");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_stop(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return;
  }

  try {
    device->stop();
  } catch (...) {
    // TODO: handle all exceptions separetely
    JniUtils::throwIllegalStateException(env, "unable to stop device");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_openSession(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return;
  }

  try {
    device->openSession();
  } catch (...) {
    // TODO: handle all exceptions separetely
    JniUtils::throwIllegalStateException(env, "unable to open session");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_closeSession(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return;
  }

  try {
    device->closeSession();
  } catch (...) {
    // TODO: handle all exceptions separetely
    JniUtils::throwIllegalStateException(env, "unable to close session");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_requestBarcodes(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return;
  }

  try {
    device->requestBarcodes()
      .then([device](vector<Barcode> barcodes) {
        std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
        auto env = JniUtils::attachCurrentThread();
        if (env == nullptr) {
          AutoDeviceJni::log.e() << "unable to get env" << Logger::endl;
          return;
        }

        auto object = JniUtils::getJObject(device);
        if (object == nullptr) {
          AutoDeviceJni::log.e() << "unable to get jobject" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto autoDeviceClass = env->GetObjectClass(object);
        if (autoDeviceClass == nullptr) {
          AutoDeviceJni::log.e() << "unable to get AutoDevice class" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto onBarcodeBegin = env->GetMethodID(autoDeviceClass, "onBarcodeBegin", "(I)V");
        if (onBarcodeBegin == nullptr) {
          AutoDeviceJni::log.e() << "unable to get onBarcodeBegin method" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto onBarcodeValue = env->GetMethodID(autoDeviceClass, "onBarcodeValue", "(ILjava/lang/String;I)V");
        if (onBarcodeValue == nullptr) {
          AutoDeviceJni::log.e() << "unable to get onBarcodeValue method" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto onBarcodeEnd = env->GetMethodID(autoDeviceClass, "onBarcodeEnd", "()V");
        if (onBarcodeEnd == nullptr) {
          AutoDeviceJni::log.e() << "unable to get onBarcodeEnd method" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        env->CallVoidMethod(object, onBarcodeBegin, barcodes.size());

        int i = 0;
        for (auto it = barcodes.begin(); it != barcodes.end(); ++it, ++i) {
          jstring jValue = env->NewStringUTF((*it).value.c_str());
          if (jValue == nullptr) {
            AutoDeviceJni::log.e() << "unable to create jString" << Logger::endl;
            return JniUtils::detachCurrentThread();
          }
          jint jType = (jint)(*it).type;

          env->CallVoidMethod(object, onBarcodeValue, i, jValue, jType);
        }

        env->CallVoidMethod(object, onBarcodeEnd);

        JniUtils::detachCurrentThread();
      })
      .catchError([](exception_ptr ptr) {
        // we don't have to handle this error here, as it will be passed to errorCallback as well
      });
  } catch (...) {
    // TODO: handle all exceptions separetely
    JniUtils::throwIllegalStateException(env, "unable to close session");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_cancelBarcodes(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return;
  }

  try {
    device->cancelBarcodes();
  } catch (...) {
    // TODO: handle all exceptions separetely
    JniUtils::throwIllegalStateException(env, "unable to close session");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_createPaymentToken(
  JNIEnv* env, jobject object, jlong ptr, jint jamount, jstring jtransactionId, jstring jcashierId, jobjectArray jmetadataKeys, jobjectArray jmetadataValues) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return;
  }

  PaymentMetadata metadata;
  uint32_t amount = (uint32_t)jamount;
  auto transactionId = JniUtils::getString(env, jtransactionId);
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
    device->createPaymentToken(amount, transactionId, cashierId, metadata)
      .then([device](std::string token) {
        std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
        auto env = JniUtils::attachCurrentThread();
        if (env == nullptr) {
          AutoDeviceJni::log.e() << "unable to get env" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto object = JniUtils::getJObject(device);
        if (object == nullptr) {
          AutoDeviceJni::log.e() << "unable to get jobject" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto autoDeviceClass = env->GetObjectClass(object);
        if (autoDeviceClass == nullptr) {
          AutoDeviceJni::log.e() << "unable to get AutoDevice class" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto onPaymentToken = env->GetMethodID(autoDeviceClass, "onPaymentToken", "(Ljava/lang/String;)V");
        if (onPaymentToken == nullptr) {
          AutoDeviceJni::log.e() << "unable to get onPaymentToken method" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        jstring jToken = env->NewStringUTF(token.c_str());
        if (jToken == nullptr) {
          AutoDeviceJni::log.e() << "unable to create jString" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        env->CallVoidMethod(object, onPaymentToken, jToken);
        JniUtils::detachCurrentThread();
      })
      .catchError([](exception_ptr ptr) {
        // we don't have to handle this error here, as it will be passed to errorCallback as well
      });
  } catch (...) {
    // TODO: handle all exceptions separetely
    JniUtils::throwIllegalStateException(env, "unable to close session");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_cancelPayment(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return;
  }

  try {
    device->cancelPayment();
  } catch (...) {
    // TODO: handle all exceptions separetely
    JniUtils::throwIllegalStateException(env, "unable to close session");
  }
}

JNIEXPORT jobject JNICALL Java_com_jetbeep_AutoDevice_state(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return nullptr;
  }

  auto state = device->state();
  return JniUtils::convertAutoDeviceState(env, state);
}

JNIEXPORT jboolean JNICALL Java_com_jetbeep_AutoDevice_isMobileConnected(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return false;
  }

  return device->isMobileConnected();
}