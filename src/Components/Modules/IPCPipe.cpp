#include <STDInclude.hpp>

namespace Components
{
	Pipe IPCPipe::ServerPipe;
	Pipe IPCPipe::ClientPipe;

#pragma region Pipe

	Pipe::Pipe() : connectCallback(0), pipe(INVALID_HANDLE_VALUE), threadAttached(false), type(IPCTYPE_NONE), reconnectAttempt(0)
	{
		this->destroy();
	}

	Pipe::~Pipe()
	{
		this->destroy();
	}

	bool Pipe::connect(const std::string& name)
	{
		this->destroy();

		this->type = IPCTYPE_CLIENT;
		this->setName(name);

		this->pipe = CreateFileA(this->pipeFile, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

		if (INVALID_HANDLE_VALUE == this->pipe)
		{
			Logger::Print("Failed to connect to the pipe\n");

			if (this->reconnectAttempt < IPC_MAX_RECONNECTS)
			{
				Logger::Print("Attempting to reconnect to the pipe.\n");
				++this->reconnectAttempt;
				std::this_thread::sleep_for(500ms);

				return this->connect(name);
			}
			else
			{
				this->destroy();
				return false;
			}
		}

		this->reconnectAttempt = 0;
		Logger::Print("Successfully connected to the pipe\n");

		return true;
	}

	bool Pipe::create(const std::string& name)
	{
		this->destroy();

		this->type = IPCTYPE_SERVER;
		this->setName(name);

		this->pipe = CreateNamedPipeA(this->pipeFile, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, sizeof(this->packet), sizeof(this->packet), NMPWAIT_USE_DEFAULT_WAIT, nullptr);

		if (INVALID_HANDLE_VALUE != this->pipe && this->pipe)
		{
			// Only create the thread, when not performing unit tests!
			if (!Loader::IsPerformingUnitTests())
			{
				this->threadAttached = true;
				this->thread = std::thread(Pipe::ReceiveThread, this);
			}

			Logger::Print("Pipe successfully created\n");
			return true;
		}

		Logger::Print("Failed to create the pipe\n");
		this->destroy();
		return false;
	}

	void Pipe::onConnect(Pipe::Callback callback)
	{
		this->connectCallback = callback;
	}

	void Pipe::setCallback(const std::string& command, Utils::Slot<Pipe::PacketCallback> callback)
	{
		this->packetCallbacks[command] = callback;
	}

	bool Pipe::write(const std::string& command, const std::string& data)
	{
		if (this->type != IPCTYPE_CLIENT || this->pipe == INVALID_HANDLE_VALUE) return false;

		Pipe::Packet _packet;
		strcpy_s(_packet.command, command.data());
		strcpy_s(_packet.buffer, data.data());

		DWORD cbBytes;
		return (WriteFile(this->pipe, &_packet, sizeof(_packet), &cbBytes, nullptr) || GetLastError() == ERROR_IO_PENDING);
	}

	void Pipe::destroy()
	{
		//this->Type = IPCTYPE_NONE;

		//*this->PipeFile = 0;
		//*this->PipeName = 0;

		if (this->pipe && INVALID_HANDLE_VALUE != this->pipe)
		{
			CancelIoEx(this->pipe, nullptr);
			//DeleteFileA(this->pipeFile);

			if (this->type == IPCTYPE_SERVER) DisconnectNamedPipe(this->pipe);

			CloseHandle(this->pipe);
			Logger::Print("Disconnected from the pipe.\n");
		}

		this->pipe = nullptr;
		this->threadAttached = false;

		if (this->thread.joinable())
		{
			Logger::Print("Terminating pipe thread...\n");

			this->thread.join();

			Logger::Print("Pipe thread terminated.\n");
		}
	}

	void Pipe::setName(const std::string& name)
	{
		memset(this->pipeName, 0, sizeof(this->pipeName));
		memset(this->pipeFile, 0, sizeof(this->pipeFile));

		strncpy_s(this->pipeName, name.data(), sizeof(this->pipeName));
		sprintf_s(this->pipeFile, sizeof(this->pipeFile), "\\\\.\\Pipe\\%s", this->pipeName);
	}

	void Pipe::ReceiveThread(Pipe* pipe)
	{
		if (!pipe || pipe->type != IPCTYPE_SERVER || pipe->pipe == INVALID_HANDLE_VALUE || !pipe->pipe) return;

		if (ConnectNamedPipe(pipe->pipe, nullptr) == FALSE)
		{
			Logger::Print("Failed to initialize pipe reading.\n");
			return;
		}

		Logger::Print("Client connected to the pipe\n");
		pipe->connectCallback();

		DWORD cbBytes;

		while (pipe->threadAttached && pipe->pipe && pipe->pipe != INVALID_HANDLE_VALUE)
		{
			BOOL bResult = ReadFile(pipe->pipe, &pipe->packet, sizeof(pipe->packet), &cbBytes, nullptr);

			if (bResult && cbBytes)
			{
				if (pipe->packetCallbacks.contains(pipe->packet.command))
				{
					pipe->packetCallbacks[pipe->packet.command](pipe->packet.buffer);
				}
			}
			else if (pipe->threadAttached && pipe->pipe != INVALID_HANDLE_VALUE)
			{
				Logger::Print("Failed to read from client through pipe\n");

				DisconnectNamedPipe(pipe->pipe);
				ConnectNamedPipe(pipe->pipe, nullptr);
				pipe->connectCallback();
			}

			ZeroMemory(&pipe->packet, sizeof(pipe->packet));
		}
	}

#pragma endregion

	// Callback to connect first instance's client pipe to the second instance's server pipe
	void IPCPipe::ConnectClient()
	{
		if (Singleton::IsFirstInstance())
		{
			IPCPipe::ClientPipe.connect(IPC_PIPE_NAME_CLIENT);
		}
	}

	// Writes to the process on the other end of the pipe
	bool IPCPipe::Write(const std::string& command, const std::string& data)
	{
		return IPCPipe::ClientPipe.write(command, data);
	}

	// Installs a callback for receiving commands from the process on the other end of the pipe
	void IPCPipe::On(const std::string& command, Utils::Slot<Pipe::PacketCallback> callback)
	{
		IPCPipe::ServerPipe.setCallback(command, callback);
	}

	IPCPipe::IPCPipe()
	{
		if (Dedicated::IsEnabled() || Monitor::IsEnabled() || Loader::IsPerformingUnitTests() || ZoneBuilder::IsEnabled()) return;

		// Server pipe
		IPCPipe::ServerPipe.onConnect(IPCPipe::ConnectClient);
		IPCPipe::ServerPipe.create((Singleton::IsFirstInstance() ? IPC_PIPE_NAME_SERVER : IPC_PIPE_NAME_CLIENT));

		// Connect second instance's client pipe to first instance's server pipe
		if (!Singleton::IsFirstInstance())
		{
			IPCPipe::ClientPipe.connect(IPC_PIPE_NAME_SERVER);
		}

		IPCPipe::On("ping", [](const std::string& data)
		{
			Logger::Print("Received ping form pipe, sending pong!\n");
			IPCPipe::Write("pong", data);
		});

		IPCPipe::On("pong", [](const std::string& /*data*/)
		{
			Logger::Print("Received pong form pipe!\n");
		});

		// Test pipe functionality by sending pings
		Command::Add("ipcping", [](Command::Params*)
		{
			Logger::Print("Sending ping to pipe!\n");
			IPCPipe::Write("ping", "");
		});
	}

	void IPCPipe::preDestroy()
	{
		IPCPipe::ServerPipe.destroy();
		IPCPipe::ClientPipe.destroy();
	}
}
