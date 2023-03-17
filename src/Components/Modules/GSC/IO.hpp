#pragma once

namespace Components::GSC
{
	class IO : public Component
	{
	public:
		IO();

	private:
		static const char* QueryStrings[];

		static void AddScriptFunctions();
	};
}
