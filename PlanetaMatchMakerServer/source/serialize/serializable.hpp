#pragma once

namespace pgl {
	class serializer;

	class serializable {
	public:
		virtual ~serializable() = default;
		virtual void on_serialize(serializer& serializer) = 0;
	};
}
