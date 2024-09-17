#pragma once

#include <json.hpp>

namespace Utils::JSON
{
	std::string TypeToString(nlohmann::json::value_t type);

	unsigned long ReadFlags(const std::string binaryFlags, size_t size);

	Game::Bounds ReadBounds(const nlohmann::json& value);

	template <typename T> void CopyArray(T* destination, const nlohmann::json& json_member, size_t count = 0)
	{
		if (count == 0)
		{
			count = json_member.size();
		}

		for (size_t i = 0; i < count; i++)
		{
			destination[i] = json_member[i].get<T>();
		}
	}
}
