#include "Bindings.hpp"

namespace Components
{
  namespace
  {
    constexpr std::uintptr_t ImageBase = 0x140000000;

    // com_frameTime, which CL_ExecBinding() passes to the button commands it
    // runs.
    //
    const auto& FrameTime = *reinterpret_cast<const int*>(0x141BD7958);
  }

  std::deque<std::string> Bindings::Commands;

  void (*Bindings::CL_ExecBinding_Original)(int localClientNum, int binding, int key);

  // Note that we compare ignoring case, so that 'bind k +ATTACK' still ends
  // up as the game's own binding rather than one of ours.
  //
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
    // An empty command is the empty entry, which is how a key gets unbound.
    //
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

    // The way x86 runs a button command from a binding is to append the key
    // and the time of the key event, which the command (and so the button
    // state) then goes by. x64 doesn't pass the time of the event down to
    // CL_ExecBinding(), so the frame time is the best we have here. It's
    // probably close enough for a button, though it does mean two presses
    // within one frame get the same time. We haven't seen that matter, but if
    // something like a double tap ever feels off on x64, this might be why.
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

    // Join the rest of the arguments into the command, the way x86's bind
    // does. If there are several, then quote the ones with a space in them, so
    // that something like 'bind k say "hello there" again' keeps its words
    // together when the command runs. A single argument we take as it is,
    // quotes and all, since that's presumably exactly what the user wanted.
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

  // On key down CL_KeyEvent() calls this for whatever the key is bound to.
  // The game's own bindings we pass on untouched, and the ones past the
  // game's table are ours to run.
  //
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

    // The game's table is a static array of 78 pointers in its data, with no
    // room to grow, so we make our own copy with room for MAX_BINDINGS and
    // point the game at that instead. We allocate it near the image since the
    // game reaches the table with 32-bit displacements, which have to reach
    // the copy as well.
    //
    const auto* gameTable = Game::keyBindingCommands;
    auto* table = static_cast<const char**>(Utils::Hook::AllocateNear(ImageBase, MAX_BINDINGS * sizeof(const char*)));
    std::copy_n(gameTable, Game::KEY_BINDING_COMMAND_COUNT, table);
    Game::keyBindingCommands = table;

    // Key_WriteBindings() names each binding through the table when it writes
    // the config, so point it at ours, which takes care of saving our
    // bindings. The lookup of a binding by its command (which bind and the
    // menus use) we replace with Find() altogether, since it has to know about
    // the commands we added and the game's would stop at its 78.
    //
    Utils::Hook::Displace(0x1400EFCEE, 7, reinterpret_cast<std::uintptr_t>(gameTable), reinterpret_cast<std::uintptr_t>(table));
    Utils::Hook::Detour(0x1400EF600, 5, Find);

    // x64's bind and unbind print nothing, and its bind seems to only look
    // at the first word of the command, which is no use to us. So we swap in
    // our own versions of both, which behave like x86's. The game registers
    // them with a lea of the handler whose 32-bit displacement can't reach
    // our module, so we point it at a relay instead.
    //
    // Note that on x86 we also disable the game's bind protection, which x64
    // doesn't have in the first place.
    //
    Utils::Hook::Displace(0x1400EE76B, 7, 0x1400EF440, reinterpret_cast<std::uintptr_t>(Utils::Hook::Relay(0x1400EE76B + 7, reinterpret_cast<void*>(Key_Bind_f))));
    Utils::Hook::Displace(0x1400EE785, 7, 0x1400EFBB0, reinterpret_cast<std::uintptr_t>(Utils::Hook::Relay(0x1400EE785 + 7, reinterpret_cast<void*>(Key_Unbind_f))));

    // Binding a key from a menu (the controls options, say) looks the
    // command up and then binds the key to whatever it found. Presumably the
    // menus only ever bind the game's own commands, but a mod's menu could
    // well bind anything, so we let this one add to the table too.
    //
    Utils::Hook(0x14025D245, FindOrAdd, HOOK_CALL).install()->quick();

    // And finally, running our bindings. Key down is the easy part, since
    // CL_ExecBinding() runs every binding and we can just detour it (see
    // CL_ExecBinding_Hk()).
    //
    // Key up is more awkward. CL_KeyEvent() checks the binding itself and
    // only calls CL_ExecBinding() for the release command of a game button,
    // that is, for an odd binding below 41h (the first command past the
    // buttons), with the binding plus one. Ours are all past that and so
    // never get there. So we jump to Bindings_KeyUpStub where the check
    // starts. At that point the binding is in eax, the key in ebx, and the
    // local client in edi. For the game's bindings the stub just goes on with
    // the check, and for ours it calls Bindings_KeyUp() and goes on past the
    // call. The game also keeps something in rdx across that call, which it
    // tests right after (key catchers, we think, judging by the
    // 'test dl, 10h'). Our call clobbers it, so the stub reloads it from the
    // same global the game loads it from, the same as the game itself does
    // after its own call.
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
