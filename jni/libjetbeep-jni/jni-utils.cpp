#include "jni-utils.hpp"

using namespace std;
using namespace JetBeep;

Logger JniUtils::m_log = Logger("jni-utils");
JavaVM* JniUtils::m_jvm = nullptr;
recursive_mutex JniUtils::mutex = recursive_mutex();

void JniUtils::throwIllegalStateException(JNIEnv* env, const std::string& message) {
  jclass exClass;
  const char* className = "java/lang/IllegalStateException";

  exClass = env->FindClass(className);
  if (exClass == NULL) {
    m_log.e() << "unable to find illegalstate exception" << Logger::endl;
    return;
  }

  if (env->ThrowNew(exClass, message.c_str()) != 0) {
    m_log.e() << "unable to throwNew" << Logger::endl;
    return;
  }
}

void JniUtils::throwNullPointerException(JNIEnv* env, const std::string& message) {
  jclass exClass;
  const char* className = "java/lang/NullPointerException";

  exClass = env->FindClass(className);
  if (exClass == NULL) {
    m_log.e() << "unable to find NullPointerException exception" << Logger::endl;
    return;
  }

  if (env->ThrowNew(exClass, message.c_str()) != 0) {
    m_log.e() << "unable to throwNew" << Logger::endl;
    return;
  }
}

void JniUtils::throwIOException(JNIEnv* env, const std::string& message) {
  jclass exClass;
  const char* className = "java/io/IOException";

  exClass = env->FindClass(className);
  if (exClass == NULL) {
    m_log.e() << "unable to find IOException exception" << Logger::endl;
    return;
  }

  if (env->ThrowNew(exClass, message.c_str()) != 0) {
    m_log.e() << "unable to throwNew" << Logger::endl;
    return;
  }
}

std::string JniUtils::getString(JNIEnv* env, jstring string) {
  auto cstr = env->GetStringUTFChars(string, nullptr);
  std::string returnString = std::string(cstr == nullptr ? "" : cstr);
  env->ReleaseStringUTFChars(string, cstr);
  return returnString;
}

void JniUtils::storeJvm(JNIEnv* env) {
  if (m_jvm != nullptr) {
    return;
  }

  env->GetJavaVM(&m_jvm);
}

JavaVM* JniUtils::getJvm() {
  return m_jvm;
}

JNIEnv* JniUtils::attachCurrentThread() {
  auto jvm = JniUtils::getJvm();
  JNIEnv* env = nullptr;
  jint errorCode = 0;

  if (jvm == nullptr) {
    m_log.e() << "attachCurrentThread: JVM is null!" << Logger::endl;
    return nullptr;
  }

  errorCode = jvm->AttachCurrentThread((void**)&env, nullptr);
  if (errorCode != JNI_OK) {
    m_log.e() << "unable to attach current thread: " << errorCode << Logger::endl;
    return nullptr;
  }

  return env;
}

void JniUtils::detachCurrentThread() {
  auto jvm = JniUtils::getJvm();
  jint errorCode = 0;

  if (jvm == nullptr) {
    m_log.e() << "detachCurrentThread: JVM is null!" << Logger::endl;
    return;
  }

  errorCode = jvm->DetachCurrentThread();
  if (errorCode != JNI_OK) {
    m_log.e() << "unable to detach current thread: " << errorCode << Logger::endl;
    return;
  }
}

