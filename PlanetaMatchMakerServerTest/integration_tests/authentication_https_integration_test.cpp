#include <chrono>
#include <exception>
#include <iostream>

#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>

#include "authentication/authentication_execution_context.hpp"
#include "authentication/authentication_http_client.hpp"

int main() {
	boost::asio::io_context io_context;
	int exit_code = 1;
	bool probe_started = false;

	boost::asio::spawn(io_context, [&](boost::asio::yield_context yield) {
		probe_started = true;
		try {
			const pgl::authentication_execution_context context{
				io_context.get_executor(), yield, std::chrono::steady_clock::now() + std::chrono::seconds(30), false
			};
			const auto response = pgl::authentication_http::get(
				"https://api.steampowered.com/ISteamWebAPIUtil/GetServerInfo/v1/", context);
			if (response.status != 200) {
				std::cerr << "Steam HTTPS probe returned status " << response.status << '.' << std::endl;
				return;
			}
			exit_code = 0;
		}
		catch (const std::exception& exception) {
			std::cerr << "Steam HTTPS probe failed: " << exception.what() << std::endl;
		}
	}, boost::asio::detached);

	io_context.run();
	if (!probe_started) {
		std::cerr << "Steam HTTPS probe did not start." << std::endl;
	}
	return exit_code;
}
