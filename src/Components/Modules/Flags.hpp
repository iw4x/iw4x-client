#pragma once

namespace Components
{
	class Flags : public Component
	{
	public:
		Flags();
		~Flags();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "Flags"; };
#endif

		static bool HasFlag(std::string flag);

	private:
		static std::vector<std::string> EnabledFlags;

		static void ParseFlags();
	};
}
