#include "jni-utils.hpp"
#include  <cstring>

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

void JniUtils::throwMFCOperationException(JNIEnv* env, JetBeep::NFC::MifareClassic::MifareIOException& error) {
  jclass exClass;
  const char* className = "com/jetbeep/nfc/mifare_classic/MFCOperationException";

  exClass = env->FindClass(className);
  if (exClass == NULL) {
    m_log.e() << "unable to find IOException exception" << Logger::endl;
    return;
  }

  string message = "UNKNOWN";
  switch (error.getIOErrorReason()) {
  case NFC::MifareClassic::MifareIOErrorReason::UNKNOWN:
    message = "UNKNOWN";
    break;
  case NFC::MifareClassic::MifareIOErrorReason::PARAMS_INVALID:
    message = "PARAMS_INVALID";
    break;
  case NFC::MifareClassic::MifareIOErrorReason::KEY_PARAM_INVALID:
    message = "KEY_PARAM_INVALID";
    break;
  case NFC::MifareClassic::MifareIOErrorReason::INTERRUPTED:
    message = "INTERRUPTED";
    break;
  case NFC::MifareClassic::MifareIOErrorReason::DATA_SIZE:
    message = "DATA_SIZE";
    break;
  case NFC::MifareClassic::MifareIOErrorReason::UNSUPPORTED_CARD_TYPE:
    message = "UNSUPPORTED_CARD_TYPE";
    break;
  case NFC::MifareClassic::MifareIOErrorReason::AUTH_ERROR:
    message = "AUTH_ERROR";
    break;
  case NFC::MifareClassic::MifareIOErrorReason::CARD_REMOVED:
    message = "CARD_REMOVED";
    break;
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
    auto objSignatureFun = [&](const string& name) -> string { return "L" + name + ";"; };

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

void JniUtils::getMifareClassicKeyFromMFCKey(JNIEnv* env, jobject jMFCKeyObj, NFC::MifareClassic::MifareClassicKey* key_p) {
  string mfcKeyTypeClassName = "com/jetbeep/nfc/mifare_classic/MFCKey$Type";
  jclass mfcKeyTypeClass = env->FindClass(mfcKeyTypeClassName.c_str());
  jclass mfcKeyClass = env->GetObjectClass(jMFCKeyObj);

  if (mfcKeyClass == nullptr) {
    m_log.e() << "unable to get MFCKey class" << Logger::endl;
    return;
  }

  //set key type
  jfieldID jKeyTypeValueFieldId = env->GetFieldID(mfcKeyClass, "type", ("L" + mfcKeyTypeClassName + ";").c_str());
  jobject jKeyTypeValueObj = env->GetObjectField(jMFCKeyObj, jKeyTypeValueFieldId);
  jint keyInt = env->CallIntMethod(jKeyTypeValueObj, env->GetMethodID(mfcKeyTypeClass, "getValue", "()I"));

  switch ((int)keyInt) {
  case 0:
    key_p->type = NFC::MifareClassic::MifareClassicKeyType::NONE;
    break;
  case 1:
    key_p->type = NFC::MifareClassic::MifareClassicKeyType::KEY_A;
    break;
  case 2:
    key_p->type = NFC::MifareClassic::MifareClassicKeyType::KEY_B;
    break;
  }

  //set key data
  jfieldID jKeyValueArrFieldId = env->GetFieldID(mfcKeyClass, "value", "[B");
  auto jKeyValueArrObj = env->GetObjectField(jMFCKeyObj, jKeyValueArrFieldId);
  jbyte * jKeyValueArrP = env->GetByteArrayElements((jbyteArray)jKeyValueArrObj, 0);
  std::memcpy(key_p->key_data, jKeyValueArrP, MFC_KEY_SIZE);
  env->ReleaseByteArrayElements((jbyteArray)jKeyValueArrObj, jKeyValueArrP, JNI_ABORT);
}

jobject JniUtils::getMFCBlockDataFromMifareBlockContent(JNIEnv* env, NFC::MifareClassic::MifareBlockContent * content_p) {
  string mfcBlockDataClassName = "com/jetbeep/nfc/mifare_classic/MFCBlockData";
  jclass mfcBlockDataClass = env->FindClass(mfcBlockDataClassName.c_str());
  jint blockNo = (jint) content_p->blockNo;

  //create value field
  jbyteArray jByteArr = env->NewByteArray(MFC_BLOCK_SIZE);
  env->SetByteArrayRegion(jByteArr, 0, MFC_BLOCK_SIZE, (jbyte *) content_p->data);

  //create new MFCBlockData
  jmethodID constructorMethodId = env->GetMethodID(mfcBlockDataClass, "<init>", "(I[B)V");
  jobject returnObj = env->NewObject(mfcBlockDataClass, constructorMethodId, blockNo, jByteArr);

  return returnObj;
}

void JniUtils::getMifareBlockContentFromMFCBlockData(JNIEnv* env, jobject jBlockDataObj, NFC::MifareClassic::MifareBlockContent * content_p) {
  jclass jBlockDataClass = env->GetObjectClass(jBlockDataObj);
  if (jBlockDataClass == nullptr) {
    m_log.e() << "unable to get jBlockDataObj class" << Logger::endl;
    return;
  }
  jfieldID jBlockNoFieldId = env->GetFieldID(jBlockDataClass, "blockNo", "I");
  content_p->blockNo = env->GetIntField(jBlockDataObj, jBlockNoFieldId);

  jfieldID jValueArrFieldId = env->GetFieldID(jBlockDataClass, "value", "[B");
  auto jValueArrObj = env->GetObjectField(jBlockDataObj, jValueArrFieldId);
  jbyte * jValueArrP = env->GetByteArrayElements((jbyteArray)jValueArrObj, 0);
  std::memcpy(content_p->data, jValueArrP, MFC_BLOCK_SIZE);
  env->ReleaseByteArrayElements((jbyteArray)jValueArrObj, jValueArrP, JNI_ABORT);
}

