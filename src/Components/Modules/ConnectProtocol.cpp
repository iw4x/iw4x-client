#include "..\..\STDInclude.hpp"
#include <psapi.h> 

namespace Components
{
#define MAX_PROCESSES 1024

	int evaluated = 0;
	//Declarations SendMessage stuff
	DWORD proc_id = 0, proc_win_id = 0;
	HWND console, con_in;

	bool ConnectProtocol::InstallProtocol()
	{
		HKEY hKey;
		LPCTSTR sk = TEXT("SOFTWARE\\Classes\\iw4x\\shell\\open\\command");
		LPCTSTR data = "URL:iw4x Protocol";
		LPCTSTR value = TEXT("URL Protocol");
		char ownPth[MAX_PATH] = {0};
		char workdir[MAX_PATH] = {0};
		char regred[MAX_PATH] = {0};
		DWORD dwsize = MAX_PATH;
		HMODULE hModule = GetModuleHandle(NULL);
		if (hModule != NULL)
		{
			
			if (GetModuleFileName(hModule, ownPth, MAX_PATH) == ERROR)
			{
				OutputDebugString("ownPth = Error");
				return false;
			}
			
			if (GetModuleFileName(hModule, workdir, MAX_PATH) == ERROR)
			{
				OutputDebugString("workdir = Error");
				return false;
			}
			else
			{
				char* endPtr = strstr(workdir, "iw4x.exe");
				if (endPtr != NULL)
				{
					*endPtr = 0;
				}
				else
				{
					
					return false;
				}
			}
			//OutputDebugString(Utils::VA("EXE Path: %s", ownPth));
		}
		else
		{
			//OutputDebugString(Utils::VA("Cant get executable path"));
			return false;
		}



		/*OutputDebugString(Utils::VA("EXE Path: %s", ownPth));
		OutputDebugString(Utils::VA("EXE Path2: %s", workdir));*/

		SetCurrentDirectory(workdir);

		LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, sk, 0, KEY_ALL_ACCESS, &hKey);
		if (openRes == ERROR_SUCCESS)
		{
			//Insert Check of the Key Value here, so the protocol will work even if the game was moved.
			openRes = RegQueryValueEx(hKey, 0, 0, 0, LPBYTE(regred), &dwsize);
			if (openRes == ERROR_SUCCESS)
			{
				char* endPt = strstr(regred, "\" \"%1\"");
				if (endPt != NULL)
				{
					*endPt = 0;
				}
				else
				{
					OutputDebugString("endPt = Null");
					return false;
				}


				char* regredPtr = regred;
				regredPtr++;

				////DBG(("Reg Read1: %s", regredPtr));


				RegCloseKey(hKey);
				if (strcmp(regredPtr, ownPth))
				{
					////DBG("Protocol changed, reinstall");
					//sk = TEXT("SOFTWARE\\Classes\\iw4x");
					//openRes = RegOpenKeyEx(HKEY_CURRENT_USER, sk, 0, KEY_ALL_ACCESS, &hKey);
					//if (openRes == ERROR_SUCCESS)
					//{
					//	////DBG("Protocol is corrupted, reinstall");
					//	RegDeleteKey(hKey, 0);
					//	RegCloseKey(hKey);

					//}
					sk = TEXT("SOFTWARE\\Classes\\iw4x");
					openRes = RegDeleteKey(HKEY_CURRENT_USER, sk);
					if (openRes != ERROR_SUCCESS)
					{
						////DBG("Protocol is corrupted, reinstall");
						
						//RegCloseKey(hKey);


					}
				}
				else{
					////DBG("Protocol is already installed");
					return true;
				}
			}
			else{
				//openRes = RegOpenKeyEx(HKEY_CURRENT_USER, sk, 0, KEY_ALL_ACCESS, &hKey);
				//if (openRes == ERROR_SUCCESS)
				//{
				//	////DBG("Protocol is corrupted, reinstall");
				//	RegDeleteKey(hKey, 0);
				//	RegCloseKey(hKey);

				//}
				sk = TEXT("SOFTWARE\\Classes\\iw4x");
				openRes = RegDeleteKey(HKEY_CURRENT_USER, sk);
				if (openRes != ERROR_SUCCESS)
				{
					////DBG("Protocol is corrupted, reinstall");
					
					//RegCloseKey(hKey);


				}

			}

			//////DBG("Protocol is already installed");
			//return true;

		}
		else
		{
			sk = TEXT("SOFTWARE\\Classes\\iw4x");
			openRes = RegDeleteKey(HKEY_CURRENT_USER, sk);
			if (openRes != ERROR_SUCCESS)
			{
				////DBG("Protocol is corrupted, reinstall");
				//return false;
				//RegCloseKey(hKey);


			}
		}

