#ifndef JETBEEP_IOCONTEXT_IMPL__H
#define JETBEEP_IOCONTEXT_IMPL__H

#include <thread>
#include <boost/asio.hpp>
#include "iocontext.hpp"

namespace JetBeep {
  class IOContext::Impl {
  public:
    Impl();
    virtual ~Impl();

    boost::asio::io_service ioService;
  private:
    std::thread m_thread;    
    boost::asio::io_service::work m_work;

    void runLoop();
  };
}

#endif