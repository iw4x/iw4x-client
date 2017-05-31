#pragma once

namespace Components
{
	class PlayerName : public Component
	{
	public:
		PlayerName();
		~PlayerName();

	private:
		static std::string PlayerNames[18];

		static int GetClientName(int localClientNum, int index, char *buf, int size);
	};
}
