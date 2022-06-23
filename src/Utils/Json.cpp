#include <STDInclude.hpp>

namespace Utils::Json
{
	std::string TypeToString(json11::Json::Type type)
	{
		switch (type)
		{
		case json11::Json::NUL:
			return "NUL";
		case json11::Json::NUMBER:
			return "NUMBER";
		case json11::Json::BOOL:
			return "BOOL";
		case json11::Json::STRING:
			return "STRING";
		case json11::Json::ARRAY:
			return "ARRAY";
		case json11::Json::OBJECT:
			return "OBJECT";
		default:
			return "NUL";
		}
	}
}
