namespace Components
{
	class Logger : public Component
	{
	public:
		Logger();
		~Logger();

#ifdef DEBUG
		const char* GetName() { return "Logger"; };
#endif

		static void MessagePrint(int channel, std::string message);
		static void Print(int channel, const char* message, ...);
		static void Print(const char* message, ...);
		static void ErrorPrint(int error, std::string message);
		static void Error(const char* message, ...);
		static void Error(int error, const char* message, ...);
		static void SoftError(const char* message, ...);
		static bool IsConsoleReady();

		static void PipeOutput(void(*callback)(std::string));
		static bool validInt(char* str);
		static const DWORD fsBuildOSPathForThreadHookLoc = 0x642139;
		//static const DWORD fsBuildOSPathForThreadHookLocRet = 0x64213F;
		 Game::cmd_function_t sv_log_add;
		 Game::cmd_function_t sv_log_add2;

		 Game::cmd_function_t sv_log_del;
		 Game::cmd_function_t sv_log_del2;

		 Game::cmd_function_t sv_log_list;
		 Game::cmd_function_t sv_log_list2;

		 Game::cmd_function_t sv_glog_add;
		 Game::cmd_function_t sv_glog_add2;

		 Game::cmd_function_t sv_glog_del;
		 Game::cmd_function_t sv_glog_del2;

		 Game::cmd_function_t sv_glog_list;
		 Game::cmd_function_t sv_glog_list2;
		 static void PipeOutputStub(std::string message);
	private:
		static std::mutex MessageMutex;
		static std::vector<std::string> MessageQueue;
		static std::vector<Game::netadr_t> addresses;
		static std::vector<Game::netadr_t> gaddresses;
		static void(*PipeCallback)(std::string);

		
		static void Frame();
		static void PrintMessageStub();
		static void PrintMessagePipe(const char* data);
		static void EnqueueMessage(std::string message);	

		static std::string Format(const char** message);

		//Logging over network stuff
static void FS_BuildOSPathForThreadHookFunc();
		static void FS_BuildOSPathForThreadHookTest();
		static void SV_GLog_Add_f();
		static void SV_Log_Add_f();
		static void SV_GLog_Del_f();
		static void SV_Log_Del_f();
		static void SV_GLog_List_f();
		static void SV_Log_List_f();

		
		
		
		
	};
}
