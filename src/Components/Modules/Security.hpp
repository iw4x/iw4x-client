#pragma once

extern "C"
{
  int Security_G_GetClientScore_Hk();
  void Security_UI_ReplaceDirective_Stub();

  void Security_UI_ReplaceDirective_Hook(int localClientNum, char* message, char* hudElemString, const size_t hudElemStringLength);
}

namespace Components
{
  class Security : public Component
  {
  public:
    Security();

    static int Msg_ReadBitsCompressCheckSV(const unsigned char* from, unsigned char* to, int size);
    static int Msg_ReadBitsCompressCheckCL(const unsigned char* from, unsigned char* to, int size);

  private:
    static long AtolAdjustPlayerLimit(const char* string);

    static void SelectStringTableEntryInDvar_Stub();

    static void G_LogPrintf_Stub(const char* fmt);

    static void NET_DeferPacketToClient_Hk(Game::netadr_t* net_from, Game::msg_t* net_message);

    static void SV_ExecuteClientMessage_Stub(Game::client_s* client, Game::msg_t* msg);
  };
}
