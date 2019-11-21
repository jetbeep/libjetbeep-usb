#include "../../lib/libjetbeep.hpp"

using namespace JetBeep;
using namespace std;

Logger l("main");

int main() {
  Logger::coutEnabled = true;
  Logger::level = LoggerLevel::verbose;
  auto backend = EasyPayBackend(EasyPayHostEnv::Development, "test");

  backend.getPaymentStatus("test")
    .then([=](EasyPayResult result) { l.i() << "Request result: " << result._rawResponse << Logger::endl; })
    .catchError([=](const std::exception_ptr error) {
      try {
        rethrow_exception(error);
      } catch (JetBeep::HttpErrors::NetworkError& e) {
        l.e() << e.what() << Logger::endl;
      } catch (JetBeep::HttpErrors::ServerError& e) {
        l.e() << e.what() << " status code: " << e.statusCode << Logger::endl;
      } catch (std::exception& e) {
        l.e() << e.what() << Logger::endl;
      }
    });

  while (true) {
    string input;
    getline(cin, input);
  }
  return 0;
}