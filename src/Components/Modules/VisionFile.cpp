#include <STDInclude.hpp>
#include "VisionFile.hpp"

namespace Components
{
	std::vector<std::string> VisionFile::DvarExceptions =
	{
		"r_pretess",
	};

	std::unordered_map<std::string, std::string> VisionFile::VisionReplacements
	{
		{"511", "r_glow"},
		{"516", "r_glowRadius0"},
		{"512", "r_glowBloomCutoff"},
		{"513", "r_glowBloomDesaturation"},
		{"514", "r_glowBloomIntensity0"},
		{"520", "r_filmEnable"},
		{"522", "r_filmContrast"},
		{"521", "r_filmBrightness"},
		{"523", "r_filmDesaturation"},
		{"524", "r_filmDesaturationDark"},
		{"525", "r_filmInvert"},
		{"526", "r_filmLightTint"},
		{"527", "r_filmMediumTint"},
		{"528", "r_filmDarkTint"},
		{"529", "r_primaryLightUseTweaks"},
		{"530", "r_primaryLightTweakDiffuseStrength"},
		{"531", "r_primaryLightTweakSpecularStrength"},
	};

	bool VisionFile::ApplyExemptDvar(const char* dvarName, const char** buffer, const char* filename)
	{
		for (auto& exceptions : DvarExceptions)
		{
			if (!_stricmp(dvarName, exceptions.data()))
			{
				const auto* dvar = Game::Dvar_FindVar(dvarName);
				const auto* parsedValue = Game::Com_ParseOnLine(buffer);

				assert(dvar);
				assert(parsedValue);

				Game::Dvar_SetFromStringFromSource(dvar, parsedValue, Game::DVAR_SOURCE_INTERNAL);
				Logger::Print("Overriding '{}' from '{}'\n", dvar->name, filename);

				// Successfully found and tried to apply the string value to the dvar
				return true;
			}
		}

		return false;
	}

	bool VisionFile::LoadVisionSettingsFromBuffer(const char* buffer, const char* filename, Game::visionSetVars_t* settings)
	{
		assert(settings);

		bool wasRead[21]{};
		Game::Com_BeginParseSession(filename);

		while (true)
		{
			auto* token = Game::Com_Parse(&buffer);

			if (!*token)
			{
				break;
			}

			auto found = false;
			auto fieldNum = 0;

			const auto it = VisionReplacements.find(token);
			for (fieldNum = 0; fieldNum < 21; ++fieldNum)
			{
				if (!wasRead[fieldNum] && !_stricmp((it == VisionReplacements.end()) ? token : it->second.data(), Game::visionDefFields[fieldNum].name))
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				if (!ApplyExemptDvar(token, &buffer, filename))
				{
					Logger::Warning(Game::CON_CHANNEL_SYSTEM, "WARNING: unknown dvar '{}' in file '{}'\n", token, filename);
					Game::Com_SkipRestOfLine(&buffer);
				}
				continue;
			}

			token = Game::Com_ParseOnLine(&buffer);
			if (ApplyTokenToField(fieldNum, token, settings))
			{
				wasRead[fieldNum] = true;
			}
			else
			{
				Logger::Warning(Game::CON_CHANNEL_SYSTEM, "WARNING: malformed dvar '{}' in file '{}'\n", token, filename);
				Game::Com_SkipRestOfLine(&buffer);
			}
		}

		Game::Com_EndParseSession();
		return true;
	}

	__declspec(naked) bool VisionFile::LoadVisionSettingsFromBuffer_Stub()
	{
		__asm
		{
			push eax
			pushad

			push [esp + 0x24 + 0x8] // settings
			push ebx // filename
			push [esp + 0x24 + 0xC] // buffer
			call LoadVisionSettingsFromBuffer
			add esp, 0xC

			mov [esp + 0x20], eax
			popad
			pop eax

			ret
		}
	}

	VisionFile::VisionFile()
	{
		AssertSize(Game::visField_t, 12);

		// Place hook in LoadVisionFile function
		Utils::Hook(0x59A98A, LoadVisionSettingsFromBuffer_Stub, HOOK_CALL).install()->quick();
	}
}
