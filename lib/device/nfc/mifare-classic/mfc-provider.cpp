#include "../utils/platform.hpp"
#include "mfc-provider.hpp"
#include "mfc-provider_impl.hpp"

using namespace std;
using namespace JetBeep::NFC::MifareClassic;

MifareClassicProvider::MifareClassicProvider(std::weak_ptr<SerialDevice> device_p) : m_impl(new MifareClassicProvider::Impl(std::move(device_p))) {
}

MifareClassicProvider::~MifareClassicProvider() {}
