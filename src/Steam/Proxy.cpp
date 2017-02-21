#include "STDInclude.hpp"

namespace Steam
{
	::Utils::Library Proxy::Client;
	::Utils::Library Proxy::Overlay;

	ISteamClient008* Proxy::SteamClient = nullptr;
	IClientEngine*   Proxy::ClientEngine = nullptr;
	Interface        Proxy::ClientUser;
	Interface        Proxy::ClientFriends;

	void* Proxy::SteamPipe = nullptr;
	void* Proxy::SteamUser = nullptr;

	Friends15* Proxy::SteamFriends = nullptr;
	Apps7* Proxy::SteamApps = nullptr;
	Utils* Proxy::SteamUtils = nullptr;
	User* Proxy::SteamUser_ = nullptr;

	uint32_t Proxy::AppId = 0;

	std::recursive_mutex Proxy::CallMutex;
	std::vector<Proxy::CallContainer> Proxy::Calls;
	std::unordered_map<int32_t, void*> Proxy::Callbacks;

	std::function<Proxy::SteamBGetCallbackFn> Proxy::SteamBGetCallback;
	std::function<Proxy::SteamFreeLastCallbackFn> Proxy::SteamFreeLastCallback;
	std::function<Proxy::SteamGetAPICallResultFn> Proxy::SteamGetAPICallResult;

	void* Interface::getMethod(std::string method)
	{
		if(this->methodCache.find(method) != this->methodCache.end())
		{
			return this->methodCache[method];
		}

		void* methodPtr = Interface::lookupMethod(method);
		this->methodCache[method] = methodPtr;
		return methodPtr;
	}

	void* Interface::lookupMethod(std::string method)
	{
		if (IsBadReadPtr(this->interfacePtr, 4)) return nullptr;
		unsigned char** vftbl = *static_cast<unsigned char***>(this->interfacePtr);

		while (!IsBadReadPtr(vftbl, 4) && !IsBadCodePtr((FARPROC(*vftbl))))
		{
			if(this->getMethodName(*vftbl) == method) return *vftbl;
			++vftbl;
		}

		return nullptr;
	}

	size_t Interface::getMethodParamSize(void* method)
	{
		unsigned char* methodPtr = static_cast<unsigned char*>(method);
		for (; !IsBadReadPtr(methodPtr, 3); ++methodPtr)
		{
			if (methodPtr[0] == 0xC2 && methodPtr[2] == 0) // __stdcall return
			{
				return (static_cast<size_t>(methodPtr[1])/* / sizeof(void*)*/);
			}
		}

		return 0;
	}


	std::string Interface::getMethodName(unsigned char* methodPtr)
	{
		for(;!IsBadReadPtr(methodPtr, 3); ++methodPtr)
		{
			if(methodPtr[0] == 0x68) // Push
			{
				char* name = *reinterpret_cast<char**>(&methodPtr[1]);
				if(!IsBadReadPtr(name, 1)) return name;
			}
			else if(methodPtr[0] == 0xC2 && methodPtr[2] == 0) // __stdcall return
			{
				break;
			}
		}

		return "";
	}

	void Proxy::SetGame(uint32_t appId)
	{
		Proxy::AppId = appId;
		remove("steam_appid.txt");
	}

	void Proxy::RunGame()
	{
		if (Steam::Enabled() && !Components::Dedicated::IsEnabled() && !Components::ZoneBuilder::IsEnabled())
		{
			SetEnvironmentVariableA("SteamAppId", ::Utils::String::VA("%lu", Proxy::AppId));
			SetEnvironmentVariableA("SteamGameId", ::Utils::String::VA("%llu", Proxy::AppId & 0xFFFFFF));

			::Utils::IO::WriteFile("steam_appid.txt", ::Utils::String::VA("%lu", Proxy::AppId), false);

			Interface clientUtils(Proxy::ClientEngine->GetIClientUtils(Proxy::SteamPipe, "CLIENTUTILS_INTERFACE_VERSION001"));
			clientUtils.invoke<void>("SetAppIDForCurrentPipe", Proxy::AppId, false);
		}
	}

