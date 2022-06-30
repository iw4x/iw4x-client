#pragma once

namespace Components
{
	class VisionFile : public Component
	{
	public:
		VisionFile();

	private:
		static const char* DvarExceptions[];

		static void ApplyExemptDvar(const std::string& dvarName, const char* buffer, const std::string& fileName);
		static void ApplyValueToSettings(const std::string& dvarName, const char* buffer, const std::string& fileName, Game::visionSetVars_t* settings);

		static bool LoadVisionSettingsFromBuffer(const char* buffer, const char* fileName, Game::visionSetVars_t* settings);
		static bool LoadVisionSettingsFromBuffer_Stub();
	};
}
