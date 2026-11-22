#pragma once

extern "C"
{
  void Bindings_KeyUpStub();

  void Bindings_KeyUp(int localClientNum, int binding, int key);
}

namespace Components
{
  // Make key bindings work the way x86 players expect them to.
  //
  // On x86 a key is bound to a string, and when it's pressed the game simply
  // runs that string through the command buffer, the same as if it was typed
  // into the console. Which means a key can be bound to anything at all,
  // from say hello to a vstr that cycles through some script. People rely
  // on that a lot, and their configs are full of such binds.
  //
  // x64 doesn't work like that. There a key is bound to an index into a
  // fixed table of 78 commands that the game knows about (+attack or
  // togglemenu, for example), and CL_ExecBinding() runs the command with a
  // big switch over that index. The button commands come in pairs, the press
  // command (+attack, say) at an odd index and its release command right
  // after it, and on key up CL_KeyEvent() runs the release command of the
  // pair the key is bound to. Presumably this was done so that the client
  // could tell the server which button command ran by index rather than by
  // name (which it does, see Compatibility). But the consequence for us is
  // that binding a key to anything that isn't in that table quietly does
  // nothing, and the key just stays unbound. So every bind in an x86 config
  // to something other than these 78 would be lost, and perhaps worse,
  // without a word about it, since x64's bind doesn't print anything either.
  //
  // What can we do about it? One option would be to replace the whole key
  // binding machinery with x86's, strings and all. But x64 reaches the
  // bindings in many places (the config writer and the menus, to name two),
  // all of which expect an index, so we would end up rewriting a good part
  // of the key handling. Instead we keep the index scheme and grow the
  // table. Any command a key gets bound to that the game doesn't have is
  // appended to the table past the game's 78, and the key is bound to its
  // new index like any other. Then, when such a binding runs, we run it the
  // x86 way. A press command (one that starts with a plus sign) goes
  // through the command buffer with the key and the time on key down, and
  // its release command, the same name with a minus sign, on key up.
  // Anything else runs on key down only. The game's own bindings we leave
  // to the game.
  //
  // This way the rest of the game never has to know. The config writer
  // names our bindings through the same table as the game's, so a key bound
  // to say hello comes back from the config as just another bind, and the
  // menus bind through the same lookup.
  //
  // Note that the table only ever grows (until MAX_BINDINGS), even if a key
  // is later rebound to something else. It seems unlikely that anyone binds
  // more than a thousand distinct commands in one session, so we haven't
  // bothered reusing the slots, at least for now.
  //
  class Bindings : public Component
  {
  public:
    Bindings();

    // Return the binding for the command, or 0 (the empty entry) if there is
    // none.
    //
    static int Find(const char* command);

    // As above, but add the command to the table if it isn't there yet. If
    // the table is full, then warn and return 0, which leaves the key
    // unbound.
    //
    static int FindOrAdd(const char* command);

  private:
    static constexpr int MAX_BINDINGS = 1024;

    // The commands we added to the table, which it points into. A deque since
    // growing it doesn't move the strings already in it.
    //
    static std::deque<std::string> Commands;

    // Run one of our bindings the x86 way (see above).
    //
    static void Execute(int localClientNum, int binding, int key, bool down);

    static void Key_Bind_f();
    static void Key_Unbind_f();

    static void (*CL_ExecBinding_Original)(int localClientNum, int binding, int key);
    static void CL_ExecBinding_Hk(int localClientNum, int binding, int key);

    friend void (::Bindings_KeyUp)(int localClientNum, int binding, int key);
  };
}
