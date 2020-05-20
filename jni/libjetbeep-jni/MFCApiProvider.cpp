#include "../../lib/libjetbeep.hpp"
#include "./include/com_jetbeep_nfc_mifare_classic_MFCApiProvider.h"
#include "jni-utils.hpp"
#include <future>

using namespace std;
using namespace JetBeep;
using namespace JetBeep::NFC;
using namespace JetBeep::NFC::MifareClassic;

class MFCApiProviderJni {
public:
  static Logger log;
};

Logger MFCApiProviderJni::log = Logger("mfc_api_provider-jni");

static bool getMifareClassicProviderPointer(JNIEnv* env, jlong ptr, MifareClassicProvider** provider_p) {
  if (0 == ptr) {
    JniUtils::throwNullPointerException(env, "MifareClassicProvider pointer is null");
    *provider_p = nullptr;
    return false;
  }
  *provider_p = static_cast<MifareClassicProvider*>((void*)ptr);
  return true;
}


JNIEXPORT void JNICALL Java_com_jetbeep_nfc_mifare_1classic_MFCApiProvider_saveObj(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  auto objGlobRef = env->NewGlobalRef(object);
  MifareClassicProvider * provider_p = nullptr;
  if (!getMifareClassicProviderPointer(env, ptr, &provider_p)) {
    MFCApiProviderJni::log.e() << "unable to getMifareClassicProviderPointer" << Logger::endl;
    return;
  }
  provider_p->opaque = objGlobRef;
}

JNIEXPORT void JNICALL Java_com_jetbeep_nfc_mifare_1classic_MFCApiProvider_free(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  MifareClassicProvider * provider_p = nullptr;
  if (!getMifareClassicProviderPointer(env, ptr, &provider_p)) {
    MFCApiProviderJni::log.e() << "unable to getMifareClassicProviderPointer" << Logger::endl;
    return;
  }

  auto ptrField = JniUtils::getPtrField(env, object, "MifareClassicProvider");
  if (ptrField == nullptr) {
    MFCApiProviderJni::log.e() << "unable to get ptr field" << Logger::endl;
    return;
  }
  env->SetLongField(object, ptrField, 0);

  env->DeleteGlobalRef((jobject)(provider_p->opaque));
  provider_p->opaque = nullptr;
  delete provider_p;
}

