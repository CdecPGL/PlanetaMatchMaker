#include <ostream>

#include "minimal_serializer/string_utility.hpp"

#include "logger_common.hpp"
#include "datetime/datetime.hpp"

using namespace std;
using namespace minimal_serializer;

namespace pgl {
	void output_formatted_log(ostream& out_stream, log_level level, const string& header,
		const std::string& message) {
		out_stream << "[" << get_now_datetime_string() << "] " << generate_string(level) << header
			<< ": " << message << std::endl;
	}
}
