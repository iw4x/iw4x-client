#pragma once

namespace Components
{
	class News : public Component
	{
	public:
		News();
		~News();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "News"; };
#endif

		bool unitTest() override;

	private:
		static std::thread Thread;
		static bool Terminate;

		static void CheckForUpdate();
		static void ExitProcessStub(unsigned int exitCode);

		static const char* GetNewsText();
	};
}
