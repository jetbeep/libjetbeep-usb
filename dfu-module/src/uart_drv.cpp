#include "uart_drv.h"
#include "logging.h"

#include "libjetbeep.hpp"
#include "sync_serial_device.hpp"

static JetBeep::Logger l("uart_drv");

#define READ_CHUNK_SIZE 1

using namespace DFU;

int uart_drv_send(uart_drv_t* p_uart, const uint8_t* pData, uint32_t nSize) {
  try {
    if (!p_uart || !p_uart->p_serial_device) {
      throw std::runtime_error("uart_drv_t* p_uart is NULL");
    }
    SyncSerialDevice * p_sd = (SyncSerialDevice *) p_uart->p_serial_device;
    p_sd->writeBytes((void *) pData, nSize);
  } catch (std::exception &e) {
    l.w() << e.what() << JetBeep::Logger::endl;
    return -1;
  }
  return 0;
}

int uart_drv_receive(uart_drv_t* p_uart, uint8_t* pData, uint32_t nSize, uint32_t* pSize) {
  try {
    if (!p_uart || !p_uart->p_serial_device) {
      throw std::runtime_error("uart_drv_t* p_uart is NULL");
    }
    SyncSerialDevice * p_sd = (SyncSerialDevice *) p_uart->p_serial_device;
    *pSize = p_sd->readBytes((void *) pData, READ_CHUNK_SIZE);
  } catch (std::exception &e) {
    l.w() << e.what() << JetBeep::Logger::endl;
    return -1;
  }
  return 0;
}
