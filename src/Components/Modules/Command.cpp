#include <STDInclude.hpp>

namespace Components
{
	std::unordered_map<std::string, std::function<void(Command::Params*)>> Command::FunctionMap;
	std::unordered_map<std::string, std::function<void(Command::Params*)>> Command::FunctionMapSV;

	std::string Command::Params::join(const int index) const
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

	int Command::ClientParams::size() const noexcept
	{
		return Game::cmd_args->argc[this->nesting_];
	}

	const char* Command::ClientParams::get(const int index) const noexcept
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

	int Command::ServerParams::size() const noexcept
	{
		return Game::sv_cmd_args->argc[this->nesting_];
	}

	const char* Command::ServerParams::get(const int index) const noexcept
	{
		if (index >= this->size())
		{
			return "";
		}

		return Game::sv_cmd_args->argv[this->nesting_][index];
	}

	void Command::Add(const char* name, const std::function<void()>& callback)
	{
		Add(name, [callback]([[maybe_unused]] const Params* params)
		{
			callback();
		});
	}

	void Command::Add(const char* name, const std::function<void(Params*)>& callback)
	{
		const auto command = Utils::String::ToLower(name);

		if (!FunctionMap.contains(command))
		{
			AddRaw(name, MainCallback);
		}

		FunctionMap.insert_or_assign(command, callback);
	}

	void Command::AddSV(const char* name, const std::function<void(Params*)>& callback)
	{
		if (Loader::IsPregame())
		{
			MessageBoxA(nullptr, "Registering server commands in pregame state is illegal!", nullptr, MB_ICONERROR);

#ifdef _DEBUG
			__debugbreak();
#endif

			return;
		}

		const auto command = Utils::String::ToLower(name);

		if (!FunctionMapSV.contains(command))
		{
			AddRawSV(name, MainCallbackSV);
		}

		FunctionMapSV.insert_or_assign(command, callback);
	}

	void Command::AddRaw(const char* name, void(*callback)(), bool key)
	{
		Game::Cmd_AddCommand(name, callback, Allocate(), key);
	}

	void Command::AddRawSV(const char* name, void(*callback)())
	{
		Game::Cmd_AddServerCommand(name, callback, Allocate());

		// If the main command is registered as Cbuf_AddServerText, the command will be redirected to the SV handler
		AddRaw(name, Game::Cbuf_AddServerText_f, false);
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

	Game::cmd_function_s* Command::Find(const std::string& command)
	{
		auto* cmdFunction = *Game::cmd_functions;

		while (cmdFunction != nullptr)
		{
			if (cmdFunction->name != nullptr && cmdFunction->name == command)
			{
				return cmdFunction;
			}

			cmdFunction = cmdFunction->next;
		}

		return nullptr;
	}

	Game::cmd_function_s* Command::Allocate()
	{
		return Utils::Memory::GetAllocator()->allocate<Game::cmd_function_s>();
	}

	void Command::MainCallback()
	{
		ClientParams params;
		const auto command = Utils::String::ToLower(params[0]);

		if (const auto itr = FunctionMap.find(command); itr != FunctionMap.end())
		{
			itr->second(&params);
		}
	}

	void Command::MainCallbackSV()
	{
		ServerParams params;
		const auto command = Utils::String::ToLower(params[0]);

		if (const auto itr = FunctionMapSV.find(command); itr != FunctionMapSV.end())
		{
			itr->second(&params);
		}
	}
}
