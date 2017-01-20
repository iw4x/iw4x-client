#pragma once

namespace Components
{
	class Singleton : public Component
	{
	public:
		Singleton();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "Singleton"; };
#endif

		static bool IsFirstInstance();

	private:
		static bool FirstInstance;
	};
}
