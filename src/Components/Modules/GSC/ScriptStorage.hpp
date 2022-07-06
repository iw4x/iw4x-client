#pragma once

namespace Components
{
	class ScriptStorage : public Component
	{
	public:
		ScriptStorage();

	private:
		static std::unordered_map<std::string, std::string> Data;

		static void AddScriptFunctions();
	};
}
