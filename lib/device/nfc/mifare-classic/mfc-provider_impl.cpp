#include "../utils/platform.hpp"
#include "mfc-provider_impl.hpp"


using namespace std;
using namespace JetBeep::NFC::MifareClassic;

MifareClassicProvider::Impl::Impl(std::weak_ptr<SerialDevice> sDevice_p): m_serialDevice_p(std::move(sDevice_p)) {
};

MifareClassicProvider::Impl::~Impl() {

};