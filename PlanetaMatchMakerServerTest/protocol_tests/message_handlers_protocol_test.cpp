#include <boost/test/unit_test.hpp>

#include <array>
#include <chrono>
#include <exception>
#include <future>
#include <memory>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include "../../PlanetaMatchMakerServer/source/message/message_handler_invoker.hpp"
#include "../../PlanetaMatchMakerServer/source/message/message_handler_invoker_factory.hpp"
#include "../../PlanetaMatchMakerServer/source/message/message_handle_parameter.hpp"
#include "../../PlanetaMatchMakerServer/source/message/messages.hpp"
#include "../../PlanetaMatchMakerServer/source/server/server_constants.hpp"
#include "../../PlanetaMatchMakerServer/source/server/server_data.hpp"
#include "../../PlanetaMatchMakerServer/source/server/server_errors.hpp"
#include "../../PlanetaMatchMakerServer/source/server/server_setting.hpp"
#include "../../PlanetaMatchMakerServer/source/session/session_data.hpp"
#include "../../PlanetaMatchMakerServer/source/utilities/pack.hpp"

namespace {
	using tcp = boost::asio::ip::tcp;

	constexpr auto public_open_room = pgl::room_setting_flag::public_room | pgl::room_setting_flag::open_room;

	void write_packed(tcp::socket& socket, const auto&... data) {
		auto buffer = pgl::pack_data(data...);
		boost::asio::write(socket, boost::asio::buffer(buffer));
	}

	template <typename T>
	T read_packed(tcp::socket& socket) {
		std::vector<uint8_t> buffer(pgl::get_packed_size<T>());
		boost::asio::read(socket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(buffer.size()));
		T data{};
		pgl::unpack_data(buffer, data);
		return data;
	}

	void expect_no_more_reply_data(tcp::socket& socket) {
		BOOST_CHECK_EQUAL(socket.available(), 0);
	}

	bool is_intended_disconnect(const std::exception_ptr& exception) {
		if (!exception) { return false; }

		try { std::rethrow_exception(exception); }
		catch (const pgl::server_session_intended_disconnect_error&) { return true; }
		catch (...) { return false; }
	}

	pgl::server_setting make_protocol_test_setting() {
		pgl::server_setting setting;
		setting.common.time_out_seconds = 2;
		setting.common.max_room_count = 16;
		setting.common.max_player_per_room = 8;
		setting.authentication.game_id = u8"protocol-test";
		setting.authentication.game_version = u8"1.0.0";
		setting.authentication.enable_game_version_check = true;
		setting.connection_test.connection_check_tcp_time_out_seconds = 2;
		setting.connection_test.connection_check_udp_time_out_seconds = 2;
		setting.connection_test.connection_check_udp_try_count = 1;
		return setting;
	}

	pgl::endpoint make_endpoint(const pgl::port_number_type port_number) {
		auto endpoint = pgl::endpoint();
		endpoint.port_number = port_number;
		return endpoint;
	}

	pgl::room_data make_room(const pgl::room_id_t room_id, const pgl::player_full_name& host_full_name,
		const pgl::room_setting_flag setting_flags = public_open_room,
		const pgl::room_password_t& password = {},
		const uint8_t max_player_count = 4,
		const uint8_t current_player_count = 1) {
		pgl::game_host_external_id_t external_id{};
		external_id[0] = 42;
		return {
			room_id,
			host_full_name,
			setting_flags,
			password,
			max_player_count,
			pgl::datetime(2024, 1, static_cast<int>(room_id)),
			make_endpoint(57000),
			pgl::game_host_connection_establish_mode::builtin,
			make_endpoint(57001),
			external_id,
			current_player_count
		};
	}

	struct protocol_context final {
		boost::asio::io_context io;
		tcp::acceptor acceptor;
		tcp::socket client_socket;
		tcp::socket server_socket;
		pgl::server_data server_data;
		pgl::server_setting setting;
		pgl::session_data session_data;

		protocol_context():
			acceptor(io, tcp::endpoint(tcp::v4(), 0)),
			client_socket(io),
			server_socket(io),
			setting(make_protocol_test_setting()) {
			client_socket.connect(acceptor.local_endpoint());
			acceptor.accept(server_socket);
			session_data.set_remote_endpoint(pgl::endpoint::make_from_boost_endpoint(server_socket.remote_endpoint()));
		}
	};

