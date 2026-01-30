#pragma once

namespace Components
{
	class RandomName : public Component
	{
	public:
		RandomName();

	private:
		static void FetchAndSetRandomName();
	};
}
