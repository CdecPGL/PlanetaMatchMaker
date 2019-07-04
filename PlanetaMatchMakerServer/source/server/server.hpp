#pragma once

#include <cstdint>
#include <memory>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

#include "network/network_layer.hpp"
#include "session/session_data.hpp"
#include "./server_setting.hpp"
#include "./server_data.hpp"

namespace pgl {
	class server final : boost::noncopyable {
	public:
		server();
		void run();
	private:
		boost::asio::io_service io_service_;
		boost::asio::ip::tcp::acceptor acceptor_;
		std::unique_ptr<server_data> server_data_;
		server_setting server_setting_;

		void load_setting(const std::string& setting_path);
	};
}
