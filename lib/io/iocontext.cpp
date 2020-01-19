#include "../utils/platform.hpp"
#include "iocontext.hpp"
#include "iocontext_impl.hpp"

using namespace std;
using namespace JetBeep;

IOContext IOContext::context = IOContext();

IOContext::IOContext() : m_impl(new IOContext::Impl()) {
}

IOContext::IOContext(const IOContext& other) : m_impl(other.m_impl) {
}

IOContext& IOContext::operator=(const IOContext& other) {
  m_impl = other.m_impl;
  return *this;
}