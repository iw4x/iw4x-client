#pragma once

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

		typedef void(__cdecl PacketCallback)(const std::string& data);
		typedef void(__cdecl Callback)();

		Pipe();
		~Pipe();

		bool connect(const std::string& name);
		bool create(const std::string& name);

		bool write(const std::string& command, const std::string& data);
		void setCallback(const std::string& command, Utils::Slot<PacketCallback> callback);
		void onConnect(Callback callback);

		void destroy();

	private:
		Utils::Slot<void()> connectCallback;
		std::map<std::string, Utils::Slot<PacketCallback>> packetCallbacks;

		HANDLE pipe;
		std::thread thread;
		bool threadAttached;

		Type type;
		Packet packet;

		char pipeName[MAX_PATH]{};
		char pipeFile[MAX_PATH]{};
		unsigned int reconnectAttempt;

		void setName(const std::string& name);

		static void ReceiveThread(Pipe* pipe);
	};

	class IPCPipe : public Component
	{
	public:
		IPCPipe();

		void preDestroy() override;

		static bool Write(const std::string& command, const std::string& data);
		static void On(const std::string& command, const Utils::Slot<Pipe::PacketCallback>& callback);

	private:
		static Pipe ServerPipe;
		static Pipe ClientPipe;

		static void ConnectClient();
	};
}
