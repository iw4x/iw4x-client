namespace Components
{
	class News : public Component
	{
	public:
		News();
		const char* GetName() { return "News"; };

	private:
		static std::string Motd;
		static const char* GetMotd();
	};
}
