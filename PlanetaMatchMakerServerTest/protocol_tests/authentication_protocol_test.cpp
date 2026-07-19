#include <boost/test/unit_test.hpp>

#include "server/server_session.hpp"
#include "server/server_tls_context.hpp"
#include "authentication/authentication_http_client.hpp"
#include "authentication/steam_authentication_verifier.hpp"

#include "protocol_test_support.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>

namespace {
	using namespace pgl::test;

	constexpr auto tls_test_certificate_pem = R"pem(-----BEGIN CERTIFICATE-----
MIIC/zCCAeegAwIBAgIIQOvn4O2kVScwDQYJKoZIhvcNAQELBQAwFDESMBAGA1UE
AxMJbG9jYWxob3N0MB4XDTI2MDcwNDE2NTIzM1oXDTI2MDgwNDE2NTIzM1owFDES
MBAGA1UEAxMJbG9jYWxob3N0MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKC
AQEAurWRO0UKXZnX1HTTkrBiTj8pWvoOIYWZWYU5sNp9CfRCowqQUqyleDQWus9J
vdFL4ieUQ9nB4azqk1QaaE83jj2bmPtx361kbmicfjKNtqLxfR5V33ZLFO459q+B
KAloZvtoExzaEVmhxDSmfok3Pi0pVjLBfRq0U/39sJ4BZ5HnB/fPkBxfDvO3wlcl
fTmY1eecv8/44t0GLZJOUeZ0WLgcN4QeBTxW3VTYShX/mxwlFyUKlAi+N88Mjqcq
mEHJ5CjH1Hul6iFDp7L6xmQ73hrKnWJOHhf4/Kv53mYea+HuP/AVKUpo9sBLR27z
R4w3FsKBWCqrtnpqqkpCVPJKLQIDAQABo1UwUzAJBgNVHRMEAjAAMAsGA1UdDwQE
AwIFoDAdBgNVHQ4EFgQUN/q0k1ADCYS6yvR1EiZbbzsNogcwGgYDVR0RBBMwEYIJ
bG9jYWxob3N0hwR/AAABMA0GCSqGSIb3DQEBCwUAA4IBAQB5XHmQOxJlgEO+1axp
NyCWIspOX9NPdZshvhO/4eoxBirSzymfiQ4VqtzCz7CxAU9GNJ2o55AdEbnn5+iw
fuEIP8l/Q1lkllxUx9O9RT9CMNP/SL0K5yU0Q/bE+0fjR3I5ZhUy98GN1HKFtjU+
nnsc+1RfW1pM7utZeaQVQzwJcpMT50WXJA8Xnqap3zSkcKzzPr3coDVndTkt/IhU
AIhon0IqG0fzZiyJCK39Iy6QmwOn4cunFy52BCvFwmrZCzXAns38SGr50ZsCVJDs
cr+jL0HPmQcUSPWSI8d0CuKA/EFZtktRiLLWiImsYvu7ztRN32pt3LkKPH5snbV/
eD1j
-----END CERTIFICATE-----)pem";

	constexpr auto tls_test_private_key_pem = R"pem(-----BEGIN PRIVATE KEY-----
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQC6tZE7RQpdmdfU
dNOSsGJOPyla+g4hhZlZhTmw2n0J9EKjCpBSrKV4NBa6z0m90UviJ5RD2cHhrOqT
VBpoTzeOPZuY+3HfrWRuaJx+Mo22ovF9HlXfdksU7jn2r4EoCWhm+2gTHNoRWaHE
NKZ+iTc+LSlWMsF9GrRT/f2wngFnkecH98+QHF8O87fCVyV9OZjV55y/z/ji3QYt
kk5R5nRYuBw3hB4FPFbdVNhKFf+bHCUXJQqUCL43zwyOpyqYQcnkKMfUe6XqIUOn
svrGZDveGsqdYk4eF/j8q/neZh5r4e4/8BUpSmj2wEtHbvNHjDcWwoFYKqu2emqq
SkJU8kotAgMBAAECggEAH9nRiLbiac1Q0GGNpjyIyKhluatmtblAy9C6Yr0jD4un
woCO1ku1fqgEKKIsBkGqHE58MIb5WDJTYga66oh5Bb7kyFg8uNevhlY6PB0Lp9hs
Mdf29kQL0upA4SBXfCj+snK48mm9mbActIf2zydBUU4K+zV1ZI4lrR5rJLIPs0cZ
7FQ9h6PQANT9iPiNV40PGPC0yBDpyU2Ap8V9mZUbGa8Z60KXulu5LrWLgC9vW6/Z
gGqcYgQqP5au+xKYLbPnJEAz1S5QxBUL4VZWKqjGYtQfy/qAxsbWlQUFMv9v3882
9eJl5981kCtEk7haPph52Ef8Ga0VAKBWHXjNNFcJ0QKBgQDTgX0SJIwKXG1omt9j
ImtViQFomHNlZOfujL/PFpaBgHz7Vt7vB0DlZeS3QlyL1HDL/bpDBnJ4G3H3I9CV
9ebf7qrYz2pYY8HPvsli+o6qZdCJ68dvYf+fnrSn0CWCQuDiol2kODOpLvgFaH0h
STDfN+s38DKsr7o/zB1/Wc5lUwKBgQDh/KzjzM3MHmW9Yt0B02CodsLmvaAg83yD
INGkKh9jn01F06Cgi2m7DQl+x+i5Z/swbSE2dO+pZv1nE8okJvqKJmrJ/CJLXQX3
1ujfH6pQFQnNptYFcuv8xUqG52cawfvaUFQp0HTffEqXaTo5hEs2g3tRy2kkij8N
Jo60NWMifwKBgQDCFBsZT9H1G/UFPL/pLsdBGNPjD6ksDFjKy/qh43893BJTa9Zr
jcNd/I7QHqPfZ5QNi4ikQkE9tylvdzzKrncpWs8I3eMesoNAxzfyjCnncD/YnzNF
IK644J+zMUuJ6SaEsCqvKyc0grXx+HyB07B109ESZmWJkkKckgcrOdQrowKBgQCI
b7O16L1LAgMsvxZtdr/Blcn/4vZdToOa62KGeYPv8oqFoMo2g1T3QFy+NVV1Mqj9
yLUnmpvjK7HL3K/K+dmDNMR+ZAIUe/oRcqevpf9+T+VWSual7Yc6WaawId8m0X0L
hjn58MfO8cxa84XamduK8wCuFl/JVsnQV7oKZL0QYwKBgGAbS2pKpqTW2Ed/bD1A
JDs63/0ekwYSzdd2TeGqLWQqn75FtjexPNW/7aGvb2I0BNm5gPvXL+jN8jn3gc9K
Bfnq2B6IKldqVZnDIsfbNE+ggr6ChQL5vuascFVmVuTTrqahgZrx5ulkNvhikil1
5o2lHdTf6ESQMeCDvjsukhSb
-----END PRIVATE KEY-----)pem";

	class tls_test_certificate_files final {
	public:
		tls_test_certificate_files():
			directory_(std::filesystem::temp_directory_path() /
				("pmms_tls_protocol_test_" + std::to_string(
					std::chrono::steady_clock::now().time_since_epoch().count()))),
			certificate_path_(directory_ / "server.crt"),
			private_key_path_(directory_ / "server.key") {
			std::filesystem::create_directory(directory_);
			std::ofstream certificate_stream(certificate_path_);
			certificate_stream << tls_test_certificate_pem;
			std::ofstream private_key_stream(private_key_path_);
			private_key_stream << tls_test_private_key_pem;
		}

		~tls_test_certificate_files() {
			std::error_code ignored_error;
			std::filesystem::remove_all(directory_, ignored_error);
		}

		[[nodiscard]] const std::filesystem::path& certificate_path() const { return certificate_path_; }
		[[nodiscard]] const std::filesystem::path& private_key_path() const { return private_key_path_; }

	private:
		std::filesystem::path directory_;
		std::filesystem::path certificate_path_;
		std::filesystem::path private_key_path_;
	};

	class io_context_thread final {
	public:
		explicit io_context_thread(boost::asio::io_context& io):
			io_(io),
			thread_([this] { io_.run(); }) {}

		~io_context_thread() {
			io_.stop();
			if (thread_.joinable()) { thread_.join(); }
		}

	private:
		boost::asio::io_context& io_;
		std::thread thread_;
	};

	template <typename SyncWriteStream, typename... Data>
	void write_packed_to_stream(SyncWriteStream& stream, const Data&... data) {
		const auto buffer = pgl::pack_data(data...);
		boost::asio::write(stream, boost::asio::buffer(buffer));
	}

	template <typename T, typename SyncReadStream>
	T read_packed_from_stream(SyncReadStream& stream) {
		std::vector<uint8_t> buffer(pgl::get_packed_size<T>());
		boost::asio::read(stream, boost::asio::buffer(buffer), boost::asio::transfer_exactly(buffer.size()));
		T data{};
		pgl::unpack_data(buffer, data);
		return data;
	}

	const std::vector<uint8_t> test_credential{1, 2, 3, 4};

	pgl::authentication_request_message make_authentication_request(
		const pgl::server_setting& setting,
		const pgl::player_name_t& player_name,
		const pgl::api_version_type request_api_version = pgl::api_version,
		const pgl::authentication_method method = pgl::authentication_method::steam) {
		return {
			request_api_version,
			method,
			pgl::game_id_t(setting.authentication.game_id),
			pgl::game_version_t(setting.authentication.game_version),
			player_name
		};
	}

	pgl::message_attachment_chunk make_credential_chunk() {
		pgl::message_attachment_chunk chunk{};
		chunk.sequence = 0;
		chunk.data_size = static_cast<uint8_t>(test_credential.size());
		std::ranges::copy(test_credential, chunk.data.begin());
		return chunk;
	}

	void write_authentication_request(tcp::socket& socket, const pgl::authentication_request_message& request,
		const std::vector<uint8_t>& credential = test_credential) {
		write_packed(socket, pgl::request_message_header{pgl::message_type::authentication,
			static_cast<pgl::message_attachment_size_t>(credential.size())}, request);
		for (std::size_t offset = 0, sequence = 0; offset < credential.size();
			offset += pgl::message_attachment_chunk_data_size, ++sequence) {
			pgl::message_attachment_chunk chunk{};
			chunk.sequence = static_cast<uint16_t>(sequence);
			chunk.data_size = static_cast<uint8_t>(std::min<std::size_t>(
				pgl::message_attachment_chunk_data_size, credential.size() - offset));
			std::ranges::copy_n(credential.begin() + offset, chunk.data_size, chunk.data.begin());
			write_packed(socket, chunk);
		}
	}

	template <typename SyncWriteStream>
	void write_authentication_request_to_stream(SyncWriteStream& stream,
		const pgl::authentication_request_message& request,
		const std::vector<uint8_t>& credential = test_credential) {
		write_packed_to_stream(stream, pgl::request_message_header{pgl::message_type::authentication,
			static_cast<pgl::message_attachment_size_t>(credential.size())}, request);
		for (std::size_t offset = 0, sequence = 0; offset < credential.size();
			offset += pgl::message_attachment_chunk_data_size, ++sequence) {
			pgl::message_attachment_chunk chunk{};
			chunk.sequence = static_cast<uint16_t>(sequence);
			chunk.data_size = static_cast<uint8_t>(std::min<std::size_t>(
				pgl::message_attachment_chunk_data_size, credential.size() - offset));
			std::ranges::copy_n(credential.begin() + offset, chunk.data_size, chunk.data.begin());
			write_packed_to_stream(stream, chunk);
		}
	}

	struct test_http_response final {
		unsigned status = 200;
		std::string body;
		std::chrono::milliseconds delay{};
	};

	std::vector<test_http_response> successful_steam_responses() {
		return {
			{200, R"({"response":{"params":{"result":"OK","steamid":"76561198000000000"}}})"},
			{200, R"({"appownership":{"ownsapp":true}})"}
		};
	}

	class authentication_http_test_server final {
	public:
		explicit authentication_http_test_server(
			std::vector<test_http_response> responses = successful_steam_responses()):
			acceptor_(io_, tcp::endpoint(boost::asio::ip::address_v4::loopback(), 0)),
			responses_(std::move(responses)),
			thread_([this] { run(); }) {}

		~authentication_http_test_server() {
			boost::system::error_code ignored_error;
			acceptor_.close(ignored_error);
			if (thread_.joinable()) { thread_.join(); }
		}

		[[nodiscard]] std::string url(const std::string& path) const {
			return "http://127.0.0.1:" + std::to_string(acceptor_.local_endpoint().port()) + path;
		}

		[[nodiscard]] std::size_t request_count() const { return request_count_; }

	private:
		void run() {
			for (const auto& current_response : responses_) {
				tcp::socket socket(io_);
				boost::system::error_code error;
				acceptor_.accept(socket, error);
				if (error) { return; }

				boost::asio::streambuf request_buffer;
				boost::asio::read_until(socket, request_buffer, "\r\n\r\n", error);
				if (error) { return; }
				++request_count_;
				if (current_response.delay.count() > 0) { std::this_thread::sleep_for(current_response.delay); }

				const auto& body = current_response.body;
				const auto response = std::string("HTTP/1.1 ") + std::to_string(current_response.status) +
					" Test\r\nContent-Type: application/json\r\nContent-Length: " +
					std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
				boost::asio::write(socket, boost::asio::buffer(response), error);
			}
		}

		boost::asio::io_context io_;
		tcp::acceptor acceptor_;
		std::vector<test_http_response> responses_;
		std::atomic_size_t request_count_{};
		std::thread thread_;
	};

	class authentication_https_test_server final {
	public:
		authentication_https_test_server():
			ssl_context_(boost::asio::ssl::context::tls_server),
			acceptor_(io_, tcp::endpoint(tcp::v4(), 0)) {
			ssl_context_.use_certificate_chain(boost::asio::buffer(tls_test_certificate_pem,
				std::char_traits<char>::length(tls_test_certificate_pem)));
			ssl_context_.use_private_key(boost::asio::buffer(tls_test_private_key_pem,
				std::char_traits<char>::length(tls_test_private_key_pem)),
				boost::asio::ssl::context::pem);
			thread_ = std::thread([this] { run(); });
		}

		~authentication_https_test_server() {
			boost::system::error_code ignored_error;
			acceptor_.close(ignored_error);
			if (thread_.joinable()) { thread_.join(); }
		}

		[[nodiscard]] std::string url(const std::string& host) const {
			return "https://" + host + ":" + std::to_string(acceptor_.local_endpoint().port()) + "/auth";
		}

	private:
		void run() {
			tcp::socket socket(io_);
			boost::system::error_code error;
			acceptor_.accept(socket, error);
			if (error) { return; }
			boost::asio::ssl::stream<tcp::socket> stream(std::move(socket), ssl_context_);
			stream.handshake(boost::asio::ssl::stream_base::server, error);
			if (error) { return; }

			boost::asio::streambuf request_buffer;
			boost::asio::read_until(stream, request_buffer, "\r\n\r\n", error);
			if (error) { return; }
			const std::string body = R"({"keys":[]})";
			const auto response = std::string("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: ") +
				std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
			boost::asio::write(stream, boost::asio::buffer(response), error);
		}

		boost::asio::io_context io_;
		boost::asio::ssl::context ssl_context_;
		tcp::acceptor acceptor_;
		std::thread thread_;
	};

	std::pair<std::optional<pgl::authentication_http::response>, std::exception_ptr> get_test_https_url(
		const std::string& url) {
		boost::asio::io_context io;
		std::optional<pgl::authentication_http::response> response;
		std::exception_ptr exception;
		boost::asio::spawn(io, [&](boost::asio::yield_context yield) {
			try {
				const pgl::authentication_execution_context context{
					io.get_executor(), yield, std::chrono::steady_clock::now() + std::chrono::seconds(2)
				};
				response = pgl::authentication_http::get(url, context, tls_test_certificate_pem);
			}
			catch (...) { exception = std::current_exception(); }
		}, boost::asio::detached);
		io.run();
		return {std::move(response), exception};
	}

	bool is_invalid_argument(const std::exception_ptr& exception) {
		try {
			std::rethrow_exception(exception);
		}
		catch (const std::invalid_argument&) {
			return true;
		}
		catch (...) {
			return false;
		}
	}

	void configure_steam_authentication(pgl::server_setting& setting,
		const authentication_http_test_server& steam_server) {
		setting.authentication.allow_plain_connections = true;
		setting.authentication.allow_plain_external_service_connections = true;
		setting.authentication.method = pgl::authentication_method::steam;
		setting.authentication.steam.app_id = 480;
		setting.authentication.steam.publisher_key = "test-publisher-key";
		setting.authentication.steam.authenticate_user_ticket_url = steam_server.url("/auth");
		setting.authentication.steam.check_app_ownership_url = steam_server.url("/ownership");
	}

	void enable_steam_authentication_without_http(pgl::server_setting& setting) {
		setting.authentication.allow_plain_connections = true;
		setting.authentication.method = pgl::authentication_method::steam;
		setting.authentication.steam.app_id = 480;
		setting.authentication.steam.publisher_key = "test-publisher-key";
	}

	pgl::authentication_result authenticate(
		protocol_context& context,
		const pgl::authentication_method method,
		const std::vector<uint8_t>& credential) {
		const auto request = make_authentication_request(context.setting, u8"player", pgl::api_version, method);
		protocol_handler_run handler(context, pgl::message_type::authentication);
		write_authentication_request(context.client_socket, request, credential);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::authentication_reply_message>(context.client_socket);
		const auto exception = handler.wait();
		BOOST_CHECK(reply_header.message_type == pgl::message_type::authentication);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		if (reply.result == pgl::authentication_result::success) {
			BOOST_CHECK(!exception);
		}
		else {
			BOOST_CHECK(is_intended_disconnect(exception));
		}
		return reply.result;
	}
}