		sk = TEXT("SOFTWARE\\Classes");
		data = "URL:iw4x Protocol";
		value = TEXT("URL Protocol");
		openRes = RegOpenKeyEx(HKEY_CURRENT_USER, sk, 0, KEY_ALL_ACCESS, &hKey);

		if (openRes == ERROR_SUCCESS)
		{
			////DBG("Success opening SOFTWARE\\Classes.");
			sk = TEXT("iw4x");
			openRes = RegCreateKeyEx(hKey, sk, 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hKey, 0);

			if (openRes == ERROR_SUCCESS)
			{
				////DBG("Success creating SOFTWARE\\Classes\\iw3mp.");
				openRes = RegSetValueEx(hKey, value, 0, REG_SZ, (LPBYTE)data, strlen(data) + 1);

				if (openRes == ERROR_SUCCESS)
				{
					////DBG("Success writing URL:iw4x Protocol");
					data = TEXT("");
					openRes = RegSetValueEx(hKey, value, 0, REG_SZ, (LPBYTE)data, strlen(data) + 1);

					if (openRes == ERROR_SUCCESS)
					{
						////DBG("Success writing URL Protocol");
						sk = TEXT("DefaultIcon");
						openRes = RegCreateKeyEx(hKey, sk, 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hKey, 0);

						if (openRes == ERROR_SUCCESS)
						{
							////DBG("Success creating SOFTWARE\\Classes\\iw3mp\\DefaultIcon");
							data = Utils::VA("%s,1", ownPth);
							openRes = RegSetValueEx(hKey, 0, 0, REG_SZ, (LPBYTE)data, strlen(data) + 1);

							if (openRes == ERROR_SUCCESS)
							{
								openRes = RegCloseKey(hKey);

								if (openRes == ERROR_SUCCESS)
								{
									sk = TEXT("SOFTWARE\\Classes\\iw4x");
									openRes = RegOpenKeyEx(HKEY_CURRENT_USER, sk, 0, KEY_ALL_ACCESS, &hKey);

									if (openRes == ERROR_SUCCESS)
									{
										sk = TEXT("shell\\open\\command");
										openRes = RegCreateKeyEx(hKey, sk, 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hKey, 0);

										if (openRes == ERROR_SUCCESS)
										{
											data = Utils::VA("\"%s\" \"%s\"", ownPth, "%1");
											////DBG(("Command is %s", data));
											openRes = RegSetValueEx(hKey, 0, 0, REG_SZ, (LPBYTE)data, strlen(data) + 1);

											if (openRes == ERROR_SUCCESS)
											{
												RegCloseKey(hKey);

											}
											else
											{
												////DBG("Error writing shell command to registry");
												RegCloseKey(hKey);
												return false;
											}
										}
										else
										{
											////DBG("Error creating Key shell\\open\\command");
											//RegCloseKey(hKey);
											return false;
										}
									}
									else
									{
										////DBG("Error opening SOFTWARE\\Classes\\iw3mp");
										//RegCloseKey(hKey);
										return false;
									}
								}
								else
								{
									////DBG("Error closing DefaultIcon Key");
									RegCloseKey(hKey);
									return false;
								}
							}
							else
							{
								////DBG("Error writing EXE Path,1 to DefaultIcon Key");
								RegCloseKey(hKey);
								return false;
							}
						}
						else
						{
							////DBG("Error creating subkey DefaultIcon");
							//RegCloseKey(hKey);
							return false;
						}
					}
					else
					{
						////DBG("Error writing URL Protocol");
						RegCloseKey(hKey);
						return false;
					}
				}
				else
				{
					////DBG("Error writing URL:iw4x Protocol Code: %d", openRes);
					RegCloseKey(hKey);
					return false;
				}
			}
			else
			{
				////DBG("Error creating key SOFTWARE\\Classes\\iw3mp");
				//RegCloseKey(hKey);
				return false;
			}
		}
		else
		{
			////DBG("Error opening key.");
			return false;
		}

