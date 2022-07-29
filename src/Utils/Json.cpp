#include <STDInclude.hpp>

namespace Utils::Json
{
	std::string TypeToString(nlohmann::json::value_t type)
	{
		switch (type)
		{
		case nlohmann::json::value_t::null:
			return "null";
		case nlohmann::json::value_t::number_integer:
			return "number_integer";
		case nlohmann::json::value_t::number_unsigned:
			return "number_unsigned";
		case nlohmann::json::value_t::number_float:
			return "number_float";
		case nlohmann::json::value_t::boolean:
			return "boolean";
		case nlohmann::json::value_t::string:
			return "string";
		case nlohmann::json::value_t::array:
			return "array";
		case nlohmann::json::value_t::object:
			return "object";
		default:
			return "null";
		}
	}
}