jobject JniUtils::convertAutoDeviceState(JNIEnv* env, const AutoDeviceState& state) {
  string className = "com/jetbeep/AutoDevice$State";
  jclass jAutoDeviceState = env->FindClass(className.c_str());
  jobject returnValue = nullptr;
  jfieldID field = nullptr;
  auto signatureFun = [&]() -> string { return "L" + className + ";"; };

  switch (state) {
  case AutoDeviceState::invalid:
    field = env->GetStaticFieldID(jAutoDeviceState, "invalid", signatureFun().c_str());
    returnValue = env->GetStaticObjectField(jAutoDeviceState, field);
    break;
  case AutoDeviceState::firmwareVersionNotSupported:
    field = env->GetStaticFieldID(jAutoDeviceState, "firmwareVersionNotSupported", signatureFun().c_str());
    returnValue = env->GetStaticObjectField(jAutoDeviceState, field);
    break;
  case AutoDeviceState::sessionOpened:
    field = env->GetStaticFieldID(jAutoDeviceState, "sessionOpened", signatureFun().c_str());
    returnValue = env->GetStaticObjectField(jAutoDeviceState, field);
    break;
  case AutoDeviceState::sessionClosed:
    field = env->GetStaticFieldID(jAutoDeviceState, "sessionClosed", signatureFun().c_str());
    returnValue = env->GetStaticObjectField(jAutoDeviceState, field);
    break;
  case AutoDeviceState::waitingForBarcodes:
    field = env->GetStaticFieldID(jAutoDeviceState, "waitingForBarcodes", signatureFun().c_str());
    returnValue = env->GetStaticObjectField(jAutoDeviceState, field);
    break;
  case AutoDeviceState::waitingForPaymentResult:
    field = env->GetStaticFieldID(jAutoDeviceState, "waitingForPaymentResult", signatureFun().c_str());
    returnValue = env->GetStaticObjectField(jAutoDeviceState, field);
    break;
  case AutoDeviceState::waitingForConfirmation:
    field = env->GetStaticFieldID(jAutoDeviceState, "waitingForConfirmation", signatureFun().c_str());
    returnValue = env->GetStaticObjectField(jAutoDeviceState, field);
    break;
  case AutoDeviceState::waitingForPaymentToken:
    field = env->GetStaticFieldID(jAutoDeviceState, "waitingForPaymentToken", signatureFun().c_str());
    returnValue = env->GetStaticObjectField(jAutoDeviceState, field);
    break;
  }

  return returnValue;
}

bool JniUtils::getAutoDevicePointer(JNIEnv* env, jlong ptr, AutoDevice** autoDevice) {
  if (0 == ptr) {
    JniUtils::throwNullPointerException(env, "AutoDevice pointer is null");
    *autoDevice = nullptr;
    return false;
  }
  *autoDevice = static_cast<AutoDevice*>((void*)ptr);
  return true;
}

jobject JniUtils::storeAutoDeviceJObject(JNIEnv* env, jobject object, AutoDevice* autoDevice) {
  auto newObject = env->NewGlobalRef(object);
  autoDevice->opaque = newObject;
  return newObject;
}
void JniUtils::releaseAutoDeviceJObject(JNIEnv* env, AutoDevice* autoDevice) {
  env->DeleteGlobalRef(JniUtils::getAutoDeviceJObject(autoDevice));
  autoDevice->opaque = nullptr;
}

jobject JniUtils::getAutoDeviceJObject(AutoDevice* autoDevice) {
  return (jobject)(autoDevice->opaque);
}

bool JniUtils::getEasyPayBackendPointer(JNIEnv* env, jlong ptr, EasyPayBackend** backend) {
  if (0 == ptr) {
    JniUtils::throwNullPointerException(env, "EasyPayBackend pointer is null");
    *backend = nullptr;
    return false;
  }
  *backend = static_cast<EasyPayBackend*>((void*)ptr);
  return true;
}

void JniUtils::storeEasyPayBackendJObject(JNIEnv* env, jobject object, EasyPayBackend* backend) {
  backend->opaque = env->NewGlobalRef(object);
}
void JniUtils::releaseEasyPayBackendJObject(JNIEnv* env, EasyPayBackend* backend) {
  env->DeleteGlobalRef(JniUtils::getEasyPayBackendJObject(backend));
  backend->opaque = nullptr;
}

jobject JniUtils::getEasyPayBackendJObject(EasyPayBackend* backend) {
  return (jobject)(backend->opaque);
}

