#include "serial_device.hpp"

using namespace std;
using namespace JetBeep;
using namespace boost;
using namespace boost::asio;

#define ENDING "\r\n"
#define ENDING_LEN 2

DFU::SerialDevice::SerialDevice()
  : m_port(m_io_service), m_log("serial_device"), m_state(SerialDeviceState::idle), m_timer(m_io_service) {
}

DFU::SerialDevice::~SerialDevice() {
  try {
    m_port.close();
  } catch (...) {
  }
}

string DFU::SerialDevice::getResponseStr() {
  read_until(m_port, m_readBuffer, "\r\n");
  asio::streambuf::const_buffers_type bufs = m_readBuffer.data();
  string response(asio::buffers_begin(bufs), asio::buffers_end(bufs));
  m_readBuffer.consume(m_readBuffer.size());
  return response.substr(0, response.size() - ENDING_LEN);;
}

void DFU::SerialDevice::open(const string& path) {
  m_port.open(path);
  m_port.set_option(serial_port_base::baud_rate(9600));
  
}

void DFU::SerialDevice::close() {
  m_port.close();
}

uint32_t DFU::SerialDevice::getDeviceId() {
  string getCmd = "GET deviceId" ENDING;
  write(m_port, asio::buffer(getCmd));
  string response = getResponseStr();
  auto responseParts = Utils::splitString(response);
  if (responseParts.size() != 3) {
    throw runtime_error("Invalid response for deviceId");
  }
  return std::stoul(responseParts.at(2), NULL, 16);
}

string DFU::SerialDevice::getFirmwareVer() {
  string getCmd = "GET version" ENDING;
  write(m_port, asio::buffer(getCmd));
  string response = getResponseStr();
  auto responseParts = Utils::splitString(response);
  if (responseParts.size() != 3) {
    throw runtime_error("Invalid response for firmwareVer");
  }
  return responseParts.at(2);
}