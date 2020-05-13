#include "../utils/platform.hpp"
#include "mfc-provider_impl.hpp"
#include <boost/beast/core/detail/base64.hpp>

using namespace std;
using namespace JetBeep::NFC::MifareClassic;

MifareClassicProvider::Impl::Impl(DetectionEventData& cardInfo) : m_cardInfo(cardInfo){};

MifareClassicProvider::Impl::~Impl(){};

JetBeep::Promise<void> MifareClassicProvider::Impl::readBlock(std::shared_ptr<SerialDevice> serial,
                                                              int blockNo,
                                                              const MifareClassicKey* key,
                                                              MifareBlockContent& content) {
  std::lock_guard<recursive_mutex> guard(m_mutex);
  auto result = JetBeep::Promise<void>();
  auto onResult = [blockNo, &content, &result](std::string contentBase64) {
    content.blockNo = blockNo;
    auto decodeResult = boost::beast::detail::base64::decode(content.data, contentBase64.c_str(), contentBase64.size());
    if (decodeResult.first != MFC_BLOCK_SIZE) {
      result.reject(make_exception_ptr(Errors::ProtocolError()));
    }
    result.resolve();
  };
  auto onError = [&result](const std::exception_ptr error) {
    try {
      rethrow_exception(error);
    } catch (Errors::InvalidResponseWithReason& errorWithReason) {
      result.reject(make_exception_ptr(MifareIOException(errorWithReason.getErrorCode())));
    } catch (...) {
      result.reject(make_exception_ptr(Errors::ProtocolError()));
    }
  };
  if (key == nullptr || key->type == MifareClassicKeyType::NONE) {
    serial->nfcReadMFC((uint8_t)blockNo).then(onResult).catchError(onError);
  } else {
    std::string keyBase64Str;
    keyBase64Str.resize(boost::beast::detail::base64::encoded_size(MFC_KEY_SIZE));
    std::string keyTypeStr = key->type == MifareClassicKeyType::KEY_A ? "1" : "2";
    boost::beast::detail::base64::encode((void*)keyBase64Str.c_str(), key->key_data, MFC_KEY_SIZE);
    serial->nfcSecureReadMFC((uint8_t)blockNo, keyBase64Str, keyTypeStr).then(onResult).catchError(onError);
  }

  return result;
}

JetBeep::Promise<void> MifareClassicProvider::Impl::writeBlock(std::shared_ptr<SerialDevice> serial,
                                                               const MifareBlockContent& content,
                                                               const MifareClassicKey* key) {
  std::lock_guard<recursive_mutex> guard(m_mutex);
  auto result = JetBeep::Promise<void>();
  auto onResult = [&]() { result.resolve(); };
  auto onError = [&result](const std::exception_ptr error) {
    try {
      rethrow_exception(error);
    } catch (Errors::InvalidResponseWithReason& error) {
      result.reject(make_exception_ptr(MifareIOException(error.getErrorCode())));
    } catch (...) {
      result.reject(make_exception_ptr(Errors::ProtocolError()));
    }
  };
  if (key == nullptr || key->type == MifareClassicKeyType::NONE) {
    std::string contentBase64;
    contentBase64.resize(boost::beast::detail::base64::encoded_size(MFC_BLOCK_SIZE));
    boost::beast::detail::base64::encode((void*)contentBase64.c_str(), content.data, MFC_BLOCK_SIZE);

    serial->nfcWriteMFC((uint8_t)content.blockNo, contentBase64).then(onResult).catchError(onError);
  } else {
    std::string keyBase64Str;
    keyBase64Str.resize(boost::beast::detail::base64::encoded_size(MFC_KEY_SIZE));
    std::string keyTypeStr = key->type == MifareClassicKeyType::KEY_A ? "1" : "2";
    boost::beast::detail::base64::encode((void*)keyBase64Str.c_str(), key->key_data, MFC_KEY_SIZE);
    std::string contentBase64;
    contentBase64.resize(boost::beast::detail::base64::encoded_size(MFC_BLOCK_SIZE));
    boost::beast::detail::base64::encode((void*)contentBase64.c_str(), content.data, MFC_BLOCK_SIZE);
    serial->nfcSecureWriteMFC((uint8_t)content.blockNo, contentBase64, keyBase64Str, keyTypeStr).then(onResult).catchError(onError);
  }

  return result;
}