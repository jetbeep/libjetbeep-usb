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
    return;
  }
  provider_p->opaque = objGlobRef;
}

JNIEXPORT void JNICALL Java_com_jetbeep_nfc_mifare_1classic_MFCApiProvider_free(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  MifareClassicProvider * provider_p = nullptr;
  if (!getMifareClassicProviderPointer(env, ptr, &provider_p)) {
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

JNIEXPORT jobject JNICALL Java_com_jetbeep_nfc_mifare_1classic_MFCApiProvider_native_1readBlock(JNIEnv* env, jobject object, jlong ptr, jint jBlockNo, jobject jKey){
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  MifareClassicProvider * provider_p = nullptr;
  if (!getMifareClassicProviderPointer(env, ptr, &provider_p)) {
    return nullptr;
  }

  try {
    std::promise<void> promiseResolver;
    auto readyFuture = promiseResolver.get_future();

    int blockNo = (int) jBlockNo;
    MifareBlockContent content = {};
    MifareClassicKey key = {};
    key.type = JetBeep::NFC::MifareClassic::MifareClassicKeyType::NONE;

    JniUtils::getMifareClassicKeyFromMFCKey(env, jKey, &key);

    provider_p->readBlock(blockNo,content, &key)
      .then([&promiseResolver]() {
        promiseResolver.set_value();
      })
      .catchError([&promiseResolver](const exception_ptr& ex) {
        promiseResolver.set_exception(ex);
      });

    readyFuture.wait();
    readyFuture.get(); //will throw exception if set_exception

    return JniUtils::getMFCBlockDataFromMifareBlockContent(env, &content);

  } catch (const Errors::InvalidState& ) {
    JniUtils::throwIllegalStateException(env, "invalid device state");
  } catch (JetBeep::NFC::MifareClassic::MifareIOException& error) {
    JniUtils::throwMFCOperationException(env, error);
  } catch (...) {
    MFCApiProviderJni::log.e() << "MFCApiProvider_native_1readBlock result in exception" << Logger::endl;
    JniUtils::throwIOException(env, "system error");
  }
  return nullptr;
}

JNIEXPORT void JNICALL Java_com_jetbeep_nfc_mifare_1classic_MFCApiProvider_native_1writeBlock(JNIEnv* env, jobject object, jlong ptr, jobject blockData, jobject jKey){
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  MifareClassicProvider * provider_p = nullptr;
  if (!getMifareClassicProviderPointer(env, ptr, &provider_p)) {
    return;
  }

  try {
    std::promise<void> promiseResolver;
    auto readyFuture = promiseResolver.get_future();

    MifareBlockContent content = {};
    content.blockNo = -1;
    MifareClassicKey key = {};
    key.type = JetBeep::NFC::MifareClassic::MifareClassicKeyType::NONE;

    JniUtils::getMifareClassicKeyFromMFCKey(env, jKey, &key);
    JniUtils::getMifareBlockContentFromMFCBlockData(env, blockData, &content);

    provider_p->writeBlock(content, &key)
      .then([&promiseResolver]() {
        promiseResolver.set_value();
      })
      .catchError([&promiseResolver](const exception_ptr& ex) {
        promiseResolver.set_exception(ex);
      });

    readyFuture.wait();
    readyFuture.get(); //will throw exception if set_exception

    return;
  } catch (const Errors::InvalidState& ) {
    JniUtils::throwIllegalStateException(env, "invalid device state");
  } catch (JetBeep::NFC::MifareClassic::MifareIOException& error) {
    JniUtils::throwMFCOperationException(env, error);
  } catch (...) {
    MFCApiProviderJni::log.e() << "MFCApiProvider_native_1writeBlock result in exception" << Logger::endl;
    JniUtils::throwIOException(env, "system error");
  }
}
