#include "authentication/authentication_http_client.hpp"

#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4267)
#endif
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <openssl/err.h>

namespace pgl::authentication_http {
	namespace {
		namespace asio = boost::asio;
		namespace beast = boost::beast;
		namespace http = beast::http;
		using tcp = asio::ip::tcp;

		struct parsed_url final {
			std::string scheme;
			std::string host;
			std::string port;
			std::string target;
			bool https = false;
		};

		parsed_url parse_url(const std::string& url) {
			const auto scheme_end = url.find("://");
			if (scheme_end == std::string::npos) { throw std::invalid_argument("URL scheme is missing."); }

			parsed_url result;
			result.scheme = url.substr(0, scheme_end);
			result.https = result.scheme == "https";
			if (!result.https && result.scheme != "http") { throw std::invalid_argument("Unsupported URL scheme."); }

			const auto authority_start = scheme_end + 3;
			const auto path_start = url.find('/', authority_start);
			const auto authority = path_start == std::string::npos
				                       ? url.substr(authority_start)
				                       : url.substr(authority_start, path_start - authority_start);
			result.target = path_start == std::string::npos ? "/" : url.substr(path_start);
			if (result.target.empty()) { result.target = "/"; }

			const auto port_separator = authority.rfind(':');
			if (port_separator != std::string::npos && authority.find(']') == std::string::npos) {
				result.host = authority.substr(0, port_separator);
				result.port = authority.substr(port_separator + 1);
			}
			else {
				result.host = authority;
				result.port = result.https ? "443" : "80";
			}
			if (result.host.empty()) { throw std::invalid_argument("URL host is missing."); }
			return result;
		}

		std::string url_encode(const std::string& value) {
			std::ostringstream stream;
			stream << std::hex << std::uppercase;
			for (const auto c : value) {
				const auto byte = static_cast<unsigned char>(c);
				if (std::isalnum(byte) || byte == '-' || byte == '_' || byte == '.' || byte == '~') {
					stream << c;
				}
				else {
					stream << '%' << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
				}
			}
			return stream.str();
		}
	}

	std::string append_query(std::string url, const std::string& key, const std::string& value) {
		url += url.find('?') == std::string::npos ? '?' : '&';
		url += url_encode(key);
		url += '=';
		url += url_encode(value);
		return url;
	}

	response get(const std::string& url, const std::chrono::seconds timeout) {
		const auto parsed = parse_url(url);

		asio::io_context io;
		tcp::resolver resolver(io);
		http::request<http::empty_body> request{http::verb::get, parsed.target, 11};
		request.set(http::field::host, parsed.host);
		request.set(http::field::user_agent, "PlanetaMatchMaker");

		beast::flat_buffer buffer;
		http::response<http::string_body> http_response;
		const auto results = resolver.resolve(parsed.host, parsed.port);

		if (parsed.https) {
			asio::ssl::context ssl_context(asio::ssl::context::tls_client);
			ssl_context.set_default_verify_paths();
			ssl_context.set_verify_mode(asio::ssl::verify_peer);

			beast::ssl_stream<beast::tcp_stream> stream(io, ssl_context);
			if (!SSL_set_tlsext_host_name(stream.native_handle(), parsed.host.c_str())) {
				throw beast::system_error(
					beast::error_code(static_cast<int>(::ERR_get_error()), asio::error::get_ssl_category()));
			}
			beast::get_lowest_layer(stream).expires_after(timeout);
			beast::get_lowest_layer(stream).connect(results);
			stream.handshake(asio::ssl::stream_base::client);
			http::write(stream, request);
			http::read(stream, buffer, http_response);
			beast::error_code ignored_error;
			stream.shutdown(ignored_error);
		}
		else {
			beast::tcp_stream stream(io);
			stream.expires_after(timeout);
			stream.connect(results);
			http::write(stream, request);
			http::read(stream, buffer, http_response);
			beast::error_code ignored_error;
			stream.socket().shutdown(tcp::socket::shutdown_both, ignored_error);
		}

		return {http_response.result_int(), http_response.body()};
	}
}
