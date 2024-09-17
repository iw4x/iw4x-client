#include <STDInclude.hpp>
#include <bitset>

#include "JSON.hpp"

namespace Utils::JSON
{
	std::string TypeToString(const nlohmann::json::value_t type)
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
		case nlohmann::json::value_t::binary:
			return "binary";
		case nlohmann::json::value_t::discarded:
			return "discarded";
		default:
			AssertUnreachable;
			return "null";
		}
	}

	unsigned long ReadFlags(const std::string binaryFlags, std::size_t size)
	{
		std::bitset<64>	input;
		const auto binarySize = size * 8;

		if (binaryFlags.size() > binarySize)
		{
			Components::Logger::Print("Flag {} might not be properly translated, it seems to contain an error (invalid length)\n", binaryFlags);
			return 0;
		}

		auto i = binarySize - 1;
		for (char bit : binaryFlags)
		{
			if (i < 0)
			{
				Components::Logger::Print("Flag {} might not be properly translated, it seems to contain an error (invalid length)\n", binaryFlags);
				break;
			}

			auto isOne = bit == '1';
			input.set(i--, isOne);
		}

		return input.to_ulong();
	}

	Game::Bounds ReadBounds(const nlohmann::json& value)
	{
		Game::Bounds bounds{};
		CopyArray(bounds.midPoint, value["midPoint"]);
		CopyArray(bounds.halfSize, value["halfSize"]);

		return bounds;
	}
}
