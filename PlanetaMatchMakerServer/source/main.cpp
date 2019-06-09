#include "server/match_making_server.hpp"
#include "server/server_data.hpp"
#include "message/message_handler_invoker_factory.hpp"

using namespace pgl;
using namespace boost;

int main() {
	const auto message_handler_invkr = message_handler_invoker_factory::make_standard();
	const auto server_dat = std::make_shared<server_data>();
	asio::io_service io_service;
	// prevent to stop server when all request are processed
	asio::io_service::work work(io_service);
	match_making_server server(server_dat, message_handler_invkr, io_service, ip_version::v4, 7777, 30);
	for (auto i = 0; i < 1; ++i) {
		server.start();
	}
	io_service.run();
}
