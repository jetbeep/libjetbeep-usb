#include "./https_client.hpp"

#ifdef HTTP_CLIENT_BOOST_BEAST

#include "../../io/iocontext_impl.hpp"
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

using namespace JetBeep;
using namespace std;

HttpsClient::HttpsClient() : m_log("https_client") {
  m_isCanceled.store(false);
  m_isPending.store(false);
};

HttpsClient::~HttpsClient() {
  m_isCanceled.store(true);
  if (m_thread.joinable()) {
    m_thread.join();
  }
  m_isPending.store(false);
}

Promise<Response> HttpsClient::request(RequestOptions& options) {
  if (m_isPending.load() == true) {
    throw runtime_error("previous request is not completed");
  }
  m_isCanceled.store(false);
  m_isPending.store(true);
  m_pendingRequest = Promise<Response>();
  m_thread = thread(&HttpsClient::doRequest, this, options);
  return m_pendingRequest;
};

void HttpsClient::doRequest(RequestOptions options) {
  m_thread.detach();
  try {
    net::io_context ioc;
    ssl::context ctx(ssl::context::tlsv12_client);

    m_log.d() << "HTTPS request to: " << options.host << ":" << options.port << options.path << Logger::endl;

    // TODO
    // This holds the root certificate used for verification
    // load_root_certificates(ctx);

    // Verify the remote server's certificate
    ctx.set_verify_mode(ssl::verify_none); // TODO add real verification. Suggestion https://github.com/djarek/certify

    tcp::resolver resolver(ioc);
    beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

    if (options.timeout) {
      stream.next_layer().expires_after(std::chrono::milliseconds(options.timeout));
    }

    auto closeStream = [&stream]() { stream.next_layer().close(); };

    if (!SSL_set_tlsext_host_name(stream.native_handle(), options.host.c_str())) {
      beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
      throw beast::system_error{ec};
    }

    auto const results = resolver.resolve(options.host, std::to_string(options.port));
    if (m_isCanceled.load()) {
      return;
    }
    beast::get_lowest_layer(stream).connect(results);
    if (m_isCanceled.load()) {
      return closeStream();
    }
    stream.handshake(ssl::stream_base::client);
    if (m_isCanceled.load()) {
      return closeStream();
    }

    http::verb method;
    switch (options.method) {
    case RequestMethod::GET:
      method = http::verb::get;
      break;
    case RequestMethod::POST:
      method = http::verb::post;
      break;
    case RequestMethod::PATCH:
      method = http::verb::patch;
      break;
    default:
      throw runtime_error("HTTP method not supported");
    }

    http::request<http::string_body> req{method, options.path, HTTP_VERSION};
    req.set(http::field::host, options.host);
    req.set(http::field::user_agent, HTTP_USER_AGENT);

    if (!options.body.empty()) {
      switch (options.contentType) {
      case RequestContentType::JSON: {
        req.set(beast::http::field::content_type, "application/json");
        break;
      }
      default:
        throw runtime_error("Content type not supported");
      }

      req.body() = options.body;
      req.prepare_payload();
    }

    http::write(stream, req);
    if (m_isCanceled.load()) {
      return closeStream();
    }

    beast::flat_buffer buffer;

    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    http::read(stream, buffer, res);
    closeStream();

    Response response;
    response.body = boost::beast::buffers_to_string(res.body().data());
    response.statusCode = res.result_int();
    response.isHttpError = HttpsClient::isErrorStatusCode(response.statusCode);

    m_isPending.store(false);
    m_log.d() << "API response (" << response.statusCode << "): " << response.body << Logger::endl;

    options.ioContext.m_impl->ioService.post([this, response]{
      m_pendingRequest.resolve(response); 
    });

  } catch (std::exception const& e) {
    m_log.e() << e.what() << Logger::endl;
    options.ioContext.m_impl->ioService.post([this]{
      m_pendingRequest.reject(make_exception_ptr(HttpErrors::NetworkError()));
    });
  }
}

#endif