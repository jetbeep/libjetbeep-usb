#include "../utils/platform.hpp"
#include "mfc-provider_impl.hpp"


using namespace std;
using namespace JetBeep::NFC::MifareClassic;

MifareClassicProvider::Impl::Impl(DetectionEventData &cardInfo): m_cardInfo(cardInfo) {
};

MifareClassicProvider::Impl::~Impl() {

};


void MifareClassicProvider::Impl::readBlock(std::shared_ptr<SerialDevice> serial, const int blockNo, const MifareClassicKey &key, MifareBlockContent & content) {

}

void MifareClassicProvider::Impl::writeBlock(std::shared_ptr<SerialDevice> serial, const MifareBlockContent & content, const MifareClassicKey &key) {


}