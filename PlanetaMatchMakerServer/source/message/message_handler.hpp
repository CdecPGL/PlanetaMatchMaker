#pragma once

#include <boost/asio/spawn.hpp>

#include "minimal_serializer/serializer.hpp"
#include "client/client_errors.hpp"
#include "message_handle_utilities.hpp"
#include "message_handle_parameter.hpp"

namespace pgl {
	class message_handler {
	public:
		message_handler() = default;
		message_handler(const message_handler& message_handler) = delete;
		message_handler(message_handler&& message_handler) = delete;
		virtual ~message_handler() = default;
		message_handler& operator=(const message_handler& message_handler) = delete;
		message_handler& operator=(message_handler&& message_handler) = delete;
		virtual void operator()(const request_message_header& header,
			std::shared_ptr<message_handle_parameter> param) = 0;
		[[nodiscard]] virtual size_t get_message_size() const = 0;
	};

	template <class ReplyMessage>
	struct message_handling_result {
		std::vector<ReplyMessage> reply_bodies;
		bool is_disconnect_required;
	};

	class no_reply final { };

	template <class RequestMessage, class ReplyMessage = no_reply>
	class message_handler_base : public message_handler {
	public:
		using handle_return_t = message_handling_result<ReplyMessage>;

		message_handler_base() = default;
		message_handler_base(const message_handler_base& message_handler_base) = delete;
		message_handler_base(message_handler_base&& message_handler_base) = delete;
		~message_handler_base() override = default;
		message_handler_base& operator=(const message_handler_base& message_handler_base) = delete;
		message_handler_base& operator=(message_handler_base&& message_handler_base) = delete;

		[[nodiscard]] size_t get_message_size() const final {
			return minimal_serializer::serialized_size_v<RequestMessage>;
		}

		void operator()(const request_message_header& header, std::shared_ptr<message_handle_parameter> param) final {
			// receive message
			log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Receive ", header.message_type,
				" message.");
			RequestMessage message{};
			receive(param, message);

			bool is_disconnect_required;
			std::string disconnect_reason;
			reply_message_header reply_header{
				header.message_type,
				message_error_code::ok,
			};
			std::vector<ReplyMessage> reply_bodies;
			try {
				// handle message
				log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Handle ",
					header.message_type, " message.");
				const auto result = handle_message(message, param);
				is_disconnect_required = result.is_disconnect_required;
				reply_bodies = std::move(result.reply_bodies);
				reply_header.error_code = message_error_code::ok;
				disconnect_reason = "Disconnect due to message handling result.";
			}
			catch (const client_error& e) {
				log_with_endpoint(log_level::info, param->socket.remote_endpoint(),
					"Client error occurred while handling ", header.message_type,
					" message: ", e.message());
				is_disconnect_required = e.is_disconnect_required();
				// don't reply bodies
				reply_bodies = {};
				reply_header.error_code = get_message_error_code_from_client_error_code(e.error_code());
				disconnect_reason = "Disconnect due to not continuable client error.";
			}
			catch (const server_error& e) {
				log_with_endpoint(log_level::info, param->socket.remote_endpoint(),
					"Server error occurred while handling ", header.message_type,
					" message: ", e.message());
				is_disconnect_required = e.is_disconnect_required();
				// don't reply bodies
				reply_bodies = {};
				reply_header.error_code = message_error_code::server_error;
				disconnect_reason = "Disconnect due to not continuable server error.";
			}

			// reply message if required
			if constexpr (!std::is_same_v<ReplyMessage, no_reply>) {
				log_with_endpoint(log_level::info, param->socket.remote_endpoint(), "Reply ", header.message_type,
					" message.");
				for (auto&& reply_body : reply_bodies) { send(param, reply_header, reply_body); }
			}

			// disconnect connection if required
			if (is_disconnect_required) { throw server_session_intended_disconnect_error(disconnect_reason); }
		}

	private:
		/**
		 * @brief Handle message. If ReplyMessage is no_reply, reply message bodies are ignored.
		 * - Succeeded: Return reply message bodies and whether disconnect is required.
		 * - Client Error: Throw client_error with correct client error code.
		 * - Server Error: Throw server_error.
		 * @param message A message body.
		 * @param param A parameters for message handling.
		 * @return A tuple of reply message and whether disconnect is required.
		 */
		virtual handle_return_t handle_message(const RequestMessage& message,
			std::shared_ptr<message_handle_parameter> param) = 0;
	};
}