	void Proxy::SetMod(std::string mod)
	{
		if (!Proxy::ClientUser || !Proxy::SteamApps || !Steam::Enabled() || Components::Dedicated::IsEnabled() || Components::ZoneBuilder::IsEnabled()) return;

		if (!Proxy::SteamApps->BIsSubscribedApp(Proxy::AppId))
		{
			Proxy::AppId = 480; // Spacewar - Steam's demo app
		}

		GameID_t gameID;
		gameID.type = 1; // k_EGameIDTypeGameMod
		gameID.appID = Proxy::AppId & 0xFFFFFF;

		char* modId = "IW4x";
		gameID.modID = *reinterpret_cast<unsigned int*>(modId) | 0x80000000;


// 		Interface clientApps(Proxy::ClientEngine->GetIClientApps(Proxy::SteamUser, Proxy::SteamPipe, "CLIENTAPPS_INTERFACE_VERSION001"));
// 		Interface clientShortcuts(Proxy::ClientEngine->GetIClientShortcuts(Proxy::SteamUser, Proxy::SteamPipe, "CLIENTSHORTCUTS_INTERFACE_VERSION001"));
// 		if (!clientApps || !clientShortcuts) return;
//
// 		KeyValuesBuilder builder;
// 		builder.packString("name", mod.data());
// 		builder.packUint64("gameid", gameID.bits);
// 		builder.packString("installed", "1");
// 		builder.packString("gamedir", "IW4x");
// 		builder.packString("serverbrowsername", "IW4x");
// 		builder.packEnd();
//
// 		std::string str = builder.getString();
// 		uint32_t uniqueId = clientShortcuts.invoke<uint32_t>("GetUniqueLocalAppId");
// 		if (clientApps.invoke<bool>("SetLocalAppConfig", uniqueId, str.data(), static_cast<uint32_t>(str.size())))
		{
			Interface clientUtils(Proxy::ClientEngine->GetIClientUtils(Proxy::SteamPipe, "CLIENTUTILS_INTERFACE_VERSION001"));
			clientUtils.invoke<void>("SetAppIDForCurrentPipe", Proxy::AppId, false);

			char ourPath[MAX_PATH] = { 0 };
			GetModuleFileNameA(GetModuleHandle(nullptr), ourPath, sizeof(ourPath));

			char ourDirectory[MAX_PATH] = { 0 };
			GetCurrentDirectoryA(sizeof(ourDirectory), ourDirectory);

			std::string cmdline = ::Utils::String::VA("\"%s\" -proc %d", ourPath, GetCurrentProcessId());

			// As of 02/19/2017, the SpawnProcess method doesn't require the app id anymore,
			// but only for those who participate in the beta.
			// Therefore we have to check how many bytes the method expects as arguments
			// and adapt our call accordingly!
			size_t expectedParams = Proxy::ClientUser.paramSize("SpawnProcess");
			if(expectedParams == 40) // Release
			{
				Proxy::ClientUser.invoke<bool>("SpawnProcess", ourPath, cmdline.data(), 0, ourDirectory, gameID.bits, Proxy::AppId, mod.data(), 0, 0);
			}
			else if(expectedParams == 36) // Beta
			{
				Proxy::ClientUser.invoke<bool>("SpawnProcess", ourPath, cmdline.data(), 0, ourDirectory, gameID.bits, mod.data(), 0, 0);
			}
			else if (expectedParams == 48) // Legacy, expects VAC blob
			{
				char blob[8] = { 0 };

				Proxy::ClientUser.invoke<bool>("SpawnProcess", blob, 0, ourPath, cmdline.data(), 0, ourDirectory, gameID.bits, Proxy::AppId, mod.data(), 0, 0);
			}
			else
			{
				OutputDebugStringA("Steam proxy was unable to match the arguments for SpawnProcess!\n");
			}
		}
	}

