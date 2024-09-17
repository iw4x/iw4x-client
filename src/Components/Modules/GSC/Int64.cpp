#include <STDInclude.hpp>
#include "Int64.hpp"
#include "Script.hpp"

#define INT64_OPERATION(expr) [](const std::int64_t a, [[maybe_unused]] const std::int64_t b) { return expr; }

namespace Components::GSC
{
	std::unordered_map<std::string, Int64::int64_OP> Int64::Operations =
	{
		{"+",  INT64_OPERATION(a + b)},
		{"-",  INT64_OPERATION(a - b)},
		{"*",  INT64_OPERATION(a * b)},
		{"/",  INT64_OPERATION(a / b)},
		{"&",  INT64_OPERATION(a & b)},
		{"^",  INT64_OPERATION(a ^ b)},
		{"|",  INT64_OPERATION(a | b)},
		{"~",  INT64_OPERATION(~a)},
		{"%",  INT64_OPERATION(a % b)},
		{">>", INT64_OPERATION(a >> b)},
		{"<<", INT64_OPERATION(a << b)},
		{"++", INT64_OPERATION(a + 1)},
		{"--", INT64_OPERATION(a - 1)},
	};

	std::unordered_map<std::string, Int64::int64_Comp> Int64::Comparisons
	{
		{">",  INT64_OPERATION(a > b)},
		{">=", INT64_OPERATION(a >= b)},
		{"==", INT64_OPERATION(a == b)},
		{"<=", INT64_OPERATION(a <= b)},
		{"<",  INT64_OPERATION(a < b)},
	};

	std::int64_t Int64::GetInt64Arg(unsigned int index, bool optional)
	{
		if ((optional) && (index >= Game::Scr_GetNumParam()))
		{
			return 0;
		}

		if (Game::Scr_GetType(index) == Game::VAR_INTEGER)
		{
			return Game::Scr_GetInt(index);
		}

		if (Game::Scr_GetType(index) == Game::VAR_STRING)
		{
			return std::strtoll(Game::Scr_GetString(index), nullptr, 0);
		}

		Game::Scr_ParamError(index, Utils::String::VA("cannot cast %s to int64", Game::Scr_GetTypeName(index)));
		return 0;
	}

	void Int64::AddFunctions()
	{
		Script::AddFunction("Int64IsInt", []
		{
			const auto value = GetInt64Arg(0, false);
			Game::Scr_AddBool(value <= std::numeric_limits<std::int32_t>::max() && value >= std::numeric_limits<std::int32_t>::min());
		});

		Script::AddFunction("Int64ToInt", []
		{
			Game::Scr_AddInt(static_cast<std::int32_t>(GetInt64Arg(0, false)));
		});

		Script::AddFunction("Int64OP", []
		{
			const auto a = GetInt64Arg(0, false);
			const auto* op = Game::Scr_GetString(1);
			const auto b = GetInt64Arg(2, true);

			{
				if (const auto itr = Operations.find(op); itr != Operations.end())
				{
					Game::Scr_AddString(Utils::String::VA("%lld", itr->second(a, b)));
					return;
				}
			}

			{
				if (const auto itr = Comparisons.find(op); itr != Comparisons.end())
				{
					Game::Scr_AddBool(itr->second(a, b));
					return;
				}
			}

			Game::Scr_ParamError(1, "Invalid int64 operation");
		});
	}

	Int64::Int64()
	{
		AddFunctions();
	}
}
