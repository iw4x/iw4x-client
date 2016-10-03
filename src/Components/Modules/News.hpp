namespace Components
{
	class News : public Component
	{
	public:
		News();
		~News();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* GetName() { return "News"; };
#endif

		bool UnitTest();

	private:
		static std::thread Thread;
		static bool Terminate;

		static void CheckForUpdate();
		static void ExitProcessStub(unsigned int exitCode);

		static const char* GetNewsText();
	};
}
