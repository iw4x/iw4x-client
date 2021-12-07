#pragma once

namespace Components
{
	class Cheats : public Component
	{
	public:
		Cheats();
		~Cheats();

	private:
		static void AddCheatCommands();
		static void AddScriptFunctions();
	};
}
