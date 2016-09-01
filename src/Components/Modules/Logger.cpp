#include "STDInclude.hpp"

namespace Components
{
	std::mutex Logger::MessageMutex;
	std::vector<std::string> Logger::MessageQueue;
	std::vector<Game::netadr_t> Logger::addresses;
	std::vector<Game::netadr_t> Logger::gaddresses;

	static char* writeFile;
	static char* writeFolder;
	//static DWORD fsBuildOSPathForThreadHookLoc = 0x642139;
	DWORD fsBuildOSPathForThreadHookLocRet = 0x64213F;
	Game::dvar_t* iw4m_onelog;
	

	void(*Logger::PipeCallback)(std::string) = nullptr;

	bool Logger::IsConsoleReady()
	{
		return (IsWindow(*reinterpret_cast<HWND*>(0x64A3288)) != FALSE || (Dedicated::IsEnabled() && !Flags::HasFlag("console")));
	}

	void Logger::Print(const char* message, ...)
	{
		return Logger::MessagePrint(0, Logger::Format(&message));
	}

	void Logger::Print(int channel, const char* message, ...)
	{
		return Logger::MessagePrint(channel, Logger::Format(&message));
	}

	void Logger::MessagePrint(int channel, std::string message)
	{
		if (Flags::HasFlag("stdout") || Loader::PerformingUnitTests())
		{
			printf("%s", message.data());
			/*if (Logger::addresses.size()) {
				for (size_t i = 0; i < Logger::addresses.size(); i++) {
					const char* toSend = Utils::String::VA("%i %s", channel, message);
					Network::Send(Logger::addresses[i], toSend);
				}
			}*/
			fflush(stdout);
			
			return;
		}

		if (!Logger::IsConsoleReady())
		{
			OutputDebugStringA(message.data());
			/*if (Logger::addresses.size()) {
				for (size_t i = 0; i < Logger::addresses.size(); i++) {
					const char* toSend = Utils::String::VA("%i %s", channel, message);
					Network::Send(Logger::addresses[i], toSend);
				}
			}*/
		}

		if (!Game::Sys_IsMainThread())
		{
			Logger::EnqueueMessage(message);
			/*if (Logger::addresses.size()) {
				for (size_t i = 0; i < Logger::addresses.size(); i++) {
					const char* toSend = Utils::String::VA("%i %s", channel, message);
					Network::Send(Logger::addresses[i], toSend);
				}
			}*/
		}
		else
		{
			/*if (Logger::addresses.size()) {
				for (size_t i = 0; i < Logger::addresses.size(); i++) {
					const char* toSend = Utils::String::VA("%i %s", channel, message);
					Network::Send(Logger::addresses[i], toSend);
				}
			}*/
			Game::Com_PrintMessage(channel, message.data(), 0);
		}
	}

	void Logger::ErrorPrint(int error, std::string message)
	{
		return Game::Com_Error(error, "%s", message.data());
	}

	void Logger::Error(int error, const char* message, ...)
	{
		return Logger::ErrorPrint(error, Logger::Format(&message));
	}

	void Logger::Error(const char* message, ...)
	{
		return Logger::ErrorPrint(0, Logger::Format(&message));
	}

	void Logger::SoftError(const char* message, ...)
	{
		return Logger::ErrorPrint(2, Logger::Format(&message));
	}

	std::string Logger::Format(const char** message)
	{
		char buffer[0x1000] = { 0 };

		va_list ap = reinterpret_cast<char*>(const_cast<char**>(&message[1]));
		//va_start(ap, *message);
		_vsnprintf_s(buffer, sizeof(buffer), *message, ap);
		va_end(ap);

		return buffer;
	}

	void Logger::Frame()
	{
		Logger::MessageMutex.lock();

		for (unsigned int i = 0; i < Logger::MessageQueue.size(); ++i)
		{
			
			Game::Com_PrintMessage(0, Logger::MessageQueue[i].data(), 0);

			if (!Logger::IsConsoleReady())
			{
				OutputDebugStringA(Logger::MessageQueue[i].data());
			}
		}

		Logger::MessageQueue.clear();
		Logger::MessageMutex.unlock();
	}

	void Logger::PipeOutput(void(*callback)(std::string))
	{
		Logger::PipeCallback = callback;
	}

	void Logger::PrintMessagePipe(const char* data)
	{
		if (Logger::PipeCallback)
		{

			Logger::PipeCallback(data);
		}
	}

