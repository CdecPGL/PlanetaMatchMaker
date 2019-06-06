#pragma once
#include <memory>
#include <boost/noncopyable.hpp>

namespace pgl {
	class message_handler_container;

	class message_handler_container_factory final : boost::noncopyable {
	public:
		static std::shared_ptr<message_handler_container> make_standard_message_handler_container();
	};
}
