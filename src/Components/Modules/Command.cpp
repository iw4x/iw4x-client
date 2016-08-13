#include "STDInclude.hpp"

namespace Components
{
	Utils::Memory::Allocator Command::MemAllocator;
	std::map<std::string, wink::slot<Command::Callback>> Command::FunctionMap;
	std::map<std::string, wink::slot<Command::Callback>> Command::FunctionMapSV;

	char* Command::Params::operator[](size_t index)
	{
		if (index >= this->Length()) return "";
		if (this->IsSV) return Game::cmd_argv_sv[this->CommandId][index];
		else return Game::cmd_argv[this->CommandId][index];
	}

	size_t Command::Params::Length()
	{
		if (this->IsSV) return Game::cmd_argc_sv[this->CommandId];
		else return Game::cmd_argc[this->CommandId];
	}

	std::string Command::Params::Join(size_t startIndex)
	{
		std::string result;

		for (size_t i = startIndex; i < this->Length(); ++i)
		{
			if (i > startIndex) result.append(" ");
			result.append(this->operator[](i));
		}

		return result;
	}

	void Command::Add(const char* name, Command::Callback* callback)
	{
		std::string command = Utils::String::ToLower(name);

		if (Command::FunctionMap.find(command) == Command::FunctionMap.end())
		{
			Command::AddRaw(name, Command::MainCallback);
		}

		Command::FunctionMap[command] = callback;
	}

	void Command::AddSV(const char* name, Command::Callback* callback)
	{
		std::string command = Utils::String::ToLower(name);

		if (Command::FunctionMapSV.find(command) == Command::FunctionMapSV.end())
		{
			Command::AddRawSV(name, Command::MainCallbackSV);

			// If the main command is registered as Cbuf_AddServerText, the command will be redirected to the SV handler
			Command::AddRaw(name, Game::Cbuf_AddServerText);
		}

		Command::FunctionMapSV[command] = callback;
	}

	void Command::AddRaw(const char* name, void(*callback)())
	{
		Game::Cmd_AddCommand(name, callback, Command::Allocate(), 0);
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

	Game::cmd_function_t* Command::Find(std::string command)
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
		return Command::MemAllocator.Allocate<Game::cmd_function_t>();
	}

	void Command::MainCallback()
	{
		Command::Params params(false, *Game::cmd_id);

		std::string command = Utils::String::ToLower(params[0]);

		if (Command::FunctionMap.find(command) != Command::FunctionMap.end())
		{
			Command::FunctionMap[command](params);
		}
	}

	void Command::MainCallbackSV()
	{
		Command::Params params(true, *Game::cmd_id_sv);

		std::string command = Utils::String::ToLower(params[0]);

		if (Command::FunctionMapSV.find(command) != Command::FunctionMapSV.end())
		{
			Command::FunctionMapSV[command](params);
		}
	}

	Command::Command()
	{
		// TODO: Add commands here?
	}

	Command::~Command()
	{
		Command::MemAllocator.Clear();
		Command::FunctionMap.clear();
		Command::FunctionMapSV.clear();
	}
}