	__declspec(naked) void Logger::PrintMessageStub()
	{
		__asm
		{
			mov eax, Logger::PipeCallback
			test eax, eax
			jz returnPrint

			push[esp + 8h]
			call Logger::PrintMessagePipe
			add esp, 4h
			

			returnPrint :
			push esi
				mov esi, [esp + 0Ch]

				mov eax, 4AA835h
				jmp eax
		}
	}

	void Logger::EnqueueMessage(std::string message)
	{
		Logger::MessageMutex.lock();
		Logger::MessageQueue.push_back(message);
		Logger::MessageMutex.unlock();
	}

	void Logger::PipeOutputStub(std::string message)
	{
		
		if (Logger::addresses.size()) {
			for (size_t i = 0; i < Logger::addresses.size(); i++) {
				const char* toSend = Utils::String::VA("%i %s", 0, message);
				Network::Send(Logger::addresses[i], toSend);
			}
		}
		//Logger::PrintMessagePipe(message.c_str());
	
	}


	Logger::Logger()
	{
		Logger::PipeOutput(&PipeOutputStub);

		QuickPatch::OnFrame(Logger::Frame);

		Utils::Hook(Game::Com_PrintMessage, Logger::PrintMessageStub, HOOK_JUMP).Install()->Quick();

		//Logging over network stuff
		Game::Cmd_AddCommand("log_add", Game::Cbuf_AddServerText, &sv_log_add, 0);
		Game::Cmd_AddServerCommand("log_add", Logger::SV_Log_Add_f, &sv_log_add2);

		Game::Cmd_AddCommand("log_del", Game::Cbuf_AddServerText, &sv_log_del, 0);
		Game::Cmd_AddServerCommand("log_del", Logger::SV_Log_Del_f, &sv_log_del2);

		Game::Cmd_AddCommand("log_list", Game::Cbuf_AddServerText, &sv_log_list, 0);
		Game::Cmd_AddServerCommand("log_list", Logger::SV_Log_List_f, &sv_log_list2);

		Game::Cmd_AddCommand("g_log_add", Game::Cbuf_AddServerText, &sv_glog_add, 0);
		Game::Cmd_AddServerCommand("g_log_add", Logger::SV_GLog_Add_f, &sv_glog_add2);

		Game::Cmd_AddCommand("g_log_del", Game::Cbuf_AddServerText, &sv_glog_del, 0);
		Game::Cmd_AddServerCommand("g_log_del", Logger::SV_GLog_Del_f, &sv_glog_del2);

		Game::Cmd_AddCommand("g_log_list", Game::Cbuf_AddServerText, &sv_glog_list, 0);
		Game::Cmd_AddServerCommand("g_log_list", Logger::SV_GLog_List_f, &sv_glog_list2);

		Utils::Hook(Logger::fsBuildOSPathForThreadHookLoc, FS_BuildOSPathForThreadHookFunc, HOOK_JUMP).Install()->Quick();
		Logger::FS_BuildOSPathForThreadHookTest();
		iw4m_onelog = (Game::dvar_t*)Game::Dvar_RegisterBool("iw4x_onelog", false, Game::DVAR_FLAG_LATCHED || Game::DVAR_FLAG_SAVED, "Only write the game log to the '" BASEGAME "' OS folder");


	}

	Logger::~Logger()
	{
		Logger::MessageMutex.lock();
		Logger::MessageQueue.clear();
		Logger::MessageMutex.unlock();
	}

	//Logging over network stuff


	void Logger::SV_GLog_Add_f() {
		if (Game::Cmd_Argc() != 2) {
			Game::Com_Printf(0, "USAGE: %s <IP[:Port]/Hostname[:Port]>\n", Game::Cmd_Argv(0));
			return;
		}
		Game::netadr_t ip;
		if (!Game::NET_StringToAdr(Game::Cmd_Argv(1), &ip)) {
			Game::Com_Printf(0, "Invalid address: %s\n", Game::Cmd_Argv(1));
			return;
		}
		for (size_t i = 0; i < Logger::gaddresses.size(); i++) {
			if (Game::NET_CompareAdr(Logger::gaddresses[i], ip)) {
				Game::Com_Printf(0, "Address %s already exists (#%i)\n", Game::Cmd_Argv(1), i);
				return;
			}
		}
		//all good
		Logger::gaddresses.push_back(ip);
		int size = Logger::gaddresses.size();
		Game::Com_Printf(101, "Address %s (#%i) added to games_mp.log stream list\n", Game::NET_AdrToString(Logger::gaddresses[size - 1]), size - 1);
	}
	void Logger::SV_Log_Add_f() {
		if (Game::Cmd_Argc() != 2) {
			Game::Com_Printf(0, "USAGE: %s <IP[:Port]/Hostname[:Port]>\n", Game::Cmd_Argv(0));
			return;
		}
		Game::netadr_t ip;
		if (!Game::NET_StringToAdr(Game::Cmd_Argv(1), &ip)) {
			Game::Com_Printf(0, "Invalid address: %s\n", Game::Cmd_Argv(1));
			return;
		}
		for (size_t i = 0; i < Logger::addresses.size(); i++) {
			if (Game::NET_CompareAdr(Logger::addresses[i], ip)) {
				Game::Com_Printf(0, "Address %s already exists (#%i)\n", Game::Cmd_Argv(1), i);
				return;
			}
		}
		//all good
		Logger::addresses.push_back(ip);
		int size = Logger::addresses.size();
		Game::Com_Printf(101, "Address %s (#%i) added to console_mp.log stream list\n", Game::NET_AdrToString(Logger::addresses[size - 1]), size - 1);
	}


