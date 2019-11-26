#ifndef JETBEEP_PROMISE_VOID_HPP
#define JETBEEP_PROMISE_VOID_HPP

#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <type_traits>
#include <vector>

namespace JetBeep {
  template <typename T>
  struct IsPromise : std::false_type {};

  template <typename T>
  class Promise;

  template <typename T>
  struct IsPromise<Promise<T>> : std::true_type {};

  enum class PromiseState { undefined, resolved, rejected };

  template <>
  class Promise<void> {
  public:
    Promise() : m_impl(new Promise<void>::Impl()) {
    }

    void resolve() {
      /* we have to copy shared_ptr as in case when *this* will be reassigned with other value
       during execution of m_resolvedCallbacks and m_rejectCallbacks will be undefined (will cause EXC_BAD_ACCESS)
      */
      auto impl = m_impl;

      if (impl->m_state != PromiseState::undefined) {
        throw std::runtime_error("promise is already resolved");
      }
      impl->m_state = PromiseState::resolved;
      for (auto it = impl->m_resolvedCallbacks.begin(); it != impl->m_resolvedCallbacks.end(); ++it) {
        if (*it) {
          (*it)();
        }
      }
      impl->m_resolvedCallbacks.clear();
      impl->m_rejectedCallbacks.clear();
    }

    void reject(std::exception_ptr error) {
      /* we have to copy shared_ptr as in case when *this* will be reassigned with other value
       during execution of m_resolvedCallbacks and m_rejectCallbacks will be undefined (will cause EXC_BAD_ACCESS)
      */
      auto impl = m_impl;

      if (impl->m_state != PromiseState::undefined) {
        throw std::runtime_error("promise is already resolved");
      }

      impl->m_state = PromiseState::rejected;
      impl->m_error = error;
      for (auto it = impl->m_rejectedCallbacks.begin(); it != impl->m_rejectedCallbacks.end(); ++it) {
        if (*it) {
          (*it)();
        }
      }

      impl->m_resolvedCallbacks.clear();
      impl->m_rejectedCallbacks.clear();
    }

    template <typename ReturnType, typename std::enable_if<!IsPromise<ReturnType>::value, ReturnType>::type* = nullptr>
    Promise<ReturnType> then(std::function<ReturnType()> callback) {
      auto promise = Promise<ReturnType>();
      auto resolveLambda = [=, m_impl = m_impl]() mutable {
        try {
          callback();
          promise.resolve();
        } catch (...) {
          auto error = std::current_exception();
          promise.reject(error);
        }
      };
      auto rejectLambda = [=, m_impl = m_impl]() mutable { promise.reject(m_impl->m_error); };

      switch (m_impl->m_state) {
      case PromiseState::resolved:
        resolveLambda();
        break;
      case PromiseState::rejected:
        rejectLambda();
        break;
      case PromiseState::undefined:
        m_impl->m_resolvedCallbacks.push_back(resolveLambda);
        m_impl->m_rejectedCallbacks.push_back(rejectLambda);
        break;
      }

      return promise;
    }

    Promise<void> then(std::function<void()> callback) {
      auto promise = Promise<void>();
      auto resolveLambda = [=, m_impl = m_impl]() mutable {
        try {
          callback();
          promise.resolve();
        } catch (...) {
          auto error = std::current_exception();
          promise.reject(error);
        }
      };
      auto rejectLambda = [=, m_impl = m_impl]() mutable { promise.reject(m_impl->m_error); };

      switch (m_impl->m_state) {
      case PromiseState::resolved:
        resolveLambda();
        break;
      case PromiseState::rejected:
        rejectLambda();
        break;
      case PromiseState::undefined:
        m_impl->m_resolvedCallbacks.push_back(resolveLambda);
        m_impl->m_rejectedCallbacks.push_back(rejectLambda);
        break;
      }

      return promise;
    }

    Promise<void> thenPromise(std::function<Promise<void>()> callback) {
      Promise<void> promise;

      auto resolveLambda = [=]() mutable {
        try {
          auto resultPromise = callback();

          resultPromise.then([=]() mutable { promise.resolve(); }).catchError([=](std::exception_ptr error) mutable {
            promise.reject(error);
          });
        } catch (...) {
          auto error = std::current_exception();
          promise.reject(error);
        }
      };

      auto rejectLambda = [=, m_impl = m_impl]() mutable { promise.reject(m_impl->m_error); };

      switch (m_impl->m_state) {
      case PromiseState::resolved:
        resolveLambda();
        break;
      case PromiseState::rejected:
        rejectLambda();
        break;
      case PromiseState::undefined:
        m_impl->m_resolvedCallbacks.push_back(resolveLambda);
        m_impl->m_rejectedCallbacks.push_back(rejectLambda);
        break;
      }

      return promise;
    }

