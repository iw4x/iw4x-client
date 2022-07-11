#pragma once

namespace Components
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
