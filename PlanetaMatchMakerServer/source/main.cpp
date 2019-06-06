#include "matching_server/match_making_server.hpp"
#include "message/message_handler_container_factory.hpp"

using namespace pgl;
using namespace boost;

int main() {
	const auto message_handler_container = message_handler_container_factory::make_standard_message_handler_container();
	asio::io_service io_service;
	//ˆ—‚ª‚È‚­‚Ä‚àI—¹‚µ‚È‚¢‚æ‚¤‚É‚·‚é
	asio::io_service::work work(io_service);
	match_making_server server(message_handler_container, io_service, ip_version::v4, 7777, 30);
	server.start();
	server.start();
	io_service.run();
}