    template <typename ReturnType, template <typename> class PromiseType>
    PromiseType<ReturnType> thenPromise(std::function<PromiseType<ReturnType>()> callback) {
      Promise<ReturnType> promise;

      auto resolveLambda = [=, m_impl = m_impl]() mutable {
        try {
          auto resultPromise = callback();

          resultPromise.then([=](ReturnType result) mutable { promise.resolve(result); }).catchError([=](std::exception_ptr error) mutable {
            promise.reject(error);
          });
        } catch (...) {
          auto error = std::current_exception();
          promise.reject(error);
        }
      };
      auto rejectLambda = [=, m_impl = m_impl]() mutable { promise.reject(m_impl->m_error); };

      switch (m_impl->m_state) {
      case PromiseState::resolved:
        resolveLambda();
        break;
      case PromiseState::rejected:
        rejectLambda();
        break;
      case PromiseState::undefined:
        m_impl->m_resolvedCallbacks.push_back(resolveLambda);
        m_impl->m_rejectedCallbacks.push_back(rejectLambda);
        break;
      }

      return promise;
    }

    void catchError(std::function<void(const std::exception_ptr&)> callback) {
      auto rejectLambda = [=, m_impl = m_impl] { callback(m_impl->m_error); };
      switch (m_impl->m_state) {
      case PromiseState::rejected:
        rejectLambda();
        break;
      case PromiseState::undefined:
        m_impl->m_rejectedCallbacks.push_back(rejectLambda);
        break;
      default:
        break;
      }
    }

    Promise<void> recover(std::function<void(const std::exception_ptr&)> callback) {
      Promise<void> promise;
      auto resolveLambda = [=, m_impl = m_impl]() mutable { promise.resolve(); };
      auto rejectLambda = [=, m_impl = m_impl]() mutable {
        try {
          callback(m_impl->m_error);
          promise.resolve();
        } catch (...) {
          auto error = std::current_exception();
          promise.reject(error);
        }
      };

      switch (m_impl->m_state) {
      case PromiseState::resolved:
        resolveLambda();
        break;
      case PromiseState::rejected:
        rejectLambda();
        break;
      case PromiseState::undefined:
        m_impl->m_resolvedCallbacks.push_back(resolveLambda);
        m_impl->m_rejectedCallbacks.push_back(rejectLambda);
        break;
      }

      return promise;
    }

    Promise<void> recoverPromise(std::function<Promise<void>(const std::exception_ptr&)> callback) {
      Promise<void> promise;
      auto resolveLambda = [=, m_impl = m_impl]() mutable { promise.resolve(); };
      auto rejectLambda = [=, m_impl = m_impl]() mutable {
        try {
          auto resultPromise = callback(m_impl->m_error);
          resultPromise.then([=]() mutable { promise.resolve(); }).catchError([=](std::exception_ptr error) mutable {
            promise.reject(error);
          });
        } catch (...) {
          auto error = std::current_exception();
          promise.reject(error);
        }
      };

      switch (m_impl->m_state) {
      case PromiseState::resolved:
        resolveLambda();
        break;
      case PromiseState::rejected:
        rejectLambda();
        break;
      case PromiseState::undefined:
        m_impl->m_resolvedCallbacks.push_back(resolveLambda);
        m_impl->m_rejectedCallbacks.push_back(rejectLambda);
        break;
      }

      return promise;
    }

    Promise(const Promise& other) {
      m_impl = other.m_impl;
    }

    Promise& operator=(const Promise& other) {
      m_impl = other.m_impl;
      return *this;
    }

    static Promise<void> rejected(std::exception_ptr error) {
      Promise<void> promise;
      promise.reject(error);
      return promise;
    }

    PromiseState state() {
      return m_impl->m_state;
    }

  private:
    class Impl {
    public:
      PromiseState m_state;
      std::exception_ptr m_error;
      std::vector<std::function<void()>> m_resolvedCallbacks;
      std::vector<std::function<void()>> m_rejectedCallbacks;

      Impl() : m_state(PromiseState::undefined) {
      }
    };
    std::shared_ptr<Impl> m_impl;
  };
} // namespace JetBeep

#endif