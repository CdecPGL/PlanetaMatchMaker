#pragma once

#include <memory>
#include <mutex>

#include <boost/asio/ssl.hpp>
#include <boost/noncopyable.hpp>

#include "server_setting.hpp"

namespace pgl {
	class server_tls_context final : boost::noncopyable {
	public:
		void reload(const server_tls_setting& setting);
		[[nodiscard]] std::shared_ptr<boost::asio::ssl::context> current() const;

	private:
		static std::shared_ptr<boost::asio::ssl::context> make_context(const server_tls_setting& setting);

		mutable std::mutex mutex_;
		std::shared_ptr<boost::asio::ssl::context> context_;
	};
}
