#include <STDInclude.hpp>

namespace Components
{
	const char* VisionFile::DvarExceptions[] =
	{
		"r_pretess",
	};

	void VisionFile::ApplyExemptDvar(const std::string& dvarName, const char* buffer, const std::string& fileName)
	{
		for (std::size_t i = 0; i < ARRAYSIZE(DvarExceptions); ++i)
		{
			if (dvarName == DvarExceptions[i])
			{
				const auto* dvar = Game::Dvar_FindVar(dvarName.data());
				const auto* parsedValue = Game::Com_ParseOnLine(&buffer);

				assert(dvar != nullptr);

				Game::Dvar_SetFromStringFromSource(dvar, parsedValue, Game::DvarSetSource::DVAR_SOURCE_INTERNAL);
				Logger::Print("Overriding '{}' from '{}'\n", dvarName, fileName);

				// Successfully found and tried to apply the string value to the dvar
				return;
			}
		}

		Game::Com_PrintWarning(Game::conChannel_t::CON_CHANNEL_SYSTEM,
			"WARNING: unknown dvar \'%s\' in file \'%s\'\n", dvarName.data(), fileName.data());
	}

	// Gets the dvar name and value and attemps to apply it to the vision settings
	void VisionFile::ApplyValueToSettings(const std::string& key, const char* buffer,
		const std::string& fileName, Game::visionSetVars_t* settings)
	{
		for (std::size_t i = 0; i < 21; ++i)
		{
			// Must be case insensitive comparison
			if (key == Utils::String::ToLower(Game::visionDefFields[i].name))
			{
				auto* const dvarValue = Game::Com_ParseOnLine(&buffer);

				if (!Game::ApplyTokenToField(i, dvarValue, settings))
				{
					Game::Com_PrintWarning(Game::conChannel_t::CON_CHANNEL_SYSTEM,
						"WARNING: malformed dvar \'%s\' in file \'%s\'\n", dvarValue, fileName.data());

					// Failed to apply the value. Check that sscanf can actually parse the value
					return;
				}

				// Successfully found and applied the value to the settings
				return;
			}
		}

		// Dvar not found in visionDefFields, let's try to see if it's a 'patched' dvar
		ApplyExemptDvar(key, buffer, fileName);
	}

	bool VisionFile::LoadVisionSettingsFromBuffer(const char* buffer, const char* fileName, Game::visionSetVars_t* settings)
	{
		assert(settings != nullptr);
		assert(fileName != nullptr);

		Game::Com_BeginParseSession(fileName);

		// Will split the buffer into tokens using the following delimiters: space, newlines (more?)
		for (auto i = Game::Com_Parse(&buffer); *i != '\0'; i = Game::Com_Parse(&buffer))
		{
			// Converting 'key' to lower case as it will be needed later
			ApplyValueToSettings(Utils::String::ToLower(i), buffer, fileName, settings);
			Game::Com_SkipRestOfLine(&buffer);
		}

		Game::Com_EndParseSession();
		return true;
	}

	__declspec(naked) bool VisionFile::LoadVisionSettingsFromBuffer_Stub()
	{
		// No need for push/pop ad guards, I have checked :)
		__asm
		{
			push [esp + 0x8] // settings
			push ebx // filename
			push [esp + 0xC] // buffer
			call VisionFile::LoadVisionSettingsFromBuffer
			add esp, 0xC

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
