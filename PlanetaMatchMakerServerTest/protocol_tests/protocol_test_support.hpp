#pragma once

#include <array>
#include <chrono>
#include <exception>
#include <future>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/test/unit_test.hpp>

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

namespace pgl::test {
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

	inline void expect_no_more_reply_data(tcp::socket& socket) {
		BOOST_CHECK_EQUAL(socket.available(), 0);
	}

	inline bool is_intended_disconnect(const std::exception_ptr& exception) {
		if (!exception) { return false; }

		try { std::rethrow_exception(exception); }
		catch (const pgl::server_session_intended_disconnect_error&) { return true; }
		catch (...) { return false; }
	}

	inline pgl::server_setting make_protocol_test_setting() {
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

	inline pgl::endpoint make_endpoint(const pgl::port_number_type port_number) {
		auto endpoint = pgl::endpoint();
		endpoint.port_number = port_number;
		return endpoint;
	}

	inline pgl::room_data make_room(const pgl::room_id_t room_id, const pgl::player_full_name& host_full_name,
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

	inline pgl::port_number_type find_unused_dynamic_private_tcp_port() {
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

	inline void mark_authenticated(protocol_context& context,
		const pgl::player_full_name& player_full_name = {u8"host", 1}) {
		context.session_data.set_authenticated();
		context.session_data.set_client_player_name(player_full_name);
	}

	inline void make_room_hosted_by_client(protocol_context& context, pgl::room_data& room) {
		room.host_endpoint = pgl::endpoint::make_from_boost_endpoint(context.server_socket.remote_endpoint());
		context.server_data.get_room_data_container().add_or_update(room);
	}
}
