namespace Components
{
	class News : public Component
	{
	public:
		News();
		~News();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "News"; };
#endif

		bool unitTest();

	private:
		static std::thread Thread;
		static bool Terminate;

		static void CheckForUpdate();
		static void ExitProcessStub(unsigned int exitCode);

		static const char* GetNewsText();
	};
}
