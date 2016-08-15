namespace Components
{
	class Flags : public Component
	{
	public:
		Flags();
		~Flags();

#ifdef DEBUG
		const char* GetName() { return "Flags"; };
#endif

		static bool HasFlag(std::string flag);

	private:
		static std::vector<std::string> EnabledFlags;

		static void ParseFlags();
	};
}