BOOST_AUTO_TEST_SUITE(authentication_protocol_test)
	BOOST_AUTO_TEST_CASE(test_authentication_method_wire_values) {
		BOOST_CHECK_EQUAL(static_cast<unsigned int>(pgl::authentication_method::none), 0u);
		BOOST_CHECK_EQUAL(static_cast<unsigned int>(pgl::authentication_method::steam), 1u);
	}

	BOOST_AUTO_TEST_CASE(test_steam_peer_id_is_derived_from_canonical_decimal_steam_id64) {
		const pgl::authentication_provider_user_id_t provider_user_id{u8"76561198000000000"};

		BOOST_CHECK(pgl::derive_steam_p2p_service_peer_id(provider_user_id) ==
			pgl::p2p_service_peer_id_t{u8"76561198000000000"});
		BOOST_CHECK_THROW(pgl::derive_steam_p2p_service_peer_id(
			pgl::authentication_provider_user_id_t{u8"076561198000000000"}), std::invalid_argument);
	}

	BOOST_AUTO_TEST_CASE(test_external_authentication_rejects_plain_http_by_default) {
		boost::asio::io_context io;
		std::exception_ptr exception;
		boost::asio::spawn(io, [&](boost::asio::yield_context yield) {
			try {
				const pgl::authentication_execution_context context{
					io.get_executor(), yield, std::chrono::steady_clock::now() + std::chrono::seconds(2)
				};
				pgl::authentication_http::get("http://127.0.0.1:1/auth", context);
			}
			catch (...) { exception = std::current_exception(); }
		}, boost::asio::detached);
		io.run();

		BOOST_REQUIRE(exception);
		BOOST_CHECK(is_invalid_argument(exception));
	}

	BOOST_AUTO_TEST_CASE(test_external_https_verifies_certificate_host_name) {
		{
			authentication_https_test_server server;
			const auto [response, exception] = get_test_https_url(server.url("127.0.0.1"));
			BOOST_CHECK(!exception);
			BOOST_REQUIRE(response.has_value());
			BOOST_CHECK_EQUAL(response->status, 200u);
		}

		{
			authentication_https_test_server server;
			const auto [response, exception] = get_test_https_url(server.url("127.0.0.2"));
			BOOST_CHECK(exception);
			BOOST_CHECK(!response.has_value());
		}
	}

	BOOST_AUTO_TEST_CASE(test_authentication_request_replies_success_and_assigns_player) {
		protocol_context context;
		authentication_http_test_server steam_server;
		configure_steam_authentication(context.setting, steam_server);
		const auto request = make_authentication_request(context.setting, u8"player");
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_authentication_request(context.client_socket, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::authentication_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.message_type == pgl::message_type::authentication);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.result == pgl::authentication_result::success);
		BOOST_CHECK_EQUAL(reply.player_tag, 1);
		BOOST_CHECK(context.session_data.is_authenticated());
		BOOST_REQUIRE(context.session_data.identity().has_value());
		BOOST_CHECK(context.session_data.identity()->authentication_provider_user_id ==
			pgl::authentication_provider_user_id_t{u8"76561198000000000"});
		BOOST_CHECK(context.session_data.client_player_name().name == u8"player");
		BOOST_CHECK(context.server_data.get_player_name_container().is_player_exist(
			context.session_data.client_player_name()));
	}

	BOOST_AUTO_TEST_CASE(test_unauthenticated_connection_replies_success_without_identity) {
		protocol_context context;
		context.setting.authentication.allow_plain_connections = false;

		BOOST_CHECK(authenticate(context, pgl::authentication_method::none, {}) ==
			pgl::authentication_result::success);
		BOOST_CHECK(context.session_data.is_authenticated());
		BOOST_CHECK(!context.session_data.identity().has_value());
		BOOST_CHECK(context.session_data.client_player_name().name == u8"player");
	}

	BOOST_AUTO_TEST_CASE(test_authentication_request_is_rejected_when_method_differs_from_server_setting) {
		protocol_context context;
		context.setting.authentication.method = pgl::authentication_method::steam;

		BOOST_CHECK(authenticate(context, pgl::authentication_method::none, {}) ==
			pgl::authentication_result::unsupported_authentication_method);
		BOOST_CHECK(!context.session_data.is_authenticated());
	}

	BOOST_AUTO_TEST_CASE(test_unauthenticated_connection_rejects_credential_attachment) {
		protocol_context context;
		const auto request = make_authentication_request(context.setting, u8"player", pgl::api_version,
			pgl::authentication_method::none);
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_authentication_request(context.client_socket, request, test_credential);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.message_type == pgl::message_type::authentication);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::request_parameter_wrong);
		BOOST_CHECK(!context.session_data.is_authenticated());
	}

	BOOST_AUTO_TEST_CASE(test_steam_authentication_replies_ticket_invalid) {
		protocol_context context;
		authentication_http_test_server steam_server({
			{200, R"({"response":{"params":{"result":"InvalidTicket"}}})"}
		});
		configure_steam_authentication(context.setting, steam_server);

		BOOST_CHECK(authenticate(context, pgl::authentication_method::steam, test_credential) ==
			pgl::authentication_result::steam_ticket_invalid);
		BOOST_CHECK_EQUAL(steam_server.request_count(), 1);
	}

	BOOST_AUTO_TEST_CASE(test_steam_authentication_rejects_noncanonical_steam_id64) {
		const std::vector<std::string> invalid_steam_ids{
			"", "not-a-steam-id", "076561198000000000", "18446744073709551616", std::string(129, '1')
		};
		for (const auto& steam_id : invalid_steam_ids) {
			protocol_context context;
			const auto body = std::string(R"({"response":{"params":{"result":"OK","steamid":")") +
				steam_id + R"("}}})";
			authentication_http_test_server steam_server({{200, body}});
			configure_steam_authentication(context.setting, steam_server);

			BOOST_TEST_CONTEXT("steamid=" << steam_id) {
				BOOST_CHECK(authenticate(context, pgl::authentication_method::steam, test_credential) ==
					pgl::authentication_result::steam_ticket_invalid);
				BOOST_CHECK_EQUAL(steam_server.request_count(), 1);
			}
		}
	}

	BOOST_AUTO_TEST_CASE(test_steam_authentication_replies_ownership_check_failed) {
		protocol_context context;
		authentication_http_test_server steam_server({
			{200, R"({"response":{"params":{"result":"OK","steamid":"76561198000000000"}}})"},
			{200, R"({"appownership":{"ownsapp":false}})"}
		});
		configure_steam_authentication(context.setting, steam_server);

		BOOST_CHECK(authenticate(context, pgl::authentication_method::steam, test_credential) ==
			pgl::authentication_result::steam_ownership_check_failed);
		BOOST_CHECK_EQUAL(steam_server.request_count(), 2);
	}

	BOOST_AUTO_TEST_CASE(test_steam_authentication_replies_service_unavailable) {
		protocol_context context;
		authentication_http_test_server steam_server({{503, R"({"error":"unavailable"})"}});
		configure_steam_authentication(context.setting, steam_server);

		BOOST_CHECK(authenticate(context, pgl::authentication_method::steam, test_credential) ==
			pgl::authentication_result::steam_authentication_service_unavailable);
		BOOST_CHECK_EQUAL(steam_server.request_count(), 1);
	}

	BOOST_AUTO_TEST_CASE(test_external_authentication_timeout_does_not_block_server_executor) {
		protocol_context context;
		authentication_http_test_server steam_server({
			{200, R"({"response":{"params":{"result":"OK","steamid":"76561198000000000"}}})",
				std::chrono::milliseconds(1500)}
		});
		configure_steam_authentication(context.setting, steam_server);
		context.setting.authentication.timeout_seconds = 1;
		const auto request = make_authentication_request(context.setting, u8"player");
		protocol_handler_run handler(context, pgl::message_type::authentication);

		std::promise<void> executor_progress;
		auto progress = executor_progress.get_future();
		boost::asio::steady_timer progress_timer(context.io, std::chrono::milliseconds(100));
		progress_timer.async_wait([&executor_progress](const boost::system::error_code& error) {
			if (!error) { executor_progress.set_value(); }
		});
		write_authentication_request(context.client_socket, request);

		BOOST_CHECK(progress.wait_for(std::chrono::milliseconds(500)) == std::future_status::ready);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::authentication_reply_message>(context.client_socket);
		const auto exception = handler.wait();
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.result == pgl::authentication_result::steam_authentication_service_unavailable);
		BOOST_CHECK(is_intended_disconnect(exception));
	}

	BOOST_AUTO_TEST_CASE(test_tls_connection_authentication_request_replies_success_and_assigns_player) {
		boost::asio::io_context server_io;
		tcp::acceptor acceptor(server_io, tcp::endpoint(tcp::v4(), 0));
		std::mutex acceptor_mutex;
		pgl::server_data server_data;
		auto setting = make_protocol_test_setting();
		authentication_http_test_server steam_server;
		configure_steam_authentication(setting, steam_server);
		setting.authentication.allow_plain_connections = false;
		setting.tls.mode = pgl::server_tls_mode::tls;
		const tls_test_certificate_files certificate_files;
		setting.tls.certificate_path = certificate_files.certificate_path();
		setting.tls.private_key_path = certificate_files.private_key_path();
		pgl::server_tls_context tls_context;
		tls_context.reload(setting.tls);
		const auto invoker = pgl::message_handler_invoker_factory::make_shared_standard();
		const auto session = std::make_shared<pgl::server_session>(
			acceptor, acceptor_mutex, tls_context, server_data, setting, invoker);
		session->start();
		io_context_thread server_thread(server_io);

		boost::asio::io_context client_io;
		boost::asio::ssl::context client_ssl_context(boost::asio::ssl::context::tls_client);
		client_ssl_context.set_verify_mode(boost::asio::ssl::verify_none);
		boost::asio::ssl::stream<tcp::socket> client_stream(client_io, client_ssl_context);
		client_stream.lowest_layer().connect(tcp::endpoint(boost::asio::ip::address_v4::loopback(),
			acceptor.local_endpoint().port()));
		client_stream.handshake(boost::asio::ssl::stream_base::client);
		const auto request = make_authentication_request(setting, u8"tls-player");

		write_authentication_request_to_stream(client_stream, request);
		const auto reply_header = read_packed_from_stream<pgl::reply_message_header>(client_stream);
		const auto reply = read_packed_from_stream<pgl::authentication_reply_message>(client_stream);

		BOOST_CHECK(reply_header.message_type == pgl::message_type::authentication);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.result == pgl::authentication_result::success);
		BOOST_CHECK_EQUAL(reply.player_tag, 1);
		BOOST_CHECK(server_data.get_player_name_container().is_player_exist({u8"tls-player", 1}));
		boost::system::error_code ignored_error;
		client_stream.shutdown(ignored_error);
		client_stream.lowest_layer().close(ignored_error);
		session->stop();
		server_io.stop();
	}

	BOOST_AUTO_TEST_CASE(test_authentication_request_replies_game_id_mismatch_and_disconnects) {
		protocol_context context;
		const pgl::authentication_request_message request{
			pgl::api_version,
			pgl::authentication_method::steam,
			u8"wrong-game",
			pgl::game_version_t(context.setting.authentication.game_version),
			u8"player"
		};
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::authentication}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::authentication_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.message_type == pgl::message_type::authentication);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.result == pgl::authentication_result::game_id_mismatch);
		BOOST_CHECK(!context.session_data.is_authenticated());
	}

	BOOST_AUTO_TEST_CASE(test_authentication_request_replies_api_version_mismatch_and_disconnects) {
		protocol_context context;
		const pgl::authentication_request_message request{
			static_cast<pgl::api_version_type>(pgl::api_version + 1),
			pgl::authentication_method::steam,
			pgl::game_id_t(context.setting.authentication.game_id),
			pgl::game_version_t(context.setting.authentication.game_version),
			u8"player"
		};
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::authentication}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::authentication_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.result == pgl::authentication_result::api_version_mismatch);
		BOOST_CHECK_EQUAL(reply.api_version, pgl::api_version);
	}

	BOOST_AUTO_TEST_CASE(test_authentication_request_replies_game_version_mismatch_and_disconnects) {
		protocol_context context;
		const pgl::authentication_request_message request{
			pgl::api_version,
			pgl::authentication_method::steam,
			pgl::game_id_t(context.setting.authentication.game_id),
			u8"2.0.0",
			u8"player"
		};
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::authentication}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::authentication_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.result == pgl::authentication_result::game_version_mismatch);
		BOOST_CHECK(reply.game_version == pgl::game_version_t(context.setting.authentication.game_version));
	}

	BOOST_AUTO_TEST_CASE(test_reserved_authentication_method_replies_unsupported_and_disconnects) {
		protocol_context context;
		context.setting.authentication.allow_plain_connections = true;
		const pgl::authentication_request_message request{
			pgl::api_version,
			static_cast<pgl::authentication_method>(2),
			pgl::game_id_t(context.setting.authentication.game_id),
			pgl::game_version_t(context.setting.authentication.game_version),
			u8"player"
		};
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::authentication}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::authentication_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.result == pgl::authentication_result::unsupported_authentication_method);
		BOOST_CHECK(!context.session_data.is_authenticated());
	}

	BOOST_AUTO_TEST_CASE(test_authentication_request_replies_insecure_connection_when_plain_is_not_allowed) {
		protocol_context context;
		context.setting.authentication.method = pgl::authentication_method::steam;
		context.setting.authentication.allow_plain_connections = false;
		const auto request = make_authentication_request(context.setting, u8"player");
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::authentication}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::authentication_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.result == pgl::authentication_result::insecure_connection);
		BOOST_CHECK(!context.session_data.is_authenticated());
	}

	BOOST_AUTO_TEST_CASE(test_authentication_request_replies_format_invalid_for_empty_credential) {
		protocol_context context;
		enable_steam_authentication_without_http(context.setting);
		const auto request = make_authentication_request(context.setting, u8"player");
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::authentication}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::authentication_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.result == pgl::authentication_result::authentication_data_format_invalid);
		BOOST_CHECK(!context.session_data.is_authenticated());
	}

	BOOST_AUTO_TEST_CASE(test_authentication_request_replies_size_exceeded_for_credential_over_setting_limit) {
		protocol_context context;
		enable_steam_authentication_without_http(context.setting);
		context.setting.authentication.max_credential_bytes = 3;
		const auto request = make_authentication_request(context.setting, u8"player");
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::authentication,
			static_cast<pgl::message_attachment_size_t>(test_credential.size())}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::authentication_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.result == pgl::authentication_result::authentication_data_size_exceeded);
		BOOST_CHECK(!context.session_data.is_authenticated());
	}

	BOOST_AUTO_TEST_CASE(test_authentication_request_replies_format_invalid_for_bad_chunk_sequence) {
		protocol_context context;
		enable_steam_authentication_without_http(context.setting);
		const auto request = make_authentication_request(context.setting, u8"player");
		auto chunk = make_credential_chunk();
		chunk.sequence = 1;
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::authentication,
			static_cast<pgl::message_attachment_size_t>(test_credential.size())}, request, chunk);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::authentication_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.result == pgl::authentication_result::authentication_data_format_invalid);
		BOOST_CHECK(!context.session_data.is_authenticated());
	}

	BOOST_AUTO_TEST_CASE(test_authentication_request_replies_format_invalid_for_bad_chunk_size) {
		protocol_context context;
		enable_steam_authentication_without_http(context.setting);
		const auto request = make_authentication_request(context.setting, u8"player");
		auto chunk = make_credential_chunk();
		--chunk.data_size;
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::authentication,
			static_cast<pgl::message_attachment_size_t>(test_credential.size())}, request, chunk);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::authentication_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.result == pgl::authentication_result::authentication_data_format_invalid);
	}

	BOOST_AUTO_TEST_CASE(test_authentication_request_replies_format_invalid_for_nonzero_chunk_padding) {
		protocol_context context;
		enable_steam_authentication_without_http(context.setting);
		const auto request = make_authentication_request(context.setting, u8"player");
		auto chunk = make_credential_chunk();
		chunk.data[chunk.data_size] = 1;
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::authentication,
			static_cast<pgl::message_attachment_size_t>(test_credential.size())}, request, chunk);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::authentication_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.result == pgl::authentication_result::authentication_data_format_invalid);
	}

	BOOST_AUTO_TEST_CASE(test_authentication_request_replies_size_exceeded_for_attachment_over_protocol_limit) {
		protocol_context context;
		enable_steam_authentication_without_http(context.setting);
		context.setting.authentication.max_credential_bytes = pgl::message_attachment_max_bytes;
		const auto request = make_authentication_request(context.setting, u8"player");
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::authentication,
			pgl::message_attachment_max_bytes + 1}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::authentication_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.result == pgl::authentication_result::authentication_data_size_exceeded);
	}

	BOOST_AUTO_TEST_CASE(test_authentication_request_replies_parameter_error_for_empty_player_name) {
		protocol_context context;
		const auto request = make_authentication_request(context.setting, u8"");
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::authentication}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::request_parameter_wrong);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_authentication_request_replies_operation_invalid_when_already_authenticated) {
		protocol_context context;
		context.session_data.set_authenticated();
		const auto request = make_authentication_request(context.setting, u8"player");
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::authentication}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::operation_invalid);
		expect_no_more_reply_data(context.client_socket);
	}

BOOST_AUTO_TEST_SUITE_END()
