#include "iocontext.hpp"
#include "iocontext_impl.hpp"

using namespace std;
using namespace JetBeep;

IOContext IOContext::context = IOContext();

IOContext::IOContext()
:m_impl(new IOContext::Impl()) {

}