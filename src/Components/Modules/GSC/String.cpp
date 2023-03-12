#include <STDInclude.hpp>

#include "Script.hpp"
#include "String.hpp"

namespace Components::GSC
{
	void String::AddScriptFunctions()
	{
		Script::AddFunction("ToUpper", [] // gsc: ToUpper(<string>)
		{
			const auto scriptValue = Game::Scr_GetConstString(0);
			const auto* string = Game::SL_ConvertToString(scriptValue);

			char out[1024]{}; // 1024 is the max for a string in this SL system
			bool changed = false;

			size_t i = 0;
			while (i < sizeof(out))
			{
				const auto value = *string;
				const auto result = static_cast<char>(std::toupper(static_cast<unsigned char>(value)));
				out[i] = result;

				if (value != result)
					changed = true;

				if (result == '\0') // Finished converting string
					break;

				++string;
				++i;
			}

			// Null terminating character was overwritten 
			if (i >= sizeof(out))
			{
				Game::Scr_Error("string too long");
				return;
			}

			if (changed)
			{
				Game::Scr_AddString(out);
			}
			else
			{
				Game::SL_AddRefToString(scriptValue);
				Game::Scr_AddConstString(scriptValue);
				Game::SL_RemoveRefToString(scriptValue);
			}
		});

		Script::AddFunction("GetChar", []
		{
			const auto* str = Game::Scr_GetString(0);
			const auto index = Game::Scr_GetInt(1);

			if (!str)
			{
				Game::Scr_Error("GetChar: Illegal parameter!");
				return;
			}
			
			if (static_cast<std::size_t>(index) >= std::strlen(str))
			{
				Game::Scr_Error("GetChar: char index is out of bounds");
			}

			Game::Scr_AddInt(str[index]);
		});

		// Func present on IW5
		Script::AddFunction("StrICmp", [] // gsc: StrICmp(<string>, <string>)
		{
			const auto value1 = Game::Scr_GetConstString(0);
			const auto value2 = Game::Scr_GetConstString(1);

			const auto result = _stricmp(Game::SL_ConvertToString(value1), Game::SL_ConvertToString(value2));

			Game::Scr_AddInt(result);
		});

		// Func present on IW5
		Script::AddFunction("IsEndStr", [] // gsc: IsEndStr(<string>, <string>)
		{
			const auto* str = Game::Scr_GetString(0);
			const auto* suffix = Game::Scr_GetString(1);

			if (!str || !suffix)
			{
				Game::Scr_Error("IsEndStr: Illegal parameters!");
				return;
			}

			Game::Scr_AddBool(Utils::String::EndsWith(str, suffix));
		});

		// Func present on IW5
		Script::AddFunction("CastFloat", [] // gsc: CastFloat()
		{
			switch (Game::Scr_GetType(0))
			{
			case Game::VAR_STRING:
				Game::Scr_AddFloat(static_cast<float>(std::atof(Game::Scr_GetString(0))));
				break;
			case Game::VAR_FLOAT:
				Game::Scr_AddFloat(Game::Scr_GetFloat(0));
				break;
			case Game::VAR_INTEGER:
				Game::Scr_AddFloat(static_cast<float>(Game::Scr_GetInt(0)));
				break;
			default:
				Game::Scr_ParamError(0, Utils::String::VA("cannot cast %s to float", Game::Scr_GetTypeName(0)));
				break;
			}
		});

		Script::AddFunction("Strtol", [] // gsc: Strtol(<string>, <int>)
		{
			const auto* input = Game::Scr_GetString(0);
			const auto base = Game::Scr_GetInt(1);

			char* end;
			const auto result = std::strtol(input, &end, base);
			if (input == end)
			{
				Game::Scr_ParamError(0, "cannot cast string to int");
			}

			Game::Scr_AddInt(result);
		});
	}

	String::String()
	{
		AddScriptFunctions();
	}
}
