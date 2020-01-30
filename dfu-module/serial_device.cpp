#include "serial_device.hpp"
#include <future>
#include <chrono>

using namespace std;
using namespace JetBeep;
using namespace boost;
using namespace boost::asio;

#define ENDING "\r\n"
#define ENDING_LEN 2

DFU::SerialDevice::SerialDevice()
  : m_port(m_io_service), m_log("serial_device") {
}

DFU::SerialDevice::~SerialDevice() {
  try {
    m_port.close();
  } catch (...) {
  }
}

size_t DFU::SerialDevice::readBytes(void * p_buff, size_t read_size = 1) {
  if (!m_port.is_open()) {
    throw runtime_error("readBytes: port closed");
  }
  size_t resSize = asio::read(m_port, asio::buffer(p_buff, read_size));
  m_log.d() << "RX: " << resSize << "bytes" <<  Logger::endl;
  return resSize;
}

void DFU::SerialDevice::writeBytes(void * p_data, size_t size) {
  if (!m_port.is_open()) {
    throw runtime_error("writeBytes: port closed");
  }
  size_t resSize = asio::write(m_port, asio::buffer(p_data, size));
  m_log.d() << "TX: " << resSize << "bytes" <<  Logger::endl;
  if (resSize != size) {
    m_log.e() << "write less bytes than expected" << Logger::endl;
    throw runtime_error("write less bytes than expected");
  }
}

void DFU::SerialDevice::open(const string& path) {
  m_port.open(path);
  m_port.set_option(serial_port_base::baud_rate(9600));
  
}

void DFU::SerialDevice::close() {
  m_port.close();
}

uint32_t DFU::SerialDevice::getDeviceId() {
  return std::stoul(getCmd("deviceId"), NULL, 16);
}

string DFU::SerialDevice::getFirmwareVer() {
  return getCmd("version");
}

string DFU::SerialDevice::getPublicKey() {
  return getCmd("pubKey");
}

void DFU::SerialDevice::enterDFUMode() {
  string cmd = "ENTER_DFU_MODE" ENDING;
  write(m_port, asio::buffer(cmd));
  m_log.d() << "TX: " <<  cmd;
}

bool DFU::SerialDevice::isBootloaderMode() {
  //TODO reimplement
  auto handle = std::async(std::launch::async, [this]{ (void) getDeviceId(); });
  std::future_status status = handle.wait_for(std::chrono::milliseconds(300));
  //asume that device is in bootloader mode if it does't respond
  return status == std::future_status::timeout;
}

string DFU::SerialDevice::getResponseStr() {
  read_until(m_port, m_readBuffer, ENDING);
  asio::streambuf::const_buffers_type bufs = m_readBuffer.data();
  string response(asio::buffers_begin(bufs), asio::buffers_end(bufs));
  m_readBuffer.consume(m_readBuffer.size());
  return response.substr(0, response.size() - ENDING_LEN);;
}

string DFU::SerialDevice::getCmd(string prop) {
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