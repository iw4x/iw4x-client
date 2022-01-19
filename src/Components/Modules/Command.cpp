#include "STDInclude.hpp"

namespace Components
{
	std::unordered_map<std::string, Utils::Slot<Command::Callback>> Command::FunctionMap;
	std::unordered_map<std::string, Utils::Slot<Command::Callback>> Command::FunctionMapSV;

	std::string Command::Params::join(size_t startIndex)
	{
		std::string result;

		for (size_t i = startIndex; i < this->length(); ++i)
		{
			if (i > startIndex) result.append(" ");
			result.append(this->operator[](i));
		}

		return result;
	}

	const char* Command::Params::operator[](size_t index)
	{
		return this->get(index);
	}

	const char* Command::ClientParams::get(size_t index)
	{
		if (index >= this->length()) return "";
		return Game::cmd_argv[this->commandId][index];
	}

	size_t Command::ClientParams::length()
	{
		return Game::cmd_argc[this->commandId];
	}

	const char* Command::ServerParams::get(size_t index)
	{
		if (index >= this->length()) return "";
		return Game::cmd_argv_sv[this->commandId][index];
	}

	size_t Command::ServerParams::length()
	{
		return Game::cmd_argc_sv[this->commandId];
	}

	void Command::Add(const char* name, Utils::Slot<Command::Callback> callback)
	{
		std::string command = Utils::String::ToLower(name);

		if (Command::FunctionMap.find(command) == Command::FunctionMap.end())
		{
			Command::AddRaw(name, Command::MainCallback);
		}

		Command::FunctionMap[command] = callback;
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

		std::string command = Utils::String::ToLower(name);

		if (Command::FunctionMapSV.find(command) == Command::FunctionMapSV.end())
		{
			Command::AddRawSV(name, Command::MainCallbackSV);

			// If the main command is registered as Cbuf_AddServerText, the command will be redirected to the SV handler
			Command::AddRaw(name, Game::Cbuf_AddServerText);
		}

		Command::FunctionMapSV[command] = callback;
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
		Command::ClientParams params(*Game::cmd_id);

		std::string command = Utils::String::ToLower(params[0]);

		if (Command::FunctionMap.find(command) != Command::FunctionMap.end())
		{
			Command::FunctionMap[command](&params);
		}
	}

	void Command::MainCallbackSV()
	{
		Command::ServerParams params(*Game::cmd_id_sv);

		std::string command = Utils::String::ToLower(params[0]);

		if (Command::FunctionMapSV.find(command) != Command::FunctionMapSV.end())
		{
			Command::FunctionMapSV[command](&params);
		}
	}

	Command::Command()
	{
		AssertSize(Game::cmd_function_t, 24);

		Command::Add("openLink", [](Command::Params* params)
		{
			if (params->length() > 1)
			{
				Utils::OpenUrl(params->get(1));
			}
		});
	}

	Command::~Command()
	{
		Command::FunctionMap.clear();
		Command::FunctionMapSV.clear();
	}
}
