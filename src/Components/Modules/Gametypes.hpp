namespace Components
{
	class Gametypes : public Component
	{
	public:
		Gametypes();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "Gametypes"; };
#endif

	private:
		static unsigned int GetGametypeCount();
		static const char* GetGametypeText(unsigned int index, int column);
		static void SelectGametype(unsigned int index);

		static void* BuildGametypeList(const char* file, void* buffer, size_t size);
	};
}
