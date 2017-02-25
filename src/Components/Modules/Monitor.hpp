#pragma once

namespace Components
{
	class Monitor : public Component
	{
	public:
		Monitor();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "Monitor"; };
#endif

		static bool IsEnabled();

	private:
		static int __stdcall EntryPoint(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
	};
}
