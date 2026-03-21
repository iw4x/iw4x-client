#pragma once

namespace Components
{
	class Sound : public Component
	{
	public:
		Sound();

	private:
		static int  Init();
		static void Loop();
	};
}