	class protocol_handler_run final {
	public:
		explicit protocol_handler_run(protocol_context& context):
			protocol_handler_run(context, std::nullopt) {}

		protocol_handler_run(protocol_context& context, const pgl::message_type message_type):
			protocol_handler_run(context, std::optional(message_type)) {}

		protocol_handler_run(protocol_context& context, const std::optional<pgl::message_type> message_type):
			context_(context),
			promise_(std::make_shared<std::promise<std::exception_ptr>>()),
			future_(promise_->get_future()) {
			const auto invoker = pgl::message_handler_invoker_factory::make_shared_standard();
			boost::asio::spawn(context_.io, [this, invoker, message_type](
				boost::asio::yield_context yield) {
				std::exception_ptr exception;
				try {
					auto param = std::make_shared<pgl::message_handle_parameter>(pgl::message_handle_parameter{
						context_.server_socket,
						context_.server_data,
						yield,
						std::chrono::seconds(context_.setting.common.time_out_seconds),
						context_.session_data,
						context_.setting
					});
					if (message_type.has_value()) { invoker->handle_specific_message(*message_type, param); }
					else { invoker->handle_message(param); }
				}
				catch (...) { exception = std::current_exception(); }

				promise_->set_value(exception);
			});
			thread_ = std::thread([this] { context_.io.run(); });
		}

		~protocol_handler_run() {
			context_.io.stop();
			if (thread_.joinable()) { thread_.join(); }
		}

		std::exception_ptr wait() {
			if (future_.wait_for(std::chrono::seconds(5)) != std::future_status::ready) {
				throw std::runtime_error("Timed out waiting for protocol handler.");
			}
			auto exception = future_.get();
			context_.io.stop();
			if (thread_.joinable()) { thread_.join(); }
			return exception;
		}

	private:
		protocol_context& context_;
		std::shared_ptr<std::promise<std::exception_ptr>> promise_;
		std::future<std::exception_ptr> future_;
		std::thread thread_;
	};

	class tcp_echo_server final {
	public:
		tcp_echo_server():
			acceptor_(io),
			socket_(io) {
			for (auto port = pgl::port_number_type{49152}; port < 65535; ++port) {
				boost::system::error_code error;
				acceptor_.open(tcp::v4(), error);
				if (error) { continue; }
				acceptor_.set_option(boost::asio::socket_base::reuse_address(true), error);
				acceptor_.bind(tcp::endpoint(boost::asio::ip::address_v4::loopback(), port), error);
				if (!error) {
					acceptor_.listen(boost::asio::socket_base::max_listen_connections, error);
					if (!error) {
						port_ = port;
						break;
					}
				}
				acceptor_.close();
			}
			if (port_ == 0) { throw std::runtime_error("No available dynamic/private TCP port."); }

			acceptor_.async_accept(socket_, [this](const boost::system::error_code& error) {
				if (error) { return; }
				boost::asio::async_read(socket_, boost::asio::buffer(buffer_.data(), test_text_.size()),
					[this](const boost::system::error_code& read_error, const size_t read_size) {
					if (read_error) { return; }
					boost::asio::async_write(socket_, boost::asio::buffer(buffer_.data(), read_size),
						[](const boost::system::error_code&, size_t) {});
				});
			});
			thread_ = std::thread([this] { io.run(); });
		}

		~tcp_echo_server() {
			io.stop();
			boost::system::error_code ignored_error;
			socket_.close(ignored_error);
			acceptor_.close(ignored_error);
			if (thread_.joinable()) { thread_.join(); }
		}

		[[nodiscard]] pgl::port_number_type port() const { return port_; }

	private:
		boost::asio::io_context io;
		tcp::acceptor acceptor_;
		tcp::socket socket_;
		std::array<char, 64> buffer_{};
		pgl::port_number_type port_{};
		std::thread thread_;
		const std::string test_text_ = "Hello. This is PMMS.";
	};

	class udp_echo_server final {
	public:
		udp_echo_server():
			socket_(io) {
			for (auto port = pgl::port_number_type{49152}; port < 65535; ++port) {
				boost::system::error_code error;
				socket_.open(boost::asio::ip::udp::v4(), error);
				if (error) { continue; }
				socket_.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::loopback(), port), error);
				if (!error) {
					port_ = port;
					break;
				}
				socket_.close();
			}
			if (port_ == 0) { throw std::runtime_error("No available dynamic/private UDP port."); }

