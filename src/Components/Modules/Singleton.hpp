namespace Components
{
	class Singleton : public Component
	{
	public:
		Singleton();

#ifdef DEBUG
		const char* GetName() { return "Singleton"; };
#endif

		static bool IsFirstInstance();

	private:
		static bool FirstInstance;
	};
}
