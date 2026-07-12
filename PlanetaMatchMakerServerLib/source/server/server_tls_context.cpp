#include "server_tls_context.hpp"

namespace pgl {
	std::shared_ptr<boost::asio::ssl::context> server_tls_context::make_context(
		const server_tls_setting& setting) {
		if (setting.mode != server_tls_mode::tls) { return {}; }

		auto context = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tls_server);
		context->set_options(
			boost::asio::ssl::context::default_workarounds |
			boost::asio::ssl::context::no_sslv2 |
			boost::asio::ssl::context::no_sslv3 |
			boost::asio::ssl::context::no_tlsv1 |
			boost::asio::ssl::context::no_tlsv1_1);
		context->use_certificate_chain_file(setting.certificate_path.string());
		context->use_private_key_file(setting.private_key_path.string(), boost::asio::ssl::context::pem);
		return context;
	}

	void server_tls_context::reload(const server_tls_setting& setting) {
		auto context = make_context(setting);
		std::lock_guard lock(mutex_);
		context_ = std::move(context);
	}

	std::shared_ptr<boost::asio::ssl::context> server_tls_context::current() const {
		std::lock_guard lock(mutex_);
		return context_;
	}
}
