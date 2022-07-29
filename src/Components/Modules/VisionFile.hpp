#pragma once

namespace Components
{
	class VisionFile : public Component
	{
	public:
		VisionFile();

	private:
		static std::vector<std::string> DvarExceptions;
		static std::unordered_map<std::string, std::string> VisionReplacements;

		static bool ApplyExemptDvar(const char* dvarName, const char** buffer, const char* filename);

		static bool LoadVisionSettingsFromBuffer(const char* buffer, const char* filename, Game::visionSetVars_t* settings);
		static bool LoadVisionSettingsFromBuffer_Stub();
	};
}
