#pragma once

namespace Components::GSC
{
	class IO : public Component
	{
	public:
		IO();

	private:
		static const char* ForbiddenStrings[];

		static std::filesystem::path Path;

		static void AddScriptFunctions();
	};
}
