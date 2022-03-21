#pragma once

namespace Components
{
	class ScriptExtension : public Component
	{
	public:
		ScriptExtension();

	private:
		static const char* QueryStrings[];

		static void AddFunctions();
		static void AddMethods();
		static void Scr_TableLookupIStringByRow();
	};
}
