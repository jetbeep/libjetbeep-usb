#include "./https_client.hpp"

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


Promise<string> HttpsClient::request() {
    try
    {
        auto const host = "jetbeep.com";
        auto const port = "443";
        auto const target = "/";
        int version = 11;

        net::io_context ioc;
        ssl::context ctx(ssl::context::tlsv12_client);

        //TODO
        // This holds the root certificate used for verification
        //load_root_certificates(ctx);

        // Verify the remote server's certificate
        ctx.set_verify_mode(ssl::verify_none); //TODO add real verification. Suggestion https://github.com/djarek/certify

        tcp::resolver resolver(ioc);
        beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

        if(! SSL_set_tlsext_host_name(stream.native_handle(), host))
        {
            beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
            throw beast::system_error{ec};
        }

        auto const results = resolver.resolve(host, port);
        beast::get_lowest_layer(stream).connect(results);

        stream.handshake(ssl::stream_base::client);

        // Set up an HTTP GET request message
        http::request<http::string_body> req{http::verb::get, target, version};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        http::write(stream, req);

        beast::flat_buffer buffer;

        http::response<http::dynamic_body> res;

        // Receive the HTTP response
        http::read(stream, buffer, res);


        // Gracefully close the stream
        beast::error_code ec;
        stream.shutdown(ec);
        if(ec == net::error::eof)
        {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec = {};
        }
        if(ec)
            throw beast::system_error{ec};

    }
    catch(std::exception const& e)
    {
        //TODO
    }

    m_pending_request = Promise<string>();
    return m_pending_request;
};