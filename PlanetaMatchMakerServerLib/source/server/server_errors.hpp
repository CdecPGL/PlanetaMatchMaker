#pragma once

#include <string>
#include <iostream>

namespace pgl {
	/**
	 * @brief A class of errors due to server in message handling.
	 */
	class server_error final : public std::exception {
	public:
		explicit server_error(bool is_disconnect_required, std::string extra_message = "");
		[[nodiscard]] bool is_disconnect_required() const;
		[[nodiscard]] std::string extra_message() const;
		[[nodiscard]] std::string message() const;

	private:
		const std::string extra_message_;
		const bool is_disconnect_required_;
	};

	std::ostream& operator <<(std::ostream& os, const server_error& error);

	/**
	 * @brief A class to communicate that intended disconnection is required.
	 */
	class server_session_intended_disconnect_error final : public std::exception {
	public:
		explicit server_session_intended_disconnect_error(std::string extra_message = "");
		[[nodiscard]] std::string extra_message() const;
		[[nodiscard]] std::string message() const;

	private:
		const std::string extra_message_;
	};

	std::ostream& operator <<(std::ostream& os, const server_session_intended_disconnect_error& error);


	/**
	 * @brief A class of errors of server session.
	 */
	class server_session_error final : public std::exception {
	public:
		explicit server_session_error(std::string extra_message = "");
		[[nodiscard]] std::string extra_message() const;
		[[nodiscard]] std::string message() const;
	private:
		std::string extra_message_;
	};

	std::ostream& operator <<(std::ostream& os, const server_session_error& error);
}