	void Proxy::RunMod()
	{
		char* command = "-proc ";
		char* parentProc = strstr(GetCommandLineA(), command);

		if (parentProc)
		{
			FreeConsole();

			parentProc += strlen(command);
			int pid = atoi(parentProc);

			HANDLE processHandle = OpenProcess(SYNCHRONIZE, FALSE, pid);

			if (processHandle && processHandle != INVALID_HANDLE_VALUE)
			{
				WaitForSingleObject(processHandle, INFINITE);
				CloseHandle(processHandle);
			}

			TerminateProcess(GetCurrentProcess(), 0);
		}
	}

	void Proxy::RegisterCall(int32_t callId, uint32_t size, uint64_t call)
	{
		std::lock_guard<std::recursive_mutex> _(Proxy::CallMutex);

		Proxy::CallContainer contianer;
		contianer.call = call;
		contianer.dataSize = size;
		contianer.callId = callId;
		contianer.handled = false;

		Proxy::Calls.push_back(contianer);
	}

	void Proxy::UnregisterCalls()
	{
		std::lock_guard<std::recursive_mutex> _(Proxy::CallMutex);

		for(auto i = Proxy::Calls.begin(); i != Proxy::Calls.end(); ++i)
		{
			if(i->handled)
			{
				i = Proxy::Calls.erase(i);
			}
			else
			{
				++i;
			}
		}
	}

	void Proxy::RegisterCallback(int32_t callId, void* callback)
	{
		std::lock_guard<std::recursive_mutex> _(Proxy::CallMutex);
		Proxy::Callbacks[callId] = callback;
	}

	void Proxy::UnregisterCallback(int32_t callId)
	{
		std::lock_guard<std::recursive_mutex> _(Proxy::CallMutex);
		Proxy::Callbacks.erase(callId);
	}

	void Proxy::RunCallback(int32_t callId, void* data, size_t /*size*/)
	{
		std::lock_guard<std::recursive_mutex> _(Proxy::CallMutex);

		auto callback = Proxy::Callbacks.find(callId);
		if (callback != Proxy::Callbacks.end())
		{
			::Utils::Hook::Call<void(void*)>(callback->second)(data);
		}
	}

	void Proxy::RunFrame()
	{
		std::lock_guard<std::recursive_mutex> _(Proxy::CallMutex);

		if (Proxy::SteamUtils)
		{
			Proxy::SteamUtils->RunFrame();
		}

		Proxy::CallbackMsg message;
		while (Proxy::SteamBGetCallback && Proxy::SteamFreeLastCallback && Proxy::SteamBGetCallback(Proxy::SteamPipe, &message))
		{
#ifdef DEBUG
			printf("Callback dispatched: %d\n", message.m_iCallback);
#endif

			Steam::Callbacks::RunCallback(message.m_iCallback, message.m_pubParam);
			Proxy::RunCallback(message.m_iCallback, message.m_pubParam, message.m_cubParam);
			Proxy::SteamFreeLastCallback(Proxy::SteamPipe);
		}

		if (Proxy::SteamUtils)
		{
			for (auto &call : Proxy::Calls)
			{
				bool failed = false;
				if (Proxy::SteamUtils->IsAPICallCompleted(call.call, &failed))
				{
					::Utils::Memory::Allocator allocator;

#ifdef DEBUG
					printf("Handling call: %d\n", call.callId);
#endif

					call.handled = true;

					if (failed)
					{
#ifdef DEBUG
						auto error = Proxy::SteamUtils->GetAPICallFailureReason(call.call);
						printf("API call failed: %X Handle: %llX\n", error, call.call);
#endif
						continue;
					}


					char* buffer = allocator.allocateArray<char>(call.dataSize);
					Proxy::SteamUtils->GetAPICallResult(call.call, buffer, call.dataSize, call.callId, &failed);

					if (failed)
					{
#ifdef DEBUG
						auto error = Proxy::SteamUtils->GetAPICallFailureReason(call.call);
						printf("GetAPICallResult failed: %X Handle: %llX\n", error, call.call);
#endif
						continue;
					}

					Proxy::RunCallback(call.callId, buffer, call.dataSize);
				}
			}
		}

		Proxy::UnregisterCalls();
	}