			socket_.async_receive_from(boost::asio::buffer(buffer_), remote_endpoint_,
				[this](const boost::system::error_code& error, const size_t received_size) {
				if (error) { return; }
				socket_.async_send_to(boost::asio::buffer(buffer_.data(), received_size), remote_endpoint_,
					[](const boost::system::error_code&, size_t) {});
			});
			thread_ = std::thread([this] { io.run(); });
		}

		~udp_echo_server() {
			io.stop();
			boost::system::error_code ignored_error;
			socket_.close(ignored_error);
			if (thread_.joinable()) { thread_.join(); }
		}

		[[nodiscard]] pgl::port_number_type port() const { return port_; }

	private:
		boost::asio::io_context io;
		boost::asio::ip::udp::socket socket_;
		boost::asio::ip::udp::endpoint remote_endpoint_;
		std::array<char, 64> buffer_{};
		pgl::port_number_type port_{};
		std::thread thread_;
	};

	pgl::port_number_type find_unused_dynamic_private_tcp_port() {
		boost::asio::io_context io;
		tcp::acceptor acceptor(io);
		for (auto port = pgl::port_number_type{49152}; port < 65535; ++port) {
			boost::system::error_code error;
			acceptor.open(tcp::v4(), error);
			if (error) { continue; }
			acceptor.bind(tcp::endpoint(boost::asio::ip::address_v4::loopback(), port), error);
			acceptor.close();
			if (!error) { return port; }
		}
		throw std::runtime_error("No unused dynamic/private TCP port.");
	}

	void mark_authenticated(protocol_context& context, const pgl::player_full_name& player_full_name = {u8"host", 1}) {
		context.session_data.set_authenticated();
		context.session_data.set_client_player_name(player_full_name);
	}

	void make_room_hosted_by_client(protocol_context& context, pgl::room_data& room) {
		room.host_endpoint = pgl::endpoint::make_from_boost_endpoint(context.server_socket.remote_endpoint());
		context.server_data.get_room_data_container().add_or_update(room);
	}
}

