#include "STDInclude.hpp"

namespace Components
{
	Pipe* IPCPipe::ServerPipe = 0;
	Pipe* IPCPipe::ClientPipe = 0;

#pragma region Pipe

	Pipe::Pipe() : mType(IPCTYPE_NONE), ReconnectAttempt(0), hPipe(INVALID_HANDLE_VALUE), ConnectCallback(0), mThreadAttached(false)
	{
		this->Destroy();
	}

	Pipe::~Pipe()
	{
		this->Destroy();
	}

	bool Pipe::Connect(std::string name)
	{
		this->Destroy();

		this->mType = IPCTYPE_CLIENT;
		this->SetName(name);

		this->hPipe = CreateFileA(this->PipeFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

		if (INVALID_HANDLE_VALUE == this->hPipe)
		{
			Logger::Print("Failed to connect to the pipe\n");

			if (this->ReconnectAttempt < IPC_MAX_RECONNECTS)
			{
				Logger::Print("Attempting to reconnect to the pipe.\n");
				++this->ReconnectAttempt;
				std::this_thread::sleep_for(500ms);

				return this->Connect(name);
			}
			else
			{
				this->Destroy();
				return false;
			}
		}

		this->ReconnectAttempt = 0;
		Logger::Print("Successfully connected to the pipe\n");

		return true;
	}

	bool Pipe::Create(std::string name)
	{
		this->Destroy();

		this->mType = IPCTYPE_SERVER;
		this->SetName(name);

		this->hPipe = CreateNamedPipeA(this->PipeFile, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, sizeof(this->mPacket), sizeof(this->mPacket), NMPWAIT_USE_DEFAULT_WAIT, NULL);

		if (INVALID_HANDLE_VALUE != this->hPipe)
		{
			// Only create the thread, when not performing unit tests!
			if (!Loader::PerformingUnitTests())
			{
				this->mThreadAttached = true;
				this->mThread = std::thread(Pipe::ReceiveThread, this);
			}

			Logger::Print("Pipe successfully created\n");
			return true;
		}

		Logger::Print("Failed to create the pipe\n");
		this->Destroy();
		return false;
	}

	void Pipe::OnConnect(Pipe::Callback callback)
	{
		this->ConnectCallback = callback;
	}

	void Pipe::SetCallback(std::string command, Pipe::PacketCallback callback)
	{
		this->PacketCallbacks[command] = callback;
	}

	bool Pipe::Write(std::string command, std::string data)
	{
		if (this->mType != IPCTYPE_CLIENT || this->hPipe == INVALID_HANDLE_VALUE) return false;

		Pipe::Packet packet;
		strcpy_s(packet.Command, command.data());
		strcpy_s(packet.Buffer, data.data());

		DWORD cbBytes;
		return (WriteFile(this->hPipe, &packet, sizeof(packet), &cbBytes, NULL) || GetLastError() == ERROR_IO_PENDING);
	}

	void Pipe::Destroy()
	{
		//this->Type = IPCTYPE_NONE;

		//*this->PipeFile = 0;
		//*this->PipeName = 0;

		if (this->hPipe && INVALID_HANDLE_VALUE != this->hPipe)
		{
			if (this->mType == IPCTYPE_SERVER) DisconnectNamedPipe(this->hPipe);

			CloseHandle(this->hPipe);
			Logger::Print("Disconnected from the pipe.\n");
		}

		this->mThreadAttached = false;

		if (this->mThread.joinable())
		{
			Logger::Print("Terminating pipe thread...\n");

			this->mThread.join();

			Logger::Print("Pipe thread terminated.\n");
		}
	}

	void Pipe::SetName(std::string name)
	{
		memset(this->PipeName, 0, sizeof(this->PipeName));
		memset(this->PipeFile, 0, sizeof(this->PipeFile));

		strncpy_s(this->PipeName, name.data(), sizeof(this->PipeName));
		sprintf_s(this->PipeFile, sizeof(this->PipeFile), "\\\\.\\Pipe\\%s", this->PipeName);
	}

	void Pipe::ReceiveThread(Pipe* pipe)
	{
		if (!pipe || pipe->mType != IPCTYPE_SERVER || pipe->hPipe == INVALID_HANDLE_VALUE || !pipe->hPipe) return;

		if (ConnectNamedPipe(pipe->hPipe, NULL) == FALSE)
		{
			Logger::Print("Failed to initialize pipe reading.\n");
			return;
		}

		Logger::Print("Client connected to the pipe\n");
		if (pipe->ConnectCallback) pipe->ConnectCallback();

		DWORD cbBytes;

		while (pipe->mThreadAttached && pipe->hPipe && pipe->hPipe != INVALID_HANDLE_VALUE)
		{
			BOOL bResult = ReadFile(pipe->hPipe, &pipe->mPacket, sizeof(pipe->mPacket), &cbBytes, NULL);

			if (bResult && cbBytes)
			{
				if (pipe->PacketCallbacks.find(pipe->mPacket.Command) != pipe->PacketCallbacks.end())
				{
					pipe->PacketCallbacks[pipe->mPacket.Command](pipe->mPacket.Buffer);
				}
			}
			else if (pipe->mThreadAttached && pipe->hPipe != INVALID_HANDLE_VALUE)
			{
				Logger::Print("Failed to read from client through pipe\n");

				DisconnectNamedPipe(pipe->hPipe);
				ConnectNamedPipe(pipe->hPipe, NULL);
				if (pipe->ConnectCallback) pipe->ConnectCallback();
			}

			ZeroMemory(&pipe->mPacket, sizeof(pipe->mPacket));
		}
	}

#pragma endregion

	// Callback to connect first instance's client pipe to the second instance's server pipe
	void IPCPipe::ConnectClient()
	{
		if (Singleton::IsFirstInstance() && IPCPipe::ClientPipe)
		{
			IPCPipe::ClientPipe->Connect(IPC_PIPE_NAME_CLIENT);
		}
	}

	// Writes to the process on the other end of the pipe
	bool IPCPipe::Write(std::string command, std::string data)
	{
		if (IPCPipe::ClientPipe)
		{
			return IPCPipe::ClientPipe->Write(command, data);
		}

		return false;
	}

	// Installs a callback for receiving commands from the process on the other end of the pipe
	void IPCPipe::On(std::string command, Pipe::PacketCallback callback)
	{
		if (IPCPipe::ServerPipe)
		{
			IPCPipe::ServerPipe->SetCallback(command, callback);
		}
	}

	IPCPipe::IPCPipe()
	{
		if (Dedicated::IsEnabled()) return;

		// Server pipe
		IPCPipe::ServerPipe = new Pipe();
		IPCPipe::ServerPipe->OnConnect(IPCPipe::ConnectClient);
		IPCPipe::ServerPipe->Create((Singleton::IsFirstInstance() ? IPC_PIPE_NAME_SERVER : IPC_PIPE_NAME_CLIENT));

		// Client pipe
		IPCPipe::ClientPipe = new Pipe();

		// Connect second instance's client pipe to first instance's server pipe
		if (!Singleton::IsFirstInstance())
		{
			IPCPipe::ClientPipe->Connect(IPC_PIPE_NAME_SERVER);
		}

		IPCPipe::On("ping", [] (std::string data)
		{
			Logger::Print("Received ping form pipe, sending pong!\n");
			IPCPipe::Write("pong", data);
		});

		IPCPipe::On("pong", [] (std::string data)
		{
			Logger::Print("Received pong form pipe!\n");
		});

		// Test pipe functionality by sending pings
		Command::Add("ipcping", [] (Command::Params)
		{
			Logger::Print("Sending ping to pipe!\n");
			IPCPipe::Write("ping", "");
		});
	}

	IPCPipe::~IPCPipe()
	{
		if (IPCPipe::ServerPipe) delete IPCPipe::ServerPipe;
		if (IPCPipe::ClientPipe) delete IPCPipe::ClientPipe;

		IPCPipe::ServerPipe = nullptr;
		IPCPipe::ClientPipe = nullptr;
	}
}
