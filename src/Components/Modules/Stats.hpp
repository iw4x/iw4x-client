#pragma once

namespace Components
{
	class Stats : public Component
	{
	public:
		Stats();
		~Stats();

		static bool IsMaxLevel();

	private:
		static void UpdateClasses(UIScript::Token token);
		static void SendStats();

		static int64_t* GetStatsID();
	};
}
