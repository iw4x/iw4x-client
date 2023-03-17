#pragma once

namespace Components
{
	class Discord : public Component
	{
	public:
		Discord();

		void preDestroy() override;

	private:
		static bool Initialized_;

		static void UpdateDiscord();
	};
}
