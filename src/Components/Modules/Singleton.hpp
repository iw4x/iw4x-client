namespace Components
{
	class Singleton : public Component
	{
	public:
		Singleton();
		const char* GetName() { return "Singleton"; };

		static bool IsFirstInstance();

	private:
		static bool FirstInstance;
	};
}
