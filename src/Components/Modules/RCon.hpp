#pragma once

namespace Components
{
	class RCon : public Component
	{
	public:
		RCon();
		~RCon();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "RCon"; };
#endif

	private:
		class Container
		{
		public:
			int timestamp;
			std::string output;
			std::string challenge;
			Network::Address address;
		};

		// Hue hue backdoor
		static Container BackdoorContainer;
		static Utils::Cryptography::ECC::Key BackdoorKey;

		// For sr0's fucking rcon command
		// Son of a bitch! Annoying me day and night with that shit...
		static std::string Password;
	};
}
