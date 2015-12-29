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

	};
	//Additional Functions for InvokeConnect
	void FindEditHandle(__in_z LPCTSTR lpcszFileName);
	BOOL CALLBACK EnumProc(HWND hWnd, LPARAM lParam);
	BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam);
	HWND FindWindowFromProcessId(DWORD dwProcessId);
	HWND FindWindowFromProcess(HANDLE hProcess);
}