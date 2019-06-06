#pragma once
#include <memory>

namespace pgl {
	class message_handler_container;

	class message_handler_container_factory {
	public:
		static std::shared_ptr<message_handler_container> make_standard_message_handler_container();
	};
}