	void Logger::SV_GLog_Del_f() {
		if (Game::Cmd_Argc() != 2) {
			Game::Com_Printf(0, "USAGE: %s <ID>\n", Game::Cmd_Argv(0));
			return;
		}
		int index = 0;
		if (!Logger::validInt(Game::Cmd_Argv(1))) {
			Game::Com_Printf(0, "%s is NaN\n", Game::Cmd_Argv(1));
			return;
		}
		index = atoi(Game::Cmd_Argv(1));
		if (index > -1 && index < (int)Logger::gaddresses.size()) {
			Game::Com_Printf(0, "Address %s (ID %i) removed\n", Game::NET_AdrToString(Logger::gaddresses[index]), index);
			Logger::gaddresses.erase(Logger::gaddresses.begin() + index);
		}
		else {
			Game::Com_Printf(0, "ID %i is not valid\n", index);
		}
	}

	void Logger::SV_Log_Del_f() {
		if (Game::Cmd_Argc() != 2) {
			Game::Com_Printf(0, "USAGE: %s <ID>\n", Game::Cmd_Argv(0));
			return;
		}
		int index = 0;
		if (!Logger::validInt(Game::Cmd_Argv(1))) {
			Game::Com_Printf(0, "%s is NaN\n", Game::Cmd_Argv(1));
			return;
		}
		index = atoi(Game::Cmd_Argv(1));
		if (index > -1 && index < (int)Logger::addresses.size()) {
			Game::Com_Printf(0, "Address %s (ID %i) removed\n", Game::NET_AdrToString(Logger::addresses[index]), index);
			Logger::addresses.erase(Logger::addresses.begin() + index);
		}
		else {
			Game::Com_Printf(0, "ID %i is not valid\n", index);
		}
	}

	void Logger::SV_GLog_List_f() {
		Game::Com_Printf(0, "# ID: Address\n");
		Game::Com_Printf(0, "-------------\n");
		for (size_t i = 0; i < Logger::gaddresses.size(); i++) {
			Game::Com_Printf(0, "#%03d: %5s\n", i, Game::NET_AdrToString(Logger::gaddresses[i]));
		}
	}
	void Logger::SV_Log_List_f() {
		Game::Com_Printf(0, "# ID: Address\n");
		Game::Com_Printf(0, "-------------\n");
		for (size_t i = 0; i < Logger::addresses.size(); i++) {
			Game::Com_Printf(0, "#%03d: %5s\n", i, Game::NET_AdrToString(Logger::addresses[i]));
		}
	}


	bool Logger::validInt(char* str) {
		for (size_t i = 0; i < strlen(str); i++) {
			if (str[i] < '0' || str[i] > '9') {
				return false;
			}
		}
		return true;
	}






	void Logger::FS_BuildOSPathForThreadHookTest()
	{
		Game::dvar_t* g_log = *(Game::dvar_t**)0x1A45D9C;

		if (g_log && strcmp(writeFile, g_log->current.string) == 0)
		{
			if (strcmp(writeFolder, BASEGAME) != 0)
			{
				if (iw4m_onelog->current.boolean)
				{
					strcpy_s(writeFolder, 256, BASEGAME);
				}
			}
		}
	}
	void __declspec(naked) Logger::FS_BuildOSPathForThreadHookFunc()
	{
		__asm
		{
			mov eax, [esp + 8h]
			mov writeFolder, eax
			mov eax, [esp + 0Ch]
			mov writeFile, eax

			mov eax, [esp + 8h]
			push ebp
			push esi
			mov esi, [esp + 0Ch]
			
			jmp fsBuildOSPathForThreadHookLocRet
		}


	}

}
