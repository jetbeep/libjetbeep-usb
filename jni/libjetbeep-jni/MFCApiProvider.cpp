#include "../../lib/libjetbeep.hpp"
#include "./include/com_jetbeep_nfc_mifare_classic_MFCApiProvider.h"
#include "jni-utils.hpp"

using namespace std;
using namespace JetBeep;
using namespace JetBeep::NFC;
using namespace JetBeep::NFC::MifareClassic;

class MFCApiProviderJni {
public:
  static Logger log;
};

Logger MFCApiProviderJni::log = Logger("mfc_api_provider-jni");

JNIEXPORT void JNICALL Java_com_jetbeep_nfc_mifare_1classic_MFCApiProvider_free(JNIEnv* env, jobject object, jlong ptr) {
  std::lock_guard<recursive_mutex> lock(JniUtils::mutex);
  MifareClassicProvider * provider_p = nullptr;
  if (0 == ptr) {
    JniUtils::throwNullPointerException(env, "MifareClassicProvider pointer is null");
    return;
  }
  provider_p = static_cast<MifareClassicProvider*>((void*)ptr);

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
  //TODO
  return nullptr;
}

JNIEXPORT void JNICALL Java_com_jetbeep_nfc_mifare_1classic_MFCApiProvider_native_1writeBlock(JNIEnv* env, jobject object, jlong ptr, jobject blockData, jobject jKey){
  //TODO
  return;
}
