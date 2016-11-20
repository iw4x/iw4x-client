namespace Components
{
	class PlayerName : public Component
	{
	public:
		PlayerName();
		~PlayerName();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "PlayerName"; };
#endif

	private:
		static std::string PlayerNames[18];

		static int GetClientName(int localClientNum, int index, char *buf, int size);
	};
}
