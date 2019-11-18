#include "promise_test.hpp"

using namespace JetBeep;
using namespace std;

PromiseTest::PromiseTest()
:m_log("promise_test") {

}

void PromiseTest::run() {
  Promise<int> promise(5);
  Promise<void> voidPromise;
  auto p2 = promise;
  Promise<string> delayedPromise;

  promise
    .then<int>([=] (int value) {
      m_log.i() << value << Logger::endl;
      return 6;
    })
    .then<string>([=] (int value) {
      m_log.i() << value << Logger::endl;
      return "value";
    })
    .then<int>([=] (string value) {
      m_log.i() << value << Logger::endl;
      return 7;
    })
    .recover([=] (const exception_ptr& error) {
      m_log.e() <<" IT SHOULDN'T BE CALLED" << Logger::endl;
      return 55;
    })
    .recoverPromise([=] (const exception_ptr & error) {
      m_log.e() <<" IT SHOULDN'T BE CALLED" << Logger::endl;
      return Promise<int>(57);
    })    
    .thenPromise<int, Promise>([=] (int value) {
      auto promise = Promise<int>(123);
      m_log.i() << value << Logger::endl;
      return promise;
    })
    .then([=] (int value) {
      m_log.i() << value << Logger::endl;
    })
    .thenPromise([=] () -> Promise<void> {
      auto promise = Promise<void>();
      promise.resolve();
      m_log.i() << "thenPromise" << Logger::endl;
      return promise;      
    })
    .thenPromise<string, Promise>([=] {
      m_log.i() << "delayed promise begin" << Logger::endl;
      return delayedPromise;
    })
    .then([=] (string value) {
      m_log.i() << "delayed promise: " << value << Logger::endl;
      throw std::runtime_error("test");
    })
    .then([=] () {
      m_log.e() << "then shouldn't be executed!" << Logger::endl;
    })
    .recover([=] (std::exception_ptr error) -> int {
      m_log.i() << "recovered" << Logger::endl;
      std::rethrow_exception(error);
    })
    .catchError([=] (const std::exception_ptr& error) {
      try {
        std::rethrow_exception(error);
      } catch (const std::runtime_error &error) {
        m_log.i() << "caught error: " << error.what() << Logger::endl;
      } catch(...) {
        m_log.e() << "this catch shouldn't be executed!" << Logger::endl;
      }
    });

    m_log.i() << "resolving delayed promise" << Logger::endl;
    delayedPromise.resolve("delayed");
}