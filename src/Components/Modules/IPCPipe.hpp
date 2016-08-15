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
			char Command[IPC_COMMAND_SIZE];
			char Buffer[IPC_BUFFER_SIZE];
		};

		enum Type
		{
			IPCTYPE_NONE,
			IPCTYPE_SERVER,
			IPCTYPE_CLIENT
		};

		typedef void(__cdecl* PacketCallback)(std::string data);
		typedef void(__cdecl* Callback)();

		Pipe();
		~Pipe();

		bool Connect(std::string name);
		bool Create(std::string name);

		bool Write(std::string command, std::string data);
		void SetCallback(std::string command, PacketCallback callback);
		void OnConnect(Callback callback);

	private:
		Callback ConnectCallback;
		std::map<std::string, PacketCallback> PacketCallbacks;

		HANDLE hPipe;
		std::thread mThread;
		bool mThreadAttached;

		Type mType;
		Packet mPacket;

		char PipeName[MAX_PATH];
		char PipeFile[MAX_PATH];
		unsigned int ReconnectAttempt;

		void Destroy();
		void SetName(std::string name);

		static void ReceiveThread(Pipe* pipe);
	};

	class IPCPipe : public Component
	{
	public:
		IPCPipe();
		~IPCPipe();

#ifdef DEBUG
		const char* GetName() { return "IPCPipe"; };
#endif

		static bool Write(std::string command, std::string data);
		static void On(std::string command, Pipe::PacketCallback callback);

	private:
		static Pipe* ServerPipe;
		static Pipe* ClientPipe;

		static void ConnectClient();
	};
}
