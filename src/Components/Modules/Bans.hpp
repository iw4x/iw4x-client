namespace Components
{
	class Bans : public Component
	{
	public:
		Bans();
		~Bans();

#ifdef DEBUG
		const char* GetName() { return "Bans"; };
#endif

		static void BanClientNum(int num, std::string reason);
	};
}
