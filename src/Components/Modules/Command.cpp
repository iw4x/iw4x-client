#include <STDInclude.hpp>

namespace Components
{
	std::unordered_map<std::string, Utils::Slot<Command::Callback>> Command::FunctionMap;
	std::unordered_map<std::string, Utils::Slot<Command::Callback>> Command::FunctionMapSV;

	std::string Command::Params::join(const int index)
	{
		std::string result;

		for (auto i = index; i < this->size(); i++)
		{
			if (i > index) result.append(" ");
			result.append(this->get(i));
		}

		return result;
	}

	Command::ClientParams::ClientParams()
		: nesting_(Game::cmd_args->nesting)
	{
		assert(Game::cmd_args->nesting < Game::CMD_MAX_NESTING);
	}

	int Command::ClientParams::size()
	{
		return Game::cmd_args->argc[this->nesting_];
	}

	const char* Command::ClientParams::get(const int index)
	{
		if (index >= this->size())
		{
			return "";
		}

		return Game::cmd_args->argv[this->nesting_][index];
	}

	Command::ServerParams::ServerParams()
		: nesting_(Game::sv_cmd_args->nesting)
	{
		assert(Game::sv_cmd_args->nesting < Game::CMD_MAX_NESTING);
	}

	int Command::ServerParams::size()
	{
		return Game::sv_cmd_args->argc[this->nesting_];
	}

	const char* Command::ServerParams::get(const int index)
	{
		if (index >= this->size())
		{
			return "";
		}

		return Game::sv_cmd_args->argv[this->nesting_][index];
	}

	void Command::Add(const char* name, Utils::Slot<Command::Callback> callback)
	{
		const auto command = Utils::String::ToLower(name);

		if (Command::FunctionMap.find(command) == Command::FunctionMap.end())
		{
			Command::AddRaw(name, Command::MainCallback);
		}

		Command::FunctionMap[command] = std::move(callback);
	}

	void Command::AddSV(const char* name, Utils::Slot<Command::Callback> callback)
	{
		if (Loader::IsPregame())
		{
			MessageBoxA(nullptr, "Registering server commands in pregamestate is illegal!", nullptr, MB_ICONERROR);

#ifdef DEBUG
			__debugbreak();
#endif

			return;
		}

		const auto command = Utils::String::ToLower(name);

		if (Command::FunctionMapSV.find(command) == Command::FunctionMapSV.end())
		{
			Command::AddRawSV(name, Command::MainCallbackSV);

			// If the main command is registered as Cbuf_AddServerText, the command will be redirected to the SV handler
			Command::AddRaw(name, Game::Cbuf_AddServerText);
		}

		FunctionMapSV[command] = std::move(callback);
	}

	void Command::AddRaw(const char* name, void(*callback)(), bool key)
	{
		Game::Cmd_AddCommand(name, callback, Command::Allocate(), key);
	}

	void Command::AddRawSV(const char* name, void(*callback)())
	{
		Game::Cmd_AddServerCommand(name, callback, Command::Allocate());

		// If the main command is registered as Cbuf_AddServerText, the command will be redirected to the SV handler
		Command::AddRaw(name, Game::Cbuf_AddServerText);
	}

	void Command::Execute(std::string command, bool sync)
	{
		command.append("\n"); // Make sure it's terminated

		if (sync)
		{
			Game::Cmd_ExecuteSingleCommand(0, 0, command.data());
		}
		else
		{
			Game::Cbuf_AddText(0, command.data());
		}
	}

	Game::cmd_function_t* Command::Find(const std::string& command)
	{
		Game::cmd_function_t* cmdFunction = *Game::cmd_functions;

		while (cmdFunction)
		{
			if (cmdFunction->name && cmdFunction->name == command)
			{
				return cmdFunction;
			}

			cmdFunction = cmdFunction->next;
		}

		return nullptr;
	}

	Game::cmd_function_t* Command::Allocate()
	{
		return Utils::Memory::GetAllocator()->allocate<Game::cmd_function_t>();
	}

	void Command::MainCallback()
	{
		Command::ClientParams params;

		const auto command = Utils::String::ToLower(params[0]);
		const auto got = Command::FunctionMap.find(command);

		if (got != Command::FunctionMap.end())
		{
			got->second(&params);
		}
	}

	void Command::MainCallbackSV()
	{
		Command::ServerParams params;

		const auto command = Utils::String::ToLower(params[0]);
		const auto got = Command::FunctionMapSV.find(command);

		if (got != Command::FunctionMapSV.end())
		{
			got->second(&params);
		}
	}

	Command::Command()
	{
		AssertSize(Game::cmd_function_t, 24);

		Command::Add("openLink", [](Command::Params* params)
		{
			if (params->size() > 1)
			{
				Utils::OpenUrl(params->get(1));
			}
		});
	}
}
