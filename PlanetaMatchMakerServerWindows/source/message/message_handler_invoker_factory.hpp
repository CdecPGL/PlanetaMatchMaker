#pragma once
#include <memory>
#include <boost/noncopyable.hpp>

namespace pgl {
	class message_handler_invoker;

	class message_handler_invoker_factory final : boost::noncopyable {
	public:
		static std::shared_ptr<message_handler_invoker> make_shared_standard();
		static std::unique_ptr<message_handler_invoker> make_unique_standard();
	};
}
