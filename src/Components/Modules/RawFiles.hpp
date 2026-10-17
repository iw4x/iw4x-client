#pragma once

namespace Components
{
  class RawFiles : public Component
  {
  public:
    RawFiles();

    // The size of the loadBuffer every Com_LoadInfoString() caller must
    // provide. The engine's own callers pass 0x2000-byte buffers and its
    // fastfile path reads up to 0x2000 bytes, so our hook doesn't go past
    // that either.
    //
    static constexpr std::size_t LOAD_INFO_STRING_BUFFER_SIZE = 0x2000;

    static char* ReadRawFile(const char* filename, char* buf, int size);

  private:
    static char* GetMenuBuffer(const char* filename);
    static char* Com_LoadInfoString_LoadObj(const char* fileName, const char* fileDesc, const char* ident, char* loadBuffer);
    static const char* Com_LoadInfoString_Hk(const char* fileName, const char* fileDesc, const char* ident, char* loadBuffer);
  };
}
