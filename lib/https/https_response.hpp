#ifndef HTTPS_RESPONSE_HPP
#define HTTPS_RESPONSE_HPP


using namespace std;

namespace JetBeep {
  class HTTPResponseBase {
    public:
    string _rawResponse;
    int statusCode;
  };

} // namespace JetBeep

#endif