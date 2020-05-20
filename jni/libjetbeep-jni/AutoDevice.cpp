#include "../../lib/libjetbeep.hpp"
#include "./include/com_jetbeep_AutoDevice.h"
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
  auto newObject = JniUtils::storeAutoDeviceJObject(env, object, device);
  device->stateCallback = [object = newObject](AutoDeviceState state, exception_ptr ptr) {
    std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
    auto env = JniUtils::attachCurrentThread();
    if (env == nullptr) {
      AutoDeviceJni::log.e() << "unable to get env" << Logger::endl;
      return;
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

  device->mobileCallback = [object = newObject](SerialMobileEvent event) {
    std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
    auto env = JniUtils::attachCurrentThread();
    if (env == nullptr) {
      AutoDeviceJni::log.e() << "unable to get env" << Logger::endl;
      return;
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

    auto isConnected = (jboolean)(event == SerialMobileEvent::connected);
    env->CallVoidMethod(object, onMobileConnectionChange, isConnected);

    JniUtils::detachCurrentThread();
  };

  device->nfcEventCallback = [object = newObject] (const SerialNFCEvent& event, const NFC::DetectionEventData& eventData) {
    std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
    auto env = JniUtils::attachCurrentThread();
    if (env == nullptr) {
      AutoDeviceJni::log.e() << "unable to get env" << Logger::endl;
      return;
    }

    auto autoDeviceClass = env->GetObjectClass(object);
    if (autoDeviceClass == nullptr) {
      AutoDeviceJni::log.e() << "unable to get AutoDevice class" << Logger::endl;
      return JniUtils::detachCurrentThread();
    }
    string jDetectionEventClassName = "com/jetbeep/nfc/DetectionEvent";
    auto onNFCDetectionEvent = env->GetMethodID(autoDeviceClass, "onNFCDetectionEvent", ("(L" + jDetectionEventClassName +";)V").c_str());
    if (onNFCDetectionEvent == nullptr) {
      AutoDeviceJni::log.e() << "unable to get onNFCDetectionEvent method" << Logger::endl;
      return JniUtils::detachCurrentThread();
    }
    auto jCardInfoObj = JniUtils::getJCardInfoObj(env, &eventData);
    string DetectionEventTypeClassName = "com/jetbeep/nfc/DetectionEvent$Event";
    jclass jDetectionEventTypeClass = env->FindClass(DetectionEventTypeClassName.c_str());
    if (jDetectionEventTypeClass == nullptr) {
      AutoDeviceJni::log.e() << "unable to FindClass for jDetectionEventTypeClass class" << Logger::endl;
      return JniUtils::detachCurrentThread();
    }
    jfieldID jDetectionEventFieldId = nullptr;
    switch(event) {
    case JetBeep::SerialNFCEvent::detected:
      jDetectionEventFieldId = env->GetStaticFieldID(jDetectionEventTypeClass, "DETECTED", ("L" +DetectionEventTypeClassName+ ";").c_str());
      break;
    case JetBeep::SerialNFCEvent::removed:
      jDetectionEventFieldId = env->GetStaticFieldID(jDetectionEventTypeClass, "REMOVED", ("L" +DetectionEventTypeClassName+ ";").c_str());
      break;
    default:
      AutoDeviceJni::log.e() << "unknown SerialNFCEvent" << Logger::endl;
      return JniUtils::detachCurrentThread();
    }
    if (jDetectionEventTypeClass == nullptr) {
      AutoDeviceJni::log.e() << "unable to GetStaticFieldID for jDetectionEventTypeClass" << Logger::endl;
      return JniUtils::detachCurrentThread();
    }
    jobject jDetectionEventTypeValueObj = env->GetStaticObjectField(jDetectionEventTypeClass, jDetectionEventFieldId);
    jclass jDetectionEventClass = env->FindClass(jDetectionEventClassName.c_str());
    string constructorSignature = "(L" + DetectionEventTypeClassName + ";Lcom/jetbeep/nfc/CardInfo;)V";
    jmethodID constructorMethodId = env->GetMethodID(jDetectionEventClass, "<init>", constructorSignature.c_str());

    jobject jDetectionEventObj = env->NewObject(jDetectionEventClass, constructorMethodId, jDetectionEventTypeValueObj, jCardInfoObj);

    env->CallVoidMethod(object, onNFCDetectionEvent, jDetectionEventObj);
    JniUtils::detachCurrentThread();
  };

  device->nfcDetectionErrorCallback = [object = newObject] (const NFC::DetectionErrorReason& reason) {
    std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
    auto env = JniUtils::attachCurrentThread();
    if (env == nullptr) {
      AutoDeviceJni::log.e() << "unable to get env" << Logger::endl;
      return;
    }

    auto autoDeviceClass = env->GetObjectClass(object);
    if (autoDeviceClass == nullptr) {
      AutoDeviceJni::log.e() << "unable to get AutoDevice class" << Logger::endl;
      return JniUtils::detachCurrentThread();
    }
    string detectionErrorClassName = "com/jetbeep/nfc/DetectionError";
    string methodSig = "(L" + detectionErrorClassName + ";)V";
    auto onNFCDetectionError = env->GetMethodID(autoDeviceClass, "onNFCDetectionError", methodSig.c_str());
    if (onNFCDetectionError == nullptr) {
      AutoDeviceJni::log.e() << "unable to get onNFCDetectionError method" << Logger::endl;
      return JniUtils::detachCurrentThread();
    }
    jclass jDetectionErrorClass = env->FindClass(detectionErrorClassName.c_str());
    jfieldID fieldId = nullptr;
    switch (reason) {
    case JetBeep::NFC::DetectionErrorReason::MULTIPLE_CARDS:
      fieldId = env->GetStaticFieldID(jDetectionErrorClass, "MULTIPLE_CARDS", ("L" + detectionErrorClassName + ";").c_str());
      break;
    case JetBeep::NFC::DetectionErrorReason::UNSUPPORTED:
      fieldId = env->GetStaticFieldID(jDetectionErrorClass, "UNSUPPORTED", ("L" + detectionErrorClassName + ";").c_str());
      break;
    case JetBeep::NFC::DetectionErrorReason::UNKNOWN: // falls through
    default:
      fieldId = env->GetStaticFieldID(jDetectionErrorClass, "UNKNOWN", ("L" + detectionErrorClassName + ";").c_str());
    }

    jobject jErrorValueObj = env->GetStaticObjectField(jDetectionErrorClass, fieldId);

    env->CallVoidMethod(object, onNFCDetectionError, jErrorValueObj);
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

  auto ptrField = JniUtils::getPtrField(env, object);
  if (ptrField == nullptr) {
    AutoDeviceJni::log.e() << "unable to get ptr field" << Logger::endl;
    return;
  }

  env->SetLongField(object, ptrField, 0);

  JniUtils::releaseAutoDeviceJObject(env, device);
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
  } catch (const Errors::InvalidState& ) {
    JniUtils::throwIllegalStateException(env, "invalid device state");
  } catch (...) {
    JniUtils::throwIOException(env, "system error");
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
  } catch (const Errors::InvalidState& ) {
    JniUtils::throwIllegalStateException(env, "invalid device state");
  } catch (...) {
    JniUtils::throwIOException(env, "system error");
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
  } catch (const Errors::InvalidState& ) {
    JniUtils::throwIllegalStateException(env, "invalid device state");
  } catch (...) {
    JniUtils::throwIOException(env, "system error");
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
  } catch (const Errors::InvalidState& ) {
    JniUtils::throwIllegalStateException(env, "invalid device state");
  } catch (...) {
    JniUtils::throwIOException(env, "system error");
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

        auto object = JniUtils::getAutoDeviceJObject(device);
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
  } catch (const Errors::InvalidState& ) {
    JniUtils::throwIllegalStateException(env, "invalid device state");
  } catch (...) {
    JniUtils::throwIOException(env, "system error");
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
  } catch (const Errors::InvalidState& ) {
    JniUtils::throwIllegalStateException(env, "invalid device state");
  } catch (...) {
    JniUtils::throwIOException(env, "system error");
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

        auto object = JniUtils::getAutoDeviceJObject(device);
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
  } catch (const Errors::InvalidState& ) {
    JniUtils::throwIllegalStateException(env, "invalid device state");
  } catch (...) {
    JniUtils::throwIOException(env, "system error");
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
  } catch (const Errors::InvalidState& ) {
    JniUtils::throwIllegalStateException(env, "invalid device state");
  } catch (...) {
    JniUtils::throwIOException(env, "system error");
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

JNIEXPORT jlong JNICALL Java_com_jetbeep_AutoDevice_deviceId(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return 0;
  }

  return device->deviceId();
}

JNIEXPORT jstring JNICALL Java_com_jetbeep_AutoDevice_version(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return env->NewStringUTF("");
  }

  jstring jDeviceId = env->NewStringUTF(device->version().c_str());
  return jDeviceId;
}

JNIEXPORT jboolean JNICALL Java_com_jetbeep_AutoDevice_isNFCDetected(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return false;
  }

  return device->isNFCDetected();
}

JNIEXPORT jobject JNICALL Java_com_jetbeep_AutoDevice_getNFCCardInfo(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return nullptr;
  }

  JetBeep::NFC::DetectionEventData cardInfo;
  try {
    cardInfo = device->getNFCCardInfo();
    return JniUtils::getJCardInfoObj(env, &cardInfo);
  } catch (const Errors::InvalidState& ) {
    JniUtils::throwIllegalStateException(env, "NFC card is not detected");
  } catch (...) {
    JniUtils::throwIOException(env, "system error");
  }
  return nullptr;
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_enableNFC(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return ;
  }
  try {
    device->enableNFC();
  } catch (const Errors::InvalidState& ) {
    JniUtils::throwIllegalStateException(env, "Enabling/Disabling interfaces is not allowed while session open");
  } catch (...) {
    JniUtils::throwIOException(env, "system error");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_disableNFC(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return ;
  }
  try {
    device->disableNFC();
  } catch (const Errors::InvalidState& ) {
    JniUtils::throwIllegalStateException(env, "Enabling/Disabling interfaces is not allowed while session open");
  } catch (...) {
    JniUtils::throwIOException(env, "system error");
  }
}

JNIEXPORT jobject JNICALL Java_com_jetbeep_AutoDevice_getNFCMifareApiProvider(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return nullptr;
  }
  jobject resultObj = nullptr;
  string providerClassName = "com/jetbeep/nfc/mifare_classic/MFCApiProvider";
  jclass jProviderClass = env->FindClass(providerClassName.c_str());

  try {
    auto provider = device->getNFCMifareApiProvider();
    auto * provider_p = new NFC::MifareClassic::MifareClassicProvider(provider);
    jmethodID constructorMethodId = env->GetMethodID(jProviderClass, "<init>", "(J)V");
    return env->NewObject(jProviderClass, constructorMethodId, (jlong) provider_p);
  } catch (const Errors::InvalidState& ) {
    JniUtils::throwIllegalStateException(env, "Mifare Classic card is not detected");
  } catch (...) {
    JniUtils::throwIOException(env, "system error");
  }
  return resultObj;
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_enableBluetooth(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return ;
  }
  try {
    device->enableBluetooth();
  } catch (const Errors::InvalidState& ) {
    JniUtils::throwIllegalStateException(env, "Enabling/Disabling interfaces is not allowed while session open");
  } catch (...) {
    JniUtils::throwIOException(env, "system error");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_disableBluetooth(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return ;
  }
  try {
    device->disableBluetooth();
  } catch (const Errors::InvalidState& ) {
    JniUtils::throwIllegalStateException(env, "Enabling/Disabling interfaces is not allowed while session open");
  } catch (...) {
    JniUtils::throwIOException(env, "system error");
  }
}