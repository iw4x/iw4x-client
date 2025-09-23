#pragma once

namespace Components::GSC
{
	class ScriptStorage
	{
	public:
		ScriptStorage();

	private:
		static std::unordered_map<std::string, std::string> Data;

		static void AddScriptFunctions();
	};
}
