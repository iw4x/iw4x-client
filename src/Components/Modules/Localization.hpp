namespace Components
{
	class Localization : public Component
	{
	public:
		Localization();
		~Localization();
		const char* GetName() { return "Localization"; };

		static void Set(const char* key, const char* value);
		static const char* Get(const char* key);

	private:
		static std::map<std::string, std::string> LocalizeMap;
		static Dvar::Var UseLocalization;
	};
}