BOOST_AUTO_TEST_SUITE(message_handlers_protocol_test)
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

	BOOST_AUTO_TEST_CASE(test_create_room_request_creates_public_room_and_replies_room_id) {
		protocol_context context;
		context.session_data.set_authenticated();
		context.session_data.set_client_player_name({u8"host", 1});
		const pgl::create_room_request_message request{
			{},
			4,
			pgl::game_host_connection_establish_mode::builtin,
			57000,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::create_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::create_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::create_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();
		const auto room = context.server_data.get_room_data_container().get(reply.room_id);

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(context.session_data.is_hosting_room());
		BOOST_CHECK_EQUAL(context.session_data.hosting_room_id(), reply.room_id);
		BOOST_CHECK(room.host_player_full_name == (pgl::player_full_name{u8"host", 1}));
		BOOST_CHECK((room.setting_flags & pgl::room_setting_flag::public_room) == pgl::room_setting_flag::public_room);
		BOOST_CHECK((room.setting_flags & pgl::room_setting_flag::open_room) == pgl::room_setting_flag::open_room);
		BOOST_CHECK_EQUAL(room.game_host_endpoint.port_number, request.port_number);
	}

	BOOST_AUTO_TEST_CASE(test_list_room_request_replies_matching_room_page) {
		protocol_context context;
		context.server_data.get_room_data_container().add_or_update(make_room(1, {u8"alice", 1}));
		context.server_data.get_room_data_container().add_or_update(make_room(2, {u8"bob", 1}));
		const pgl::list_room_request_message request{
			0,
			1,
			pgl::room_data_sort_kind::name_ascending,
			pgl::room_search_target_flag::public_room | pgl::room_search_target_flag::open_room,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::list_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::list_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::list_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK_EQUAL(reply.total_room_count, 2);
		BOOST_CHECK_EQUAL(reply.matched_room_count, 2);
		BOOST_CHECK_EQUAL(reply.reply_room_count, 1);
		BOOST_CHECK_EQUAL(reply.room_info_list[0].room_id, 1);
		BOOST_CHECK(reply.room_info_list[0].host_player_full_name == (pgl::player_full_name{u8"alice", 1}));
	}

	BOOST_AUTO_TEST_CASE(test_join_room_request_replies_host_endpoint_and_disconnects) {
		protocol_context context;
		auto room = make_room(1, {u8"host", 1}, public_open_room, {}, 2, 1);
		room.game_host_external_id[0] = 99;
		context.server_data.get_room_data_container().add_or_update(room);
		const pgl::join_room_request_message request{
			1,
			pgl::game_host_connection_establish_mode::builtin,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::join_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::join_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::join_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.game_host_endpoint == room.game_host_endpoint);
		BOOST_CHECK_EQUAL(reply.game_host_external_id[0], 99);
		BOOST_CHECK_EQUAL(context.server_data.get_room_data_container().get(1).current_player_count, 2);
	}

	BOOST_AUTO_TEST_CASE(test_join_room_request_replies_password_error_without_body) {
		protocol_context context;
		context.server_data.get_room_data_container().add_or_update(make_room(1, {u8"host", 1},
			pgl::room_setting_flag::open_room, u8"secret"));
		const pgl::join_room_request_message request{
			1,
			pgl::game_host_connection_establish_mode::builtin,
			u8"wrong"
		};
		protocol_handler_run handler(context, pgl::message_type::join_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::join_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.message_type == pgl::message_type::join_room);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::room_password_wrong);
		BOOST_CHECK_EQUAL(context.server_data.get_room_data_container().get(1).current_player_count, 1);
	}

	BOOST_AUTO_TEST_CASE(test_update_room_status_notice_closes_room_without_reply) {
		protocol_context context;
		auto room = make_room(1, {u8"host", 1});
		room.host_endpoint = pgl::endpoint::make_from_boost_endpoint(context.server_socket.remote_endpoint());
		context.server_data.get_room_data_container().add_or_update(room);
		const pgl::update_room_status_notice_message request{
			1,
			pgl::update_room_status_notice_message::status::close,
			true,
			1
		};
		protocol_handler_run handler(context, pgl::message_type::update_room_status);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::update_room_status}, request);
		const auto exception = handler.wait();
		const auto updated_room = context.server_data.get_room_data_container().get(1);

		BOOST_CHECK(!exception);
		BOOST_CHECK((updated_room.setting_flags & pgl::room_setting_flag::open_room) == pgl::room_setting_flag::none);
		BOOST_CHECK_EQUAL(updated_room.current_player_count, 1);
	}

	BOOST_AUTO_TEST_CASE(test_connection_test_request_replies_success_for_tcp_echo) {
		tcp_echo_server echo_server;
		protocol_context context;
		const pgl::connection_test_request_message request{
			pgl::transport_protocol::tcp,
			echo_server.port()
		};
		protocol_handler_run handler(context, pgl::message_type::connection_test);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::connection_test}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::connection_test_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.succeed);
	}

	BOOST_AUTO_TEST_CASE(test_connection_test_request_replies_parameter_error_for_invalid_protocol) {
		protocol_context context;
		const pgl::connection_test_request_message request{
			static_cast<pgl::transport_protocol>(255),
			57000
		};
		protocol_handler_run handler(context, pgl::message_type::connection_test);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::connection_test}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.message_type == pgl::message_type::connection_test);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::request_parameter_wrong);
	}

	BOOST_AUTO_TEST_CASE(test_keep_alive_notice_completes_without_reply) {
		protocol_context context;
		const pgl::keep_alive_notice_message request{};
		protocol_handler_run handler(context, pgl::message_type::keep_alive);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::keep_alive}, request);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
	}

	BOOST_AUTO_TEST_CASE(test_protocol_flow_disconnects_without_reply_for_invalid_message_type) {
		protocol_context context;
		protocol_handler_run handler(context);

		write_packed(context.client_socket,
			pgl::request_message_header{static_cast<pgl::message_type>(200)});
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_protocol_flow_disconnects_without_reply_when_first_message_is_not_authentication) {
		protocol_context context;
		protocol_handler_run handler(context, pgl::message_type::authentication);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::create_room});
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_protocol_flow_disconnects_without_reply_for_unimplemented_random_match) {
		protocol_context context;
		protocol_handler_run handler(context);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::random_match});
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		expect_no_more_reply_data(context.client_socket);
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

	BOOST_AUTO_TEST_CASE(test_create_room_request_creates_private_room_when_password_is_set) {
		protocol_context context;
		mark_authenticated(context);
		const pgl::create_room_request_message request{
			u8"secret",
			4,
			pgl::game_host_connection_establish_mode::builtin,
			57000,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::create_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::create_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::create_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();
		const auto room = context.server_data.get_room_data_container().get(reply.room_id);

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK((room.setting_flags & pgl::room_setting_flag::public_room) == pgl::room_setting_flag::none);
		BOOST_CHECK((room.setting_flags & pgl::room_setting_flag::open_room) == pgl::room_setting_flag::open_room);
		BOOST_CHECK(room.password == request.password);
	}

	BOOST_AUTO_TEST_CASE(test_create_room_request_creates_external_service_room_without_port_validation) {
		protocol_context context;
		mark_authenticated(context);
		pgl::game_host_external_id_t external_id{};
		external_id[0] = 7;
		const pgl::create_room_request_message request{
			{},
			4,
			pgl::game_host_connection_establish_mode::steam,
			1,
			external_id
		};
		protocol_handler_run handler(context, pgl::message_type::create_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::create_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::create_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();
		const auto room = context.server_data.get_room_data_container().get(reply.room_id);

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(room.game_host_connection_establish_mode == pgl::game_host_connection_establish_mode::steam);
		BOOST_CHECK_EQUAL(room.game_host_external_id[0], 7);
	}

	BOOST_AUTO_TEST_CASE(test_create_room_request_replies_parameter_error_for_invalid_builtin_port) {
		protocol_context context;
		mark_authenticated(context);
		const pgl::create_room_request_message request{
			{},
			4,
			pgl::game_host_connection_establish_mode::builtin,
			49151,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::create_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::create_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::request_parameter_wrong);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_create_room_request_replies_parameter_error_for_invalid_max_player_count) {
		protocol_context context;
		mark_authenticated(context);
		const pgl::create_room_request_message request{
			{},
			0,
			pgl::game_host_connection_establish_mode::builtin,
			57000,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::create_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::create_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::request_parameter_wrong);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_create_room_request_replies_client_already_hosting_room) {
		protocol_context context;
		mark_authenticated(context);
		context.session_data.set_hosting_room_id(1);
		const pgl::create_room_request_message request{
			{},
			4,
			pgl::game_host_connection_establish_mode::builtin,
			57000,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::create_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::create_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::client_already_hosting_room);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_create_room_request_replies_room_count_exceeds_limit) {
		protocol_context context;
		mark_authenticated(context);
		context.setting.common.max_room_count = 1;
		context.server_data.get_room_data_container().add_or_update(make_room(1, {u8"other", 1}));
		const pgl::create_room_request_message request{
			{},
			4,
			pgl::game_host_connection_establish_mode::builtin,
			57000,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::create_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::create_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::room_count_exceeds_limit);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_list_room_request_supports_all_sort_kinds) {
		const auto first_room_id_for = [](const pgl::room_data_sort_kind sort_kind) {
			protocol_context context;
			context.server_data.get_room_data_container().add_or_update(make_room(1, {u8"bob", 2},
				public_open_room, {}, 4, 1));
			context.server_data.get_room_data_container().add_or_update(make_room(2, {u8"alice", 1},
				public_open_room, {}, 4, 1));
			auto late_room = make_room(3, {u8"bob", 1}, public_open_room, {}, 4, 1);
			late_room.create_datetime = pgl::datetime(2024, 1, 31);
			context.server_data.get_room_data_container().add_or_update(late_room);
			const pgl::list_room_request_message request{
				0,
				1,
				sort_kind,
				pgl::room_search_target_flag::public_room | pgl::room_search_target_flag::open_room,
				{}
			};
			protocol_handler_run handler(context, pgl::message_type::list_room);

			write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::list_room}, request);
			const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
			const auto reply = read_packed<pgl::list_room_reply_message>(context.client_socket);
			const auto exception = handler.wait();

			BOOST_CHECK(!exception);
			BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
			return reply.room_info_list[0].room_id;
		};

		BOOST_CHECK_EQUAL(first_room_id_for(pgl::room_data_sort_kind::name_ascending), 2);
		BOOST_CHECK_EQUAL(first_room_id_for(pgl::room_data_sort_kind::name_descending), 1);
		BOOST_CHECK_EQUAL(first_room_id_for(pgl::room_data_sort_kind::create_datetime_ascending), 1);
		BOOST_CHECK_EQUAL(first_room_id_for(pgl::room_data_sort_kind::create_datetime_descending), 3);
	}

	BOOST_AUTO_TEST_CASE(test_list_room_request_replies_empty_result_body) {
		protocol_context context;
		const pgl::list_room_request_message request{
			0,
			10,
			pgl::room_data_sort_kind::name_ascending,
			pgl::room_search_target_flag::public_room | pgl::room_search_target_flag::open_room,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::list_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::list_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::list_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK_EQUAL(reply.total_room_count, 0);
		BOOST_CHECK_EQUAL(reply.matched_room_count, 0);
		BOOST_CHECK_EQUAL(reply.reply_room_count, 0);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_list_room_request_replies_zero_rooms_when_start_index_is_out_of_range) {
		protocol_context context;
		context.server_data.get_room_data_container().add_or_update(make_room(1, {u8"alice", 1}));
		context.server_data.get_room_data_container().add_or_update(make_room(2, {u8"bob", 1}));
		const pgl::list_room_request_message request{
			3,
			10,
			pgl::room_data_sort_kind::name_ascending,
			pgl::room_search_target_flag::public_room | pgl::room_search_target_flag::open_room,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::list_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::list_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::list_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK_EQUAL(reply.total_room_count, 2);
		BOOST_CHECK_EQUAL(reply.matched_room_count, 2);
		BOOST_CHECK_EQUAL(reply.reply_room_count, 0);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_list_room_request_splits_replies_when_result_exceeds_one_message_capacity) {
		protocol_context context;
		for (auto i = pgl::room_id_t{1}; i <= 7; ++i) {
			context.server_data.get_room_data_container().add_or_update(
				make_room(i, {u8"host", static_cast<pgl::player_tag_t>(i)}));
		}
		const pgl::list_room_request_message request{
			0,
			7,
			pgl::room_data_sort_kind::create_datetime_ascending,
			pgl::room_search_target_flag::public_room | pgl::room_search_target_flag::open_room,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::list_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::list_room}, request);
		const auto first_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto first_reply = read_packed<pgl::list_room_reply_message>(context.client_socket);
		const auto second_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto second_reply = read_packed<pgl::list_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(first_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(second_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK_EQUAL(first_reply.reply_room_count, 7);
		BOOST_CHECK_EQUAL(first_reply.room_info_list[0].room_id, 1);
		BOOST_CHECK_EQUAL(first_reply.room_info_list[5].room_id, 6);
		BOOST_CHECK_EQUAL(second_reply.reply_room_count, 7);
		BOOST_CHECK_EQUAL(second_reply.room_info_list[0].room_id, 7);
		BOOST_CHECK_EQUAL(second_reply.room_info_list[1].room_id, 0);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_list_room_request_filters_by_room_status_name_and_tag) {
		protocol_context context;
		context.server_data.get_room_data_container().add_or_update(make_room(1, {u8"alice", 7},
			pgl::room_setting_flag::none));
		context.server_data.get_room_data_container().add_or_update(make_room(2, {u8"alina", 8},
			pgl::room_setting_flag::none));
		context.server_data.get_room_data_container().add_or_update(make_room(3, {u8"alicia", 7},
			pgl::room_setting_flag::open_room));
		context.server_data.get_room_data_container().add_or_update(make_room(4, {u8"alice-public", 7},
			public_open_room));
		const pgl::list_room_request_message request{
			0,
			10,
			pgl::room_data_sort_kind::name_ascending,
			pgl::room_search_target_flag::private_room | pgl::room_search_target_flag::closed_room,
			{u8"ali", 7}
		};
		protocol_handler_run handler(context, pgl::message_type::list_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::list_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::list_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK_EQUAL(reply.total_room_count, 4);
		BOOST_CHECK_EQUAL(reply.matched_room_count, 1);
		BOOST_CHECK_EQUAL(reply.reply_room_count, 1);
		BOOST_CHECK_EQUAL(reply.room_info_list[0].room_id, 1);
	}

	BOOST_AUTO_TEST_CASE(test_list_room_request_replies_parameter_error_for_invalid_sort_kind) {
		protocol_context context;
		const pgl::list_room_request_message request{
			0,
			10,
			static_cast<pgl::room_data_sort_kind>(255),
			pgl::room_search_target_flag::public_room | pgl::room_search_target_flag::open_room,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::list_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::list_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::request_parameter_wrong);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_join_room_request_accepts_private_room_with_correct_password) {
		protocol_context context;
		auto room = make_room(1, {u8"host", 1}, pgl::room_setting_flag::open_room, u8"secret", 2, 1);
		context.server_data.get_room_data_container().add_or_update(room);
		const pgl::join_room_request_message request{
			1,
			pgl::game_host_connection_establish_mode::builtin,
			u8"secret"
		};
		protocol_handler_run handler(context, pgl::message_type::join_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::join_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::join_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.game_host_endpoint == room.game_host_endpoint);
	}

	BOOST_AUTO_TEST_CASE(test_join_room_request_ignores_password_for_public_room) {
		protocol_context context;
		auto room = make_room(1, {u8"host", 1}, public_open_room, {}, 2, 1);
		context.server_data.get_room_data_container().add_or_update(room);
		const pgl::join_room_request_message request{
			1,
			pgl::game_host_connection_establish_mode::builtin,
			u8"wrong"
		};
		protocol_handler_run handler(context, pgl::message_type::join_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::join_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::join_room_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(is_intended_disconnect(exception));
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.game_host_endpoint == room.game_host_endpoint);
	}

	BOOST_AUTO_TEST_CASE(test_join_room_request_replies_room_not_found) {
		protocol_context context;
		const pgl::join_room_request_message request{
			404,
			pgl::game_host_connection_establish_mode::builtin,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::join_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::join_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::room_not_found);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_join_room_request_replies_permission_denied_for_closed_room) {
		protocol_context context;
		context.server_data.get_room_data_container().add_or_update(make_room(1, {u8"host", 1},
			pgl::room_setting_flag::public_room));
		const pgl::join_room_request_message request{
			1,
			pgl::game_host_connection_establish_mode::builtin,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::join_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::join_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::room_permission_denied);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_join_room_request_replies_room_full) {
		protocol_context context;
		context.server_data.get_room_data_container().add_or_update(make_room(1, {u8"host", 1},
			public_open_room, {}, 2, 2));
		const pgl::join_room_request_message request{
			1,
			pgl::game_host_connection_establish_mode::builtin,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::join_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::join_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::room_full);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_join_room_request_replies_connection_establish_mode_mismatch) {
		protocol_context context;
		auto room = make_room(1, {u8"host", 1});
		room.game_host_connection_establish_mode = pgl::game_host_connection_establish_mode::steam;
		context.server_data.get_room_data_container().add_or_update(room);
		const pgl::join_room_request_message request{
			1,
			pgl::game_host_connection_establish_mode::builtin,
			{}
		};
		protocol_handler_run handler(context, pgl::message_type::join_room);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::join_room}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::room_connection_establish_mode_mismatch);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_update_room_status_notice_opens_room_without_reply) {
		protocol_context context;
		auto room = make_room(1, {u8"host", 1}, pgl::room_setting_flag::public_room, {}, 4, 1);
		make_room_hosted_by_client(context, room);
		const pgl::update_room_status_notice_message request{
			1,
			pgl::update_room_status_notice_message::status::open,
			true,
			2
		};
		protocol_handler_run handler(context, pgl::message_type::update_room_status);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::update_room_status}, request);
		const auto exception = handler.wait();
		const auto updated_room = context.server_data.get_room_data_container().get(1);

		BOOST_CHECK(!exception);
		BOOST_CHECK((updated_room.setting_flags & pgl::room_setting_flag::open_room) == pgl::room_setting_flag::open_room);
		BOOST_CHECK_EQUAL(updated_room.current_player_count, 2);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_update_room_status_notice_removes_room_without_reply) {
		protocol_context context;
		auto room = make_room(1, {u8"host", 1});
		make_room_hosted_by_client(context, room);
		context.session_data.set_hosting_room_id(1);
		const pgl::update_room_status_notice_message request{
			1,
			pgl::update_room_status_notice_message::status::remove,
			false,
			0
		};
		protocol_handler_run handler(context, pgl::message_type::update_room_status);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::update_room_status}, request);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(!context.server_data.get_room_data_container().contains(1));
		BOOST_CHECK(!context.session_data.is_hosting_room());
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_update_room_status_notice_ignores_missing_room_without_reply) {
		protocol_context context;
		const pgl::update_room_status_notice_message request{
			404,
			pgl::update_room_status_notice_message::status::close,
			false,
			0
		};
		protocol_handler_run handler(context, pgl::message_type::update_room_status);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::update_room_status}, request);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_update_room_status_notice_ignores_host_mismatch_without_reply) {
		protocol_context context;
		context.server_data.get_room_data_container().add_or_update(make_room(1, {u8"host", 1}));
		const pgl::update_room_status_notice_message request{
			1,
			pgl::update_room_status_notice_message::status::close,
			false,
			0
		};
		protocol_handler_run handler(context, pgl::message_type::update_room_status);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::update_room_status}, request);
		const auto exception = handler.wait();
		const auto room = context.server_data.get_room_data_container().get(1);

		BOOST_CHECK(!exception);
		BOOST_CHECK((room.setting_flags & pgl::room_setting_flag::open_room) == pgl::room_setting_flag::open_room);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_update_room_status_notice_ignores_invalid_player_count_without_reply) {
		protocol_context context;
		auto room = make_room(1, {u8"host", 1}, public_open_room, {}, 2, 1);
		make_room_hosted_by_client(context, room);
		const pgl::update_room_status_notice_message request{
			1,
			pgl::update_room_status_notice_message::status::close,
			true,
			3
		};
		protocol_handler_run handler(context, pgl::message_type::update_room_status);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::update_room_status}, request);
		const auto exception = handler.wait();
		const auto updated_room = context.server_data.get_room_data_container().get(1);

		BOOST_CHECK(!exception);
		BOOST_CHECK((updated_room.setting_flags & pgl::room_setting_flag::open_room) == pgl::room_setting_flag::open_room);
		BOOST_CHECK_EQUAL(updated_room.current_player_count, 1);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_update_room_status_notice_ignores_invalid_status_without_reply) {
		protocol_context context;
		auto room = make_room(1, {u8"host", 1});
		make_room_hosted_by_client(context, room);
		const auto invalid_status = static_cast<decltype(pgl::update_room_status_notice_message::status)>(255);
		const pgl::update_room_status_notice_message request{
			1,
			invalid_status,
			false,
			0
		};
		protocol_handler_run handler(context, pgl::message_type::update_room_status);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::update_room_status}, request);
		const auto exception = handler.wait();
		const auto updated_room = context.server_data.get_room_data_container().get(1);

		BOOST_CHECK(!exception);
		BOOST_CHECK((updated_room.setting_flags & pgl::room_setting_flag::open_room) == pgl::room_setting_flag::open_room);
		expect_no_more_reply_data(context.client_socket);
	}

	BOOST_AUTO_TEST_CASE(test_connection_test_request_replies_success_for_udp_echo) {
		udp_echo_server echo_server;
		protocol_context context;
		const pgl::connection_test_request_message request{
			pgl::transport_protocol::udp,
			echo_server.port()
		};
		protocol_handler_run handler(context, pgl::message_type::connection_test);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::connection_test}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::connection_test_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(reply.succeed);
	}

	BOOST_AUTO_TEST_CASE(test_connection_test_request_replies_failure_for_unreachable_tcp_endpoint) {
		protocol_context context;
		const pgl::connection_test_request_message request{
			pgl::transport_protocol::tcp,
			find_unused_dynamic_private_tcp_port()
		};
		protocol_handler_run handler(context, pgl::message_type::connection_test);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::connection_test}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::connection_test_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(!reply.succeed);
	}

	BOOST_AUTO_TEST_CASE(test_connection_test_request_replies_failure_for_unreachable_udp_endpoint) {
		protocol_context context;
		context.setting.connection_test.connection_check_udp_time_out_seconds = 1;
		context.setting.connection_test.connection_check_udp_try_count = 1;
		const pgl::connection_test_request_message request{
			pgl::transport_protocol::udp,
			find_unused_dynamic_private_tcp_port()
		};
		protocol_handler_run handler(context, pgl::message_type::connection_test);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::connection_test}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto reply = read_packed<pgl::connection_test_reply_message>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::ok);
		BOOST_CHECK(!reply.succeed);
	}

	BOOST_AUTO_TEST_CASE(test_connection_test_request_replies_parameter_error_for_invalid_port) {
		protocol_context context;
		const pgl::connection_test_request_message request{
			pgl::transport_protocol::tcp,
			49151
		};
		protocol_handler_run handler(context, pgl::message_type::connection_test);

		write_packed(context.client_socket, pgl::request_message_header{pgl::message_type::connection_test}, request);
		const auto reply_header = read_packed<pgl::reply_message_header>(context.client_socket);
		const auto exception = handler.wait();

		BOOST_CHECK(!exception);
		BOOST_CHECK(reply_header.error_code == pgl::message_error_code::request_parameter_wrong);
		expect_no_more_reply_data(context.client_socket);
	}

BOOST_AUTO_TEST_SUITE_END()
