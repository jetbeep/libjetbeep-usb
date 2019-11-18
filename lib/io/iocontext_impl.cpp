#include "iocontext_impl.hpp"

using namespace JetBeep;
using namespace std;

IOContext::Impl::Impl()
:m_thread(&IOContext::Impl::runLoop, this), m_work(ioService) {

}

IOContext::Impl::~Impl() {
  ioService.stop();
  m_thread.join();
}

void IOContext::Impl::runLoop() {
  ioService.run();
}