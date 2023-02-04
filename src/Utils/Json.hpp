#pragma once
#include <json.hpp>

namespace Utils::Json
{
	std::string TypeToString(nlohmann::json::value_t type);

	unsigned long ReadFlags(std::string binaryFlags, size_t size);
}
