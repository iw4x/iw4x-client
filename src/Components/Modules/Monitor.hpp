#pragma once

namespace Components
{
	class Monitor : public Component
	{
	public:
		Monitor();

		static bool IsEnabled();

	private:
		static int __stdcall EntryPoint(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
	};
}
