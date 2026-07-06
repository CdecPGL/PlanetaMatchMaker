#include <boost/test/unit_test.hpp>

#include "../../PlanetaMatchMakerServer/source/server/server_session.hpp"
#include "../../PlanetaMatchMakerServer/source/server/server_tls_context.hpp"

#include "protocol_test_support.hpp"

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
}

BOOST_AUTO_TEST_SUITE(authentication_protocol_test)
	BOOST_AUTO_TEST_CASE(test_authentication_request_replies_success_and_assigns_player) {
		protocol_context context;
		const pgl::authentication_request_message request{
			pgl::api_version,
			pgl::game_id_t(context.setting.authentication.game_id),
			pgl::game_version_t(context.setting.authentication.game_version),
			u8"player"
		};
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::authentication}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::authentication_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.message_type == pgl::message_type::authentication);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.result == pgl::authentication_result::success);
		BOOST_CHECK_EQUAL(reply.player_tag, 1);
		BOOST_CHECK(context.session_data.is_authenticated());
		BOOST_CHECK(context.session_data.client_player_name().name == u8"player");
		BOOST_CHECK(context.server_data.get_player_name_container().is_player_exist(
			context.session_data.client_player_name()));
	}

	BOOST_AUTO_TEST_CASE(test_tls_connection_authentication_request_replies_success_and_assigns_player) {
		boost::asio::io_context server_io;
		tcp::acceptor acceptor(server_io, tcp::endpoint(tcp::v4(), 0));
		std::mutex acceptor_mutex;
		pgl::server_data server_data;
		auto setting = make_protocol_test_setting();
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
		const pgl::authentication_request_message request{
			pgl::api_version,
			pgl::game_id_t(setting.authentication.game_id),
			pgl::game_version_t(setting.authentication.game_version),
			u8"tls-player"
		};

		write_packed_to_stream(client_stream, pgl::request_message_header{pgl::message_type::authentication}, request);
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

	BOOST_AUTO_TEST_CASE(test_authentication_request_replies_parameter_error_for_empty_player_name) {
		protocol_context context;
		const pgl::authentication_request_message request{
			pgl::api_version,
			pgl::game_id_t(context.setting.authentication.game_id),
			pgl::game_version_t(context.setting.authentication.game_version),
			u8""
		};
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
		const pgl::authentication_request_message request{
			pgl::api_version,
			pgl::game_id_t(context.setting.authentication.game_id),
			pgl::game_version_t(context.setting.authentication.game_version),
			u8"player"
		};
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::authentication}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::operation_invalid);
		expect_no_more_reply_data(context.client_socket);
	}

BOOST_AUTO_TEST_SUITE_END()
