#define IPC_MAX_RECONNECTS 3
#define IPC_COMMAND_SIZE 100
#define IPC_BUFFER_SIZE 0x2000

#define IPC_PIPE_NAME_SERVER "IW4x-Server"
#define IPC_PIPE_NAME_CLIENT "IW4x-Client"

namespace Components
{
	class Pipe
	{
	public:
		struct Packet
		{
			char command[IPC_COMMAND_SIZE];
			char buffer[IPC_BUFFER_SIZE];
		};

		enum Type
		{
			IPCTYPE_NONE,
			IPCTYPE_SERVER,
			IPCTYPE_CLIENT
		};

		typedef void(__cdecl PacketCallback)(std::string data);
		typedef void(__cdecl Callback)();

		Pipe();
		~Pipe();

		bool connect(std::string name);
		bool create(std::string name);

		bool write(std::string command, std::string data);
		void setCallback(std::string command, Utils::Slot<PacketCallback> callback);
		void onConnect(Callback callback);

	private:
		Utils::Slot<void()> connectCallback;
		std::map<std::string, Utils::Slot<PacketCallback>> packetCallbacks;

		HANDLE pipe;
		std::thread thread;
		bool threadAttached;

		Type type;
		Packet packet;

		char pipeName[MAX_PATH];
		char pipeFile[MAX_PATH];
		unsigned int reconnectAttempt;

		void destroy();
		void setName(std::string name);

		static void ReceiveThread(Pipe* pipe);
	};

	class IPCPipe : public Component
	{
	public:
		IPCPipe();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "IPCPipe"; };
#endif

		static bool Write(std::string command, std::string data);
		static void On(std::string command, Utils::Slot<Pipe::PacketCallback> callback);

	private:
		static Pipe ServerPipe;
		static Pipe ClientPipe;

		static void ConnectClient();
	};
}