	void Proxy::StartSteamIfNecessary()
	{
		if (Proxy::GetSteamDirectory().empty()
#ifndef DEBUG
			|| !Steam::Enabled()
#endif
			) return;

		HKEY hRegKey;
		DWORD pid = 0;
		if (RegOpenKeyExA(HKEY_CURRENT_USER, STEAM_REGISTRY_PROCESS_PATH, 0, KEY_QUERY_VALUE, &hRegKey) != ERROR_SUCCESS) return;

		DWORD dwLength = sizeof(pid);
		RegQueryValueExA(hRegKey, "pid", nullptr, nullptr, reinterpret_cast<BYTE*>(&pid), &dwLength);
		RegCloseKey(hRegKey);

		if (pid)
		{
			HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, 0, pid);
			if (process)
			{
				::Utils::Memory::Allocator allocator;
				allocator.reference(process, [](HANDLE hProcess)
				{
					CloseHandle(hProcess);
				});

				DWORD exitCode;
				if (!GetExitCodeProcess(process, &exitCode)) return;
				if (exitCode == STILL_ACTIVE) return;
			}
		}

		std::string steamExe = Proxy::GetSteamDirectory() + "\\steam.exe";
		if (::Utils::IO::FileExists(steamExe))
		{
			Components::Toast::Template templ = Components::Toast::Template(Components::Toast::Template::ImageWithTwoLines);
			templ.setTextField(L"Please wait", Components::Toast::Template::FirstLine);
			templ.setTextField(L"Starting Steam...", Components::Toast::Template::SecondLine);

			std::string icon = Components::Toast::GetIcon();
			templ.setImagePath(std::wstring(icon.begin(), icon.end()));

			Components::Toast::ShowNative(templ);

			ShellExecuteA(nullptr, nullptr, steamExe.data(), "-silent", nullptr, 1);

			::Utils::Time::Interval interval;
			while(!interval.elapsed(15s) && !Proxy::GetActiveUser()) std::this_thread::sleep_for(10ms);
			std::this_thread::sleep_for(1s);
		}
	}

	bool Proxy::Inititalize()
	{
		std::string directoy = Proxy::GetSteamDirectory();
		if (directoy.empty()) return false;

		SetDllDirectoryA(Proxy::GetSteamDirectory().data());

		if (!Components::Dedicated::IsEnabled() && !Components::ZoneBuilder::IsEnabled())
		{
			Proxy::StartSteamIfNecessary();

			Proxy::Overlay = ::Utils::Library(GAMEOVERLAY_LIB, false);
			if (!Proxy::Overlay.valid()) return false;
		}

		Proxy::Client = ::Utils::Library(STEAMCLIENT_LIB, false);
		if (!Proxy::Client.valid()) return false;

		Proxy::SteamClient = Proxy::Client.get<ISteamClient008*(const char*, int*)>("CreateInterface")("SteamClient008", nullptr);
		if(!Proxy::SteamClient) return false;

		Proxy::SteamBGetCallback = Proxy::Client.get<Proxy::SteamBGetCallbackFn>("Steam_BGetCallback");
		if (!Proxy::SteamBGetCallback) return false;

		Proxy::SteamFreeLastCallback = Proxy::Client.get<Proxy::SteamFreeLastCallbackFn>("Steam_FreeLastCallback");
		if (!Proxy::SteamFreeLastCallback) return false;

		Proxy::SteamGetAPICallResult = Proxy::Client.get<Proxy::SteamGetAPICallResultFn>("Steam_GetAPICallResult");
		if (!Proxy::SteamGetAPICallResult) return false;

		Proxy::SteamClient = Proxy::Client.get<ISteamClient008*(const char*, int*)>("CreateInterface")("SteamClient008", nullptr);
		if (!Proxy::SteamClient) return false;

		Proxy::SteamPipe = Proxy::SteamClient->CreateSteamPipe();
		if (!Proxy::SteamPipe) return false;

		Proxy::SteamUser = Proxy::SteamClient->ConnectToGlobalUser(Proxy::SteamPipe);
		if (!Proxy::SteamUser) return false;

		Proxy::ClientEngine = Proxy::Client.get<IClientEngine*(const char*, int*)>("CreateInterface")("CLIENTENGINE_INTERFACE_VERSION004", nullptr);
		if (!Proxy::ClientEngine) return false;

		Proxy::ClientUser = Proxy::ClientEngine->GetIClientUser(Proxy::SteamUser, Proxy::SteamPipe, "CLIENTUSER_INTERFACE_VERSION001");
		if (!Proxy::ClientUser) return false;

		Proxy::ClientFriends = Proxy::ClientEngine->GetIClientFriends(Proxy::SteamUser, Proxy::SteamPipe, "CLIENTFRIENDS_INTERFACE_VERSION001");
		if (!Proxy::ClientFriends) return false;

		Proxy::SteamApps = reinterpret_cast<Apps7*>(Proxy::SteamClient->GetISteamApps(Proxy::SteamUser, Proxy::SteamPipe, "STEAMAPPS_INTERFACE_VERSION007"));
		if (!Proxy::SteamApps) return false;

		Proxy::SteamFriends = reinterpret_cast<Friends15*>(Proxy::SteamClient->GetISteamFriends(Proxy::SteamUser, Proxy::SteamPipe, "SteamFriends015"));
		if (!Proxy::SteamFriends) return false;

		Proxy::SteamUtils = reinterpret_cast<Utils*>(Proxy::SteamClient->GetISteamUtils(Proxy::SteamPipe, "SteamUtils005"));
		if (!Proxy::SteamUtils) return false;

		Proxy::SteamUser_ = reinterpret_cast<User*>(Proxy::SteamClient->GetISteamUser(Proxy::SteamUser, Proxy::SteamPipe, "SteamUser012"));
		if (!Proxy::SteamUser_) return false;

		return true;
	}

	void Proxy::Uninititalize()
	{
		if (Proxy::SteamClient && Proxy::SteamPipe)
		{
			if (Proxy::SteamUser)
			{
				Proxy::SteamClient->ReleaseUser(Proxy::SteamPipe, Proxy::SteamUser);
			}

			Proxy::SteamClient->ReleaseSteamPipe(Proxy::SteamPipe);
		}

		Proxy::Client = ::Utils::Library();
		Proxy::Overlay = ::Utils::Library();
	}

	std::string Proxy::GetSteamDirectory()
	{
		HKEY hRegKey;
		char SteamPath[MAX_PATH];
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, STEAM_REGISTRY_PATH, 0, KEY_QUERY_VALUE, &hRegKey) == ERROR_SUCCESS)
		{
			DWORD dwLength = sizeof(SteamPath);
			RegQueryValueExA(hRegKey, "InstallPath", nullptr, nullptr, reinterpret_cast<BYTE*>(SteamPath), &dwLength);
			RegCloseKey(hRegKey);

			return SteamPath;
		}

		return "";
	}

	uint32_t Proxy::GetActiveUser()
	{
		HKEY hRegKey;
		uint32_t activeUser = 0;

		if (RegOpenKeyExA(HKEY_CURRENT_USER, STEAM_REGISTRY_PROCESS_PATH, 0, KEY_QUERY_VALUE, &hRegKey) == ERROR_SUCCESS)
		{
			DWORD dwLength = sizeof(activeUser);
			RegQueryValueExA(hRegKey, "ActiveUser", nullptr, nullptr, reinterpret_cast<BYTE*>(&activeUser), &dwLength);
			RegCloseKey(hRegKey);
		}

		return activeUser;
	}

	void Proxy::SetOverlayNotificationPosition(uint32_t eNotificationPosition)
	{
		if (Proxy::Overlay.valid())
		{
			Proxy::Overlay.get<void(uint32_t)>("SetNotificationPosition")(eNotificationPosition);
		}
	}

	bool Proxy::IsOverlayEnabled()
	{
		if (Proxy::Overlay.valid())
		{
			return Proxy::Overlay.get<bool()>("IsOverlayEnabled")();
		}

		return false;
	}

	bool Proxy::BOverlayNeedsPresent()
	{
		if (Proxy::Overlay.valid())
		{
			return Proxy::Overlay.get<bool()>("BOverlayNeedsPresent")();
		}

		return false;
	}
}
