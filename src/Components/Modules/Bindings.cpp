#include "Bindings.hpp"

namespace Components
{
  namespace
  {
    constexpr std::uintptr_t ImageBase = 0x140000000;

    // The time of the frame, which CL_ExecBinding() runs its button commands
    // with.
    //
    const auto& FrameTime = *reinterpret_cast<const int*>(0x141BD7958);
  }

  std::deque<std::string> Bindings::Commands;

  void (*Bindings::CL_ExecBinding_Original)(int localClientNum, int binding, int key);

  int Bindings::Find(const char* command)
  {
    for (int i = 0; i < Game::keyBindingCommandCount; ++i)
    {
      if (!_stricmp(command, Game::keyBindingCommands[i]))
      {
        return i;
      }
    }

    return 0;
  }

  int Bindings::FindOrAdd(const char* command)
  {
    if (const auto i = Find(command); i || !*command)
    {
      return i;
    }

    if (Game::keyBindingCommandCount == MAX_BINDINGS)
    {
      Logger::Warning(Game::CON_CHANNEL_SYSTEM, "Too many key bindings, can't bind '{}'\n", command);
      return 0;
    }

    Game::keyBindingCommands[Game::keyBindingCommandCount] = Commands.emplace_back(command).c_str();
    return Game::keyBindingCommandCount++;
  }

  void Bindings::Execute(const int localClientNum, const int binding, const int key, const bool down)
  {
    const std::string_view command(Game::keyBindingCommands[binding]);

    // Note that on x86 we run them with the time of the key event, which
    // x64 doesn't pass down to CL_ExecBinding().
    //
    if (command.starts_with('+'))
    {
      Game::Cbuf_AddText(localClientNum, Utils::String::VA("%c%s %i %i\n", down ? '+' : '-', command.data() + 1, key, FrameTime));
    }
    else if (down)
    {
      Game::Cbuf_AddText(localClientNum, Utils::String::VA("%s\n", command.data()));
    }
  }

  void Bindings::Key_Bind_f()
  {
    const Command::ClientParams params;

    if (params.size() < 2)
    {
      Logger::Print("bind <key> [command] : attach a command to a key\n");
      return;
    }

    const auto key = Game::Key_StringToKeynum(params.get(1));
    if (key == -1)
    {
      Logger::Print("\"{}\" isn't a valid key\n", params.get(1));
      return;
    }

    if (params.size() == 2)
    {
      if (const auto binding = Game::playerKeys[0].keys[key].binding; binding > 0 && binding < Game::keyBindingCommandCount)
      {
        Logger::Print("\"{}\" = \"{}\"\n", Game::Key_KeynumToString(key, 1), Game::keyBindingCommands[binding]);
      }
      else
      {
        Logger::Print("\"{}\" is not bound\n", Game::Key_KeynumToString(key, 1));
      }

      return;
    }

    // Join the rest of the arguments, quoting those with a space if there
    // are several.
    //
    std::string command;

    for (int i = 2; i < params.size(); ++i)
    {
      const std::string_view arg(params.get(i));
      const auto quote = params.size() > 3 && arg.find(' ') != std::string_view::npos;

      if (quote) command += '"';
      command += arg;
      if (quote) command += '"';

      if (i != params.size() - 1)
      {
        command += ' ';
      }
    }

    Game::Key_SetBinding(0, key, FindOrAdd(command.data()));
  }

  void Bindings::Key_Unbind_f()
  {
    const Command::ClientParams params;

    if (params.size() != 2)
    {
      Logger::Print("unbind <key> : remove commands from a key\n");
      return;
    }

    const auto key = Game::Key_StringToKeynum(params.get(1));
    if (key == -1)
    {
      Logger::Print("\"{}\" isn't a valid key\n", params.get(1));
      return;
    }

    Game::Key_SetBinding(0, key, 0);
  }

  void Bindings::CL_ExecBinding_Hk(const int localClientNum, const int binding, const int key)
  {
    if (binding < Game::KEY_BINDING_COMMAND_COUNT)
    {
      CL_ExecBinding_Original(localClientNum, binding, key);
      return;
    }

    if (binding < Game::keyBindingCommandCount)
    {
      Execute(localClientNum, binding, key, true);
    }
  }

  Bindings::Bindings()
  {
    if (Dedicated::IsEnabled()) return;

    const auto* gameTable = Game::keyBindingCommands;
    auto* table = static_cast<const char**>(Utils::Hook::AllocateNear(ImageBase, MAX_BINDINGS * sizeof(const char*)));
    std::copy_n(gameTable, Game::KEY_BINDING_COMMAND_COUNT, table);
    Game::keyBindingCommands = table;

    // Key_WriteBindings() names the bindings from the table, and
    // Key_GetBindingForCmd() looks them up in it.
    //
    Utils::Hook::Displace(0x1400EFCEE, 7, reinterpret_cast<std::uintptr_t>(gameTable), reinterpret_cast<std::uintptr_t>(table));
    Utils::Hook::Detour(0x1400EF600, 5, Find);

    // Binding a key, with bind or from a menu, adds its command.
    //
    // Note that x64's bind and unbind print nothing, and bind takes only the
    // first word of the command, so we replace them with x86's. On x86 we
    // also disable the game's bind protection, which x64 doesn't have.
    //
    Utils::Hook::Displace(0x1400EE76B, 7, 0x1400EF440, reinterpret_cast<std::uintptr_t>(Utils::Hook::Relay(0x1400EE76B + 7, reinterpret_cast<void*>(Key_Bind_f))));
    Utils::Hook::Displace(0x1400EE785, 7, 0x1400EFBB0, reinterpret_cast<std::uintptr_t>(Utils::Hook::Relay(0x1400EE785 + 7, reinterpret_cast<void*>(Key_Unbind_f))));
    Utils::Hook(0x14025D245, FindOrAdd, HOOK_CALL).install()->quick();

    // Run ours on key down past the game's table, and on key up, where the
    // game runs the '-' counterparts of its button commands from
    // CL_KeyEvent() for the bindings below 41h. There the binding is in eax,
    // the key in ebx, and the local client in edi, and the game keeps the key
    // catchers in rdx, which our call clobbers, so Bindings.asm reloads them.
    //
    CL_ExecBinding_Original = Utils::Hook::Detour(0x1400F5EC0, 5, CL_ExecBinding_Hk);
    Utils::Hook(0x1400EEA15, Bindings_KeyUpStub, HOOK_JUMP).install()->quick();
  }
}

void Bindings_KeyUp(const int localClientNum, const int binding, const int key)
{
  using Components::Bindings;

  if (binding >= Game::KEY_BINDING_COMMAND_COUNT && binding < Game::keyBindingCommandCount)
  {
    Bindings::Execute(localClientNum, binding, key, false);
  }
}
