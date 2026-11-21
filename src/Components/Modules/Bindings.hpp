#pragma once

extern "C"
{
  void Bindings_KeyUpStub();

  void Bindings_KeyUp(int localClientNum, int binding, int key);
}

namespace Components
{
  // Bind keys to any command.
  //
  // Note that x64 binds a key to an index into its fixed table of button
  // commands, which it runs from CL_ExecBinding(), and leaves a key bound to
  // anything else unbound, where x86 runs any binding through the command
  // buffer. So we grow the table with each other command a key gets bound to
  // and run those as x86 does: a '+' command with the key and the time on key
  // down and its '-' counterpart on key up, and anything else on key down.
  //
  class Bindings : public Component
  {
  public:
    Bindings();

    static int Find(const char* command);
    static int FindOrAdd(const char* command);

  private:
    static constexpr int MAX_BINDINGS = 1024;

    static std::deque<std::string> Commands;

    static void Execute(int localClientNum, int binding, int key, bool down);

    static void Key_Bind_f();
    static void Key_Unbind_f();

    static void (*CL_ExecBinding_Original)(int localClientNum, int binding, int key);
    static void CL_ExecBinding_Hk(int localClientNum, int binding, int key);

    friend void (::Bindings_KeyUp)(int localClientNum, int binding, int key);
  };
}
