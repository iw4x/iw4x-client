namespace Components
{
	class Flags : public Component
	{
	public:
		Flags();
		~Flags();
		const char* GetName() { return "Flags"; };

		static bool HasFlag(std::string flag);

	private:
		static std::vector<std::string> EnabledFlags;

		static void ParseFlags();
	};
}
