#include "authentication/authentication_http_client.hpp"

#include <cctype>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wincrypt.h>
#ifdef _MSC_VER
#pragma comment(lib, "Crypt32.lib")
#endif
#endif

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
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509err.h>

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

		tcp::resolver::results_type resolve(
			const parsed_url& parsed,
			const authentication_execution_context& context) {
			auto resolver = std::make_shared<tcp::resolver>(context.executor);
			asio::steady_timer timer(context.executor, context.deadline);
			timer.async_wait([resolver](const boost::system::error_code& error) {
				if (!error) { resolver->cancel(); }
			});

			boost::system::error_code error;
			auto results = resolver->async_resolve(parsed.host, parsed.port, context.yield[error]);
			timer.cancel();
			if (error) { throw boost::system::system_error(error); }
			return results;
		}

		void close_socket(tcp::socket& socket) {
			boost::system::error_code ignored_error;
			socket.shutdown(tcp::socket::shutdown_both, ignored_error);
			socket.close(ignored_error);
		}

		void throw_last_openssl_error() {
			throw beast::system_error(
				beast::error_code(static_cast<int>(::ERR_get_error()), asio::error::get_ssl_category()));
		}

#ifdef _WIN32
		void load_windows_certificate_store(asio::ssl::context& ssl_context, const DWORD store_location) {
			const auto certificate_store = CertOpenStore(CERT_STORE_PROV_SYSTEM_W, 0,
				static_cast<HCRYPTPROV_LEGACY>(0),
				store_location | CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG, L"ROOT");
			if (certificate_store == nullptr) {
				throw std::system_error(static_cast<int>(GetLastError()), std::system_category(),
					"Failed to open the Windows ROOT certificate store");
			}

			PCCERT_CONTEXT certificate_context = nullptr;
			try {
				auto* openssl_certificate_store = SSL_CTX_get_cert_store(ssl_context.native_handle());
				while ((certificate_context = CertEnumCertificatesInStore(
					certificate_store, certificate_context)) != nullptr) {
					if (certificate_context->cbCertEncoded >
						static_cast<DWORD>(std::numeric_limits<long>::max())) {
						continue;
					}

					const auto* encoded_certificate = certificate_context->pbCertEncoded;
					auto* certificate = d2i_X509(nullptr, &encoded_certificate,
						static_cast<long>(certificate_context->cbCertEncoded));
					if (certificate == nullptr) {
						ERR_clear_error();
						continue;
					}

					const auto add_result = X509_STORE_add_cert(openssl_certificate_store, certificate);
					X509_free(certificate);
					if (add_result != 1) {
						const auto error = ERR_peek_last_error();
						if (ERR_GET_LIB(error) == ERR_LIB_X509 &&
							ERR_GET_REASON(error) == X509_R_CERT_ALREADY_IN_HASH_TABLE) {
							ERR_clear_error();
						}
						else {
							throw_last_openssl_error();
						}
					}
				}

				const auto enumeration_error = GetLastError();
				if (enumeration_error != static_cast<DWORD>(CRYPT_E_NOT_FOUND)) {
					throw std::system_error(static_cast<int>(enumeration_error), std::system_category(),
						"Failed to enumerate the Windows ROOT certificate store");
				}
			}
			catch (...) {
				if (certificate_context != nullptr) { CertFreeCertificateContext(certificate_context); }
				CertCloseStore(certificate_store, 0);
				throw;
			}

			CertCloseStore(certificate_store, 0);
		}
#endif

		void load_system_trusted_certificate_authorities(asio::ssl::context& ssl_context) {
#ifdef _WIN32
			load_windows_certificate_store(ssl_context, CERT_SYSTEM_STORE_LOCAL_MACHINE);
			load_windows_certificate_store(ssl_context, CERT_SYSTEM_STORE_CURRENT_USER);
#else
			ssl_context.set_default_verify_paths();
#endif
		}
	}

	std::string append_query(std::string url, const std::string& key, const std::string& value) {
		url += url.find('?') == std::string::npos ? '?' : '&';
		url += url_encode(key);
		url += '=';
		url += url_encode(value);
		return url;
	}

	response get(const std::string& url, const authentication_execution_context& context,
		const std::string& additional_trusted_ca) {
		const auto parsed = parse_url(url);
		if (!parsed.https && !context.allow_plain_http) {
			throw std::invalid_argument("Plain HTTP is not allowed for external authentication services.");
		}

		http::request<http::empty_body> request{http::verb::get, parsed.target, 11};
		request.set(http::field::host, parsed.host);
		request.set(http::field::user_agent, "PlanetaMatchMaker");

		beast::flat_buffer buffer;
		http::response<http::string_body> http_response;
		const auto results = resolve(parsed, context);

		if (parsed.https) {
			asio::ssl::context ssl_context(asio::ssl::context::tls_client);
			if (additional_trusted_ca.empty()) {
				load_system_trusted_certificate_authorities(ssl_context);
			}
			else {
				ssl_context.add_certificate_authority(asio::buffer(additional_trusted_ca));
			}
			ssl_context.set_verify_mode(asio::ssl::verify_peer);

			beast::ssl_stream<beast::tcp_stream> stream(context.executor, ssl_context);
			if (!SSL_set_tlsext_host_name(stream.native_handle(), parsed.host.c_str())) {
				throw_last_openssl_error();
			}
			stream.set_verify_callback(asio::ssl::host_name_verification(parsed.host));
			beast::get_lowest_layer(stream).expires_at(context.deadline);
			beast::get_lowest_layer(stream).async_connect(results, context.yield);
			beast::get_lowest_layer(stream).expires_at(context.deadline);
			stream.async_handshake(asio::ssl::stream_base::client, context.yield);
			beast::get_lowest_layer(stream).expires_at(context.deadline);
			http::async_write(stream, request, context.yield);
			beast::get_lowest_layer(stream).expires_at(context.deadline);
			http::async_read(stream, buffer, http_response, context.yield);
			close_socket(beast::get_lowest_layer(stream).socket());
		}
		else {
			beast::tcp_stream stream(context.executor);
			stream.expires_at(context.deadline);
			stream.async_connect(results, context.yield);
			stream.expires_at(context.deadline);
			http::async_write(stream, request, context.yield);
			stream.expires_at(context.deadline);
			http::async_read(stream, buffer, http_response, context.yield);
			close_socket(stream.socket());
		}

		return {http_response.result_int(), http_response.body()};
	}
}
