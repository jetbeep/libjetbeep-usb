#include "sync_serial_device.hpp"
#include <future>
#include <chrono>

using namespace std;
using namespace JetBeep;
using namespace boost;
using namespace boost::asio;

#define ENDING "\r\n"
#define ENDING_LEN 2

DFU::SyncSerialDevice::SyncSerialDevice()
  : m_port(m_io_service), m_log("serial_device") {
}

DFU::SyncSerialDevice::~SyncSerialDevice() {
  try {
    m_log.d() << "Closing port" << Logger::endl;
    m_port.close();
  } catch (...) {
  }
}

size_t DFU::SyncSerialDevice::readBytes(void * p_buff, size_t read_size = 1) {
  if (!m_port.is_open()) {
    throw runtime_error("readBytes: port closed");
  }
  size_t resSize = asio::read(m_port, asio::buffer(p_buff, read_size));
  auto logHandle = m_log.v();

  logHandle << "RX " << resSize << " bytes: [ ";
  for (int i = 0; i < resSize; i++) {
    logHandle << *((char *) (p_buff) + i) << " ";
  }
  logHandle  << "]" <<  Logger::endl;
  return resSize;
}

void DFU::SyncSerialDevice::writeBytes(void * p_data, size_t size) {
  if (!m_port.is_open()) {
    throw runtime_error("writeBytes: port closed");
  }
  size_t resSize = asio::write(m_port, asio::buffer(p_data, size));
  m_log.v() << "TX: " << resSize << "bytes" <<  Logger::endl;
  if (resSize != size) {
    m_log.e() << "write less bytes than expected" << Logger::endl;
    throw runtime_error("write less bytes than expected");
  }
}

void DFU::SyncSerialDevice::open(const string& path) {
  m_port.open(path);
  m_port.set_option(serial_port_base::baud_rate(9600));
  m_log.d() << "Port opened: " << path << Logger::endl;
}

void DFU::SyncSerialDevice::close() {
  m_port.close();
  m_log.d() << "Port closed" << Logger::endl;
}

uint32_t DFU::SyncSerialDevice::getDeviceId() {
  return std::stoul(getCmd("deviceId"), NULL, 16);
}

string DFU::SyncSerialDevice::getFirmwareVer() {
  return getCmd("version");
}

string DFU::SyncSerialDevice::getPublicKey() {
  return getCmd("pubKey");
}

void DFU::SyncSerialDevice::enterDFUMode() {
  string cmd = "ENTER_DFU_MODE" ENDING;
  write(m_port, asio::buffer(ENDING)); //to handle case with mcp2200 buffers issue after power on
  write(m_port, asio::buffer(cmd)); 
  m_log.d() << "TX: " <<  cmd;
}

bool DFU::SyncSerialDevice::isBootloaderMode() {
  //TODO reimplement
  auto handle = std::async(std::launch::async, [this]{ (void) getDeviceId(); });
  std::future_status status = handle.wait_for(std::chrono::milliseconds(300));
  //asume that device is in bootloader mode if it does't respond
  return status == std::future_status::timeout;
}

string DFU::SyncSerialDevice::getResponseStr() {
  read_until(m_port, m_readBuffer, ENDING);
  asio::streambuf::const_buffers_type bufs = m_readBuffer.data();
  string response(asio::buffers_begin(bufs), asio::buffers_end(bufs));
  m_readBuffer.consume(m_readBuffer.size());
  return response.substr(0, response.size() - ENDING_LEN);;
}

string DFU::SyncSerialDevice::getCmd(string prop) {
  string cmd = "GET " + prop + ENDING;
  write(m_port, asio::buffer(cmd));
  m_log.d() << "TX: " <<  cmd;
  string response = getResponseStr();
  auto responseParts = Utils::splitString(response);
  if (responseParts.size() != 3) {
    throw runtime_error("Invalid response for " + prop);
  }
  return responseParts.at(2);
}