JNIEXPORT void JNICALL Java_com_jetbeep_nfc_mifare_1classic_MFCApiProvider_native_1readBlock(JNIEnv* env, jobject object, jlong ptr, jint jBlockNo, jobject jKey){
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  MifareClassicProvider * provider_p = nullptr;
  if (!getMifareClassicProviderPointer(env, ptr, &provider_p)) {
    JniUtils::throwRuntimeException(env, "Unable to getMifareClassicProviderPointer");
    return;
  }

  try {
    int blockNo = (int) jBlockNo;
    MifareBlockContent content = {};
    MifareClassicKey key = {};
    key.type = JetBeep::NFC::MifareClassic::MifareClassicKeyType::NONE;

    JniUtils::getMifareClassicKeyFromMFCKey(env, jKey, &key);
    string callBackName = "onReadResult";
    string callSignature = "(Lcom/jetbeep/nfc/mifare_classic/MFCBlockData;Ljava/lang/Exception;)V";

    provider_p->readBlock(blockNo,content, &key)
      .then([&, result = content, provider_p, callBackName, callSignature]() {
        std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
        auto env = JniUtils::attachCurrentThread();
        if (env == nullptr) {
          MFCApiProviderJni::log.e() << "unable to get env" << Logger::endl;
          return JniUtils::detachCurrentThread();;
        }
        auto jMFCProvider = (jobject) provider_p->opaque;
        if (jMFCProvider == nullptr) {
          MFCApiProviderJni::log.e() << "jMFCProvider == nullptr" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }
        auto jMFCProviderClass = env->GetObjectClass(jMFCProvider);
        if (jMFCProviderClass == nullptr) {
          MFCApiProviderJni::log.e() << "unable to env->GetObjectClass(jMFCProvider)" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto methodId = env->GetMethodID(jMFCProviderClass, callBackName.c_str(), callSignature.c_str());
        if (methodId == nullptr) {
          MFCApiProviderJni::log.e() << "unable to find " << callBackName << " method" << Logger::endl;
          return JniUtils::detachCurrentThread();;
        }
        auto jBlockContent = JniUtils::getMFCBlockDataFromMifareBlockContent(env, &result);
        env->CallVoidMethod(jMFCProvider, methodId, jBlockContent, nullptr);

        return JniUtils::detachCurrentThread();
      })
      .catchError([&, provider_p, callBackName, callSignature](const exception_ptr& ex) {
        std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
        auto env = JniUtils::attachCurrentThread();
        if (env == nullptr) {
          MFCApiProviderJni::log.e() << "unable to get env" << Logger::endl;
          return JniUtils::detachCurrentThread();;
        }
        auto jMFCProvider = (jobject) provider_p->opaque;
        if (jMFCProvider == nullptr) {
          MFCApiProviderJni::log.e() << "jMFCProvider == nullptr" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }
        auto jMFCProviderClass = env->GetObjectClass(jMFCProvider);
        if (jMFCProviderClass == nullptr) {
          MFCApiProviderJni::log.e() << "unable to env->GetObjectClass(jMFCProvider)" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto methodId = env->GetMethodID(jMFCProviderClass, callBackName.c_str(), callSignature.c_str());
        if (methodId == nullptr) {
          MFCApiProviderJni::log.e() << "unable to find " << callBackName << " method" << Logger::endl;
          return JniUtils::detachCurrentThread();;
        }
        auto jExceptionObj = JniUtils::createMFCOperationException(env, ex);
        env->CallVoidMethod(jMFCProvider, methodId, nullptr, jExceptionObj);

        return JniUtils::detachCurrentThread();
      });
  } catch (const Errors::InvalidState& ) {
    JniUtils::throwIllegalStateException(env, "invalid device state");
  } catch (...) {
    MFCApiProviderJni::log.e() << "MFCApiProvider_native_1readBlock result in exception" << Logger::endl;
    JniUtils::throwIOException(env, "system error");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_nfc_mifare_1classic_MFCApiProvider_native_1writeBlock(JNIEnv* env, jobject object, jlong ptr, jobject blockData, jobject jKey){
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  MifareClassicProvider * provider_p = nullptr;
  if (!getMifareClassicProviderPointer(env, ptr, &provider_p)) {
    JniUtils::throwRuntimeException(env, "Unable to getMifareClassicProviderPointer");
    return;
  }

  string callBackName = "onWriteResult";
  string callSignature = "(Ljava/lang/Exception;)V";

  try {
    MifareBlockContent content = {};
    content.blockNo = -1;
    MifareClassicKey key = {};
    key.type = JetBeep::NFC::MifareClassic::MifareClassicKeyType::NONE;

    JniUtils::getMifareClassicKeyFromMFCKey(env, jKey, &key);
    JniUtils::getMifareBlockContentFromMFCBlockData(env, blockData, &content);

    provider_p->writeBlock(content, &key)
      .then([&, result = content, provider_p, callBackName, callSignature]() {
        std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
        auto env = JniUtils::attachCurrentThread();
        if (env == nullptr) {
          MFCApiProviderJni::log.e() << "unable to get env" << Logger::endl;
          return JniUtils::detachCurrentThread();;
        }
        auto jMFCProvider = (jobject) provider_p->opaque;
        if (jMFCProvider == nullptr) {
          MFCApiProviderJni::log.e() << "jMFCProvider == nullptr" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }
        auto jMFCProviderClass = env->GetObjectClass(jMFCProvider);
        if (jMFCProviderClass == nullptr) {
          MFCApiProviderJni::log.e() << "unable to env->GetObjectClass(jMFCProvider)" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto methodId = env->GetMethodID(jMFCProviderClass, callBackName.c_str(), callSignature.c_str());
        if (methodId == nullptr) {
          MFCApiProviderJni::log.e() << "unable to find " << callBackName << " method" << Logger::endl;
          return JniUtils::detachCurrentThread();;
        }
        auto jBlockContent = JniUtils::getMFCBlockDataFromMifareBlockContent(env, &result);
        env->CallVoidMethod(jMFCProvider, methodId, nullptr);

        return JniUtils::detachCurrentThread();
      })
      .catchError([&, provider_p, callBackName, callSignature](const exception_ptr& ex) {
        std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
        auto env = JniUtils::attachCurrentThread();
        if (env == nullptr) {
          MFCApiProviderJni::log.e() << "unable to get env" << Logger::endl;
          return JniUtils::detachCurrentThread();;
        }
        auto jMFCProvider = (jobject) provider_p->opaque;
        if (jMFCProvider == nullptr) {
          MFCApiProviderJni::log.e() << "jMFCProvider == nullptr" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }
        auto jMFCProviderClass = env->GetObjectClass(jMFCProvider);
        if (jMFCProviderClass == nullptr) {
          MFCApiProviderJni::log.e() << "unable to env->GetObjectClass(jMFCProvider)" << Logger::endl;
          return JniUtils::detachCurrentThread();
        }

        auto methodId = env->GetMethodID(jMFCProviderClass, callBackName.c_str(), callSignature.c_str());
        if (methodId == nullptr) {
          MFCApiProviderJni::log.e() << "unable to find " << callBackName << " method" << Logger::endl;
          return JniUtils::detachCurrentThread();;
        }
        auto jExceptionObj = JniUtils::createMFCOperationException(env, ex);
        env->CallVoidMethod(jMFCProvider, methodId, jExceptionObj);

        return JniUtils::detachCurrentThread();
      });
    return;
  } catch (const Errors::InvalidState& ) {
    JniUtils::throwIllegalStateException(env, "invalid device state");
  } catch (...) {
    MFCApiProviderJni::log.e() << "MFCApiProvider_native_1writeBlock result in exception" << Logger::endl;
    JniUtils::throwIOException(env, "system error");
  }
}