jfieldID JniUtils::getPtrField(JNIEnv* env, jobject object, std::string classAlias) {
  auto jClass = env->GetObjectClass(object);
  if (jClass == nullptr) {
    m_log.e() << "unable to get " << classAlias <<" class" << Logger::endl;
    return nullptr;
  }

  auto ptrField = env->GetFieldID(jClass, "ptr", "J");
  if (ptrField == nullptr) {
    m_log.e() << "unable to get ptr field" << Logger::endl;
    return nullptr;
  }
  return ptrField;
}

jobject JniUtils::getJCardInfoObj(JNIEnv* env, const NFC::DetectionEventData* detectionEventData) {
    string cardInfoClassName = "com/jetbeep/nfc/CardInfo";
    string cardInfoTypeClassName = "com/jetbeep/nfc/CardInfo$Type";

    jclass jCardInfo = env->FindClass(cardInfoClassName.c_str());
    jclass jCardInfoType = env->FindClass(cardInfoTypeClassName.c_str());
    jobject jTypeValue = nullptr;
    jfieldID fieldId = nullptr;
    auto objSignatureFun = [&](string name) -> string { return "L" + name + ";"; };

    switch(detectionEventData->cardType) {
        case JetBeep::NFC::CardType::UNKNOWN:
            fieldId = env->GetStaticFieldID(jCardInfo, "UNKNOWN", objSignatureFun(cardInfoTypeClassName).c_str());
            break;
        case JetBeep::NFC::CardType::EMV_CARD:
            fieldId = env->GetStaticFieldID(jCardInfo, "EMV_CARD", objSignatureFun(cardInfoTypeClassName).c_str());
            break;
        case JetBeep::NFC::CardType::MIFARE_CLASSIC_1K:
            fieldId = env->GetStaticFieldID(jCardInfo, "MIFARE_CLASSIC_1K", objSignatureFun(cardInfoTypeClassName).c_str());
            break;
        case JetBeep::NFC::CardType::MIFARE_CLASSIC_4K:
            fieldId = env->GetStaticFieldID(jCardInfo, "MIFARE_CLASSIC_4K", objSignatureFun(cardInfoTypeClassName).c_str());
            break;
        case JetBeep::NFC::CardType::MIFARE_PLUS_2K:
            fieldId = env->GetStaticFieldID(jCardInfo, "MIFARE_PLUS_2K", objSignatureFun(cardInfoTypeClassName).c_str());
            break;
        case JetBeep::NFC::CardType::MIFARE_PLUS_4K:
            fieldId = env->GetStaticFieldID(jCardInfo, "MIFARE_PLUS_4K", objSignatureFun(cardInfoTypeClassName).c_str());
            break;
        case JetBeep::NFC::CardType::MIFARE_DESFIRE_2K:
            fieldId = env->GetStaticFieldID(jCardInfo, "MIFARE_DESFIRE_2K", objSignatureFun(cardInfoTypeClassName).c_str());
            break;
        case JetBeep::NFC::CardType::MIFARE_DESFIRE_4K:
            fieldId = env->GetStaticFieldID(jCardInfo, "MIFARE_DESFIRE_4K", objSignatureFun(cardInfoTypeClassName).c_str());
            break;
        case JetBeep::NFC::CardType::MIFARE_DESFIRE_8K:
            fieldId = env->GetStaticFieldID(jCardInfo, "MIFARE_DESFIRE_8K", objSignatureFun(cardInfoTypeClassName).c_str());
            break;
        default:
            fieldId = env->GetStaticFieldID(jCardInfo, "UNKNOWN", objSignatureFun(cardInfoTypeClassName).c_str());
    }

    jTypeValue = env->GetStaticObjectField(jCardInfoType, fieldId);
    jstring jMetaStr = env->NewStringUTF(detectionEventData->meta.c_str());

    string constructorSignature = "(" + objSignatureFun(cardInfoTypeClassName) + JSTRING_SIGNATURE + ")V";
    jmethodID constructorMethodId = env->GetMethodID(jCardInfo, "<init>", constructorSignature.c_str());

    return env->NewObject(jCardInfo, constructorMethodId, jTypeValue, jMetaStr);
    //DO WE NEED NEW GLOBAL REF?
}
