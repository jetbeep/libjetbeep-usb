#include "iocontext.hpp"

using namespace std;

IOContext::default = IOContext();

IOContext::IOContext()
:m_impl(new IOContext::Impl()) {

}