		return true;
	}

	void ConnectProtocol::EvaluateProtocol()
	{
		if (evaluated) return;
		evaluated = 1;
		OutputDebugString("Evaluated = 1");
		char* args = GetCommandLine();
		OutputDebugString("GetCommandLine");
		char* substr = strstr(args, "iw4x://");

		if (!substr || substr == args)
		{
			OutputDebugString("substr==args");
			OutputDebugString(substr);
			OutputDebugString("substr==args");
			OutputDebugString(args);
			return;
		}


		substr += 7;
		char* substr2 = strstr(substr, "/");
		if (substr2 != NULL)
		{
			*substr2 = 0;
		}
		else
		{
			OutputDebugString("substr2 = NULL");
			return;
		}

		////DBG(("Connecting to: %s", substr));
		OutputDebugString(Utils::VA("connect %s", substr));
		
		Command::Execute(Utils::VA("connect %s;", substr), true);
	}

	BOOL ConnectProtocol::InvokeConnect()
	{

		char* args = GetCommandLineA();
		char* substr = strstr(args, "iw4x://");

		////DBG("Mutex give us a Handle = %d", mutex);
		////DBG("Last Error = %d", GetLastError());
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			if (!substr || substr == args)
			{
				// Not started using the protocol
				return FALSE;
			}

			//Now we have to get the Window Handle of the Console and the Handle of the Input field
			////DBG("The Game is already running");
			FindEditHandle("iw4x.exe");

			substr += 7;
			char* substr2 = strstr(substr, "/");
			if (substr2 != NULL)
			{
				*substr2 = 0;
			}
			else
			{
				return false;
			}

			if (proc_id != 0)
			{

				if (con_in != NULL)
				{
					SendMessageA(con_in, WM_SETTEXT, NULL, (LPARAM)Utils::VA("connect %s;", substr));
					SendMessageA(con_in, WM_CHAR, 0xD, 0);
				}

			}
			else
			{
				return FALSE;
				////DBG("Did not find Process by Name iw4x");
			}

			////DBG("Exit this Instance");
			return TRUE;

		}
		else
		{
			if (substr && substr != args)
			{
				// Skip intro
				*(BYTE*)0x60BECF = 0xEB;
				return FALSE;
			}
			return FALSE;
		}
		return FALSE;
	}



	ConnectProtocol::ConnectProtocol()
	{
		OutputDebugString("Installing Protocol");
		ConnectProtocol::InstallProtocol();
		ConnectProtocol::EvaluateProtocol();
	}


	//Send Connect Command to running iw4x instance
	BOOL CALLBACK ConnectProtocol::EnumWindowsProc(__in  HWND hWnd, __in  LPARAM lParam) {

		DWORD id = GetWindowThreadProcessId(hWnd, &id);
		DWORD id2 = GetWindowThreadProcessId(FindWindowFromProcessId(proc_id), &id2);
		//DBG("process id: %d %d Real ID: %d", id, id2, proc_id);
		if (id == id2)
		{
			//printf("THEY MATCH!process id: %d\n", id);
			char buffer[256];
			GetWindowText(hWnd, (LPSTR)buffer, 255);
			char* endPtr = strstr(buffer, "Console");
			if (endPtr != 0)
			{
				//DBG(("Got Process Window Handle: %d, Window Text = %s", hWnd, buffer));
				console = hWnd;
				EnumChildWindows(console, EnumChildProc, NULL);
				return FALSE;
			}

		}
		else{
			//DBG(("Got no Process Window Handle!!!!!!!!!!!!"));
		}

		return TRUE;
	}

	BOOL CALLBACK ConnectProtocol::EnumChildProc(HWND hwnd, LPARAM lParam) {
		char buffer[256];

		GetClassName(hwnd, (LPSTR)buffer, 255);
		char* endPtr = strstr(buffer, "Edit");
		if (endPtr != 0)
		{
			con_in = hwnd;
			//DBG("Got Handle of Edit Field: %d", hwnd);
			return FALSE;
		}


		return TRUE;

	}

	struct EnumData {
		DWORD dwProcessId;
		HWND hWnd;
	};

	BOOL CALLBACK ConnectProtocol::EnumProc(HWND hWnd, LPARAM lParam) {
		// Retrieve storage location for communication data
		EnumData& ed = *(EnumData*)lParam;
		DWORD dwProcessId = 0x0;
		// Query process ID for hWnd
		GetWindowThreadProcessId(hWnd, &dwProcessId);
		// Apply filter - if you want to implement additional restrictions,
		// this is the place to do so.
		if (ed.dwProcessId == dwProcessId) {
			// Found a window matching the process ID
			ed.hWnd = hWnd;
			// Report success
			SetLastError(ERROR_SUCCESS);
			// Stop enumeration
			return FALSE;
		}
		// Continue enumeration
		return TRUE;
	}

	void ConnectProtocol::FindEditHandle(__in_z LPCTSTR lpcszFileName)
	{
		LPDWORD lpdwProcessIds;
		LPTSTR  lpszBaseName;
		HANDLE  hProcess;
		DWORD   i, cdwProcesses, dwProcessId = 0;

		lpdwProcessIds = (LPDWORD)HeapAlloc(GetProcessHeap(), 0, MAX_PROCESSES*sizeof(DWORD));
		if (lpdwProcessIds != NULL)
		{
			if (EnumProcesses(lpdwProcessIds, MAX_PROCESSES*sizeof(DWORD), &cdwProcesses))
			{
				lpszBaseName = (LPTSTR)HeapAlloc(GetProcessHeap(), 0, MAX_PATH*sizeof(TCHAR));
				if (lpszBaseName != NULL)
				{
					cdwProcesses /= sizeof(DWORD);
					for (i = 0; i < cdwProcesses; i++)
					{
						hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, lpdwProcessIds[i]);
						if (hProcess != NULL)
						{
							if (GetModuleBaseName(hProcess, NULL, lpszBaseName, MAX_PATH) > 0)
							{
								//DBG(("Process Name = %s", lpszBaseName));
								if (!lstrcmpi(lpszBaseName, lpcszFileName))
								{
									//DBG("Stage 8");
									dwProcessId = lpdwProcessIds[i];
									CloseHandle(hProcess);
									proc_id = dwProcessId;
									EnumWindows(EnumWindowsProc, NULL);
									if (con_in != NULL)
									{
										break;
									}

								}
							}
							CloseHandle(hProcess);
						}
					}
					HeapFree(GetProcessHeap(), 0, (LPVOID)lpszBaseName);
				}
			}
			HeapFree(GetProcessHeap(), 0, (LPVOID)lpdwProcessIds);
		}
		//DBG("Return %d", dwProcessId);
		//return dwProcessId; 
	}
	// Main entry
	HWND ConnectProtocol::FindWindowFromProcessId(DWORD dwProcessId) {
		EnumData ed = { dwProcessId };
		if (!EnumWindows(EnumProc, (LPARAM)&ed) &&
			(GetLastError() == ERROR_SUCCESS)) {
			return ed.hWnd;
		}
		return NULL;
	}
	// Helper method for convenience
	HWND ConnectProtocol::FindWindowFromProcess(HANDLE hProcess) {
		return FindWindowFromProcessId(GetProcessId(hProcess));
	}
}
