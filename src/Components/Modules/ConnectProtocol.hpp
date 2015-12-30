namespace Components
{
	class ConnectProtocol : public Component
	{
	public:
		ConnectProtocol();
		void EvaluateProtocol();
		static BOOL InvokeConnect();

	private:
		static bool InstallProtocol();

		//Additional Functions for InvokeConnect
		static void FindEditHandle(__in_z LPCTSTR lpcszFileName);
		static BOOL CALLBACK EnumProc(HWND hWnd, LPARAM lParam);
		static BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam);
		static BOOL CALLBACK EnumWindowsProc(__in  HWND hWnd, __in  LPARAM lParam);
		static HWND FindWindowFromProcessId(DWORD dwProcessId);
		static HWND FindWindowFromProcess(HANDLE hProcess);
	};
}