#include "Security.hpp"
#include "Huffman.hpp"

namespace Components
{
  void (*Security::UI_ReplaceDirective_Original)(int localClientNum, char* message, char* hudElemString, std::size_t hudElemStringLength);

  int Security::Msg_ReadBitsCompressCheckSV(const unsigned char* from, int size, unsigned char* to, [[maybe_unused]] int toSize)
  {
    static unsigned char buffer[0x8000];

    if (size > 0x800) return 0;
    size = Utils::Huffman::Decompress(from, buffer, size, sizeof(buffer));

    if (size > 0x800) return 0;
    std::memcpy(to, buffer, size);

    return size;
  }

  int Security::Msg_ReadBitsCompressCheckCL(const unsigned char* from, int size, unsigned char* to, [[maybe_unused]] int toSize)
  {
    static unsigned char buffer[0x100000];

    if (size > 0x20000) return 0;
    size = Utils::Huffman::Decompress(from, buffer, size, sizeof(buffer));

    if (size > 0x20000) return 0;
    std::memcpy(to, buffer, size);

    return size;
  }

  long Security::AtolAdjustPlayerLimit(const char* string)
  {
    return std::min<long>(std::atol(string), 18);
  }

  void Security::SelectStringTableEntryInDvar_Stub()
  {
    Command::ClientParams params;

    if (params.size() >= 4)
    {
      const auto* name = params.get(3);
      // If it's a command don't execute it
      if (Command::Find(name) != nullptr)
      {
        Logger::Debug("CL_SelectStringTableEntryInDvar_f: parameter is a command");
        return;
      }

      const auto* dvar = Game::Dvar_FindVar(name);
      if (!dvar)
      {
        // If it's not a dvar let it continue
        Game::CL_SelectStringTableEntryInDvar_f();
        return;
      }

      constexpr auto disallowedFlags = (Game::DVAR_CHEAT | Game::DVAR_INIT
        | Game::DVAR_ROM | Game::DVAR_EXTERNAL | Game::DVAR_LATCH);

      // If it's a dvar check that it does not have disallowed flags
      if ((dvar->flags & disallowedFlags) != 0)
      {
        Logger::Debug("CL_SelectStringTableEntryInDvar_f: parameter is a protected dvar");
        return;
      }
    }

    Game::CL_SelectStringTableEntryInDvar_f();
  }

  int Security::G_GetClientScore_Hk(int clientNum)
  {
    // Note that the game reads the score without checking that the level has
    // its clients, which it doesn't before the game is initialized.
    //
    if (!Game::level->clients)
    {
      return 0;
    }

    return Game::level->clients[clientNum].sess.score;
  }

  void Security::UI_ReplaceDirective_Hk(int localClientNum, char* message, char* hudElemString, std::size_t hudElemStringLength)
  {
    const auto messageLength = strnlen_s(message, hudElemStringLength + 1);

    if (messageLength > hudElemStringLength)
    {
      // Truncate the message to avoid an out of bounds write while iterating message
      message[hudElemStringLength-1] = '\x00';
      Logger::Print("Truncated an overextended UI_ReplaceDirective message ({}+ characters) regarding {}\n", messageLength, hudElemString);
    }

    UI_ReplaceDirective_Original(localClientNum, message, hudElemString, hudElemStringLength);
  }

  void Security::NET_DeferPacketToClient_Hk(Game::netadr_t* net_from, Game::msg_t* net_message)
  {
    assert(net_from);
    assert(net_message);

    if (static_cast<std::size_t>(net_message->cursize) >= sizeof(Game::DeferredMsg::data))
    {
      Logger::Debug("Dropping net_message. Size is {}", net_message->cursize);
      return;
    }

    auto* msg = &Game::deferredQueue->msgs[Game::deferredQueue->send % std::extent_v<decltype(Game::DeferredQueue::msgs)>];
    std::memcpy(msg->data, net_message->data, net_message->cursize);

    msg->datalen = net_message->cursize;
    msg->addr = *net_from;

    InterlockedIncrement(&Game::deferredQueue->send);
  }

  void Security::SV_ExecuteClientMessage_Stub(Game::client_s* client, Game::msg_t* msg)
  {
    if ((client->reliableSequence - client->reliableAcknowledge) < 0)
    {
      Logger::PrintFail2Ban("Invalid packet from IP address: {}\n", Network::AdrToString(client->header.netchan.remoteAddress));
      Logger::Print(Game::CON_CHANNEL_NETWORK, "Negative reliableAcknowledge from {} - cl->reliableSequence is {}, reliableAcknowledge is {}\n",
        client->name, client->reliableSequence, client->reliableAcknowledge);
      client->reliableAcknowledge = client->reliableSequence;
      Game::SV_DropClient(client, "EXE_LOSTRELIABLECOMMANDS", true);
      return;
    }

    Utils::Hook::Call<void(Game::client_s*, Game::msg_t*)>(0x1402365D0)(client, msg);
  }

  Security::Security()
  {
    AssertOffset(Game::client_s, reliableSequence, 0x20E70);
    AssertOffset(Game::client_s, reliableAcknowledge, 0x20E74);

    // Exploit fixes
    // Fallback option for when we don't use our own huffman implementation
    if (!Huffman::isInitialized)
    {
      Utils::Hook(0x140236641, Msg_ReadBitsCompressCheckSV, HOOK_CALL).install()->quick(); // SV_ExecuteClientMessage
      Utils::Hook(0x140100D19, Msg_ReadBitsCompressCheckCL, HOOK_CALL).install()->quick(); // CL_ParseServerMessage
    }

    // Prevent out-of-bound write in UI_ReplaceDirective (Server->Client vulnerability)
    UI_ReplaceDirective_Original = Utils::Hook::Detour(0x140267ED0, 5, &UI_ReplaceDirective_Hk);

    Utils::Hook::Set<std::uint8_t>(0x140237950, 0xC3); // SV_SteamAuthClient

    // CL_HandleRelayPacket
    //
    // x64 inlines it into CL_ConnectionlessPacket(), so we jump from where it
    // starts to where that function returns true, the same as it does once
    // it has relayed the packet.
    //
    // Note that on x86 we also disable Svcmd_EntityList_f, which x64 compiles
    // out.
    //
    Utils::Hook(0x1400F97FE, 0x1400F96F3, HOOK_JUMP).install()->quick();

    // Patch selectStringTableEntryInDvar
    Utils::Hook::Displace(0x1400FBAD0, 7, 0x14026A0E0, reinterpret_cast<uintptr_t>(Utils::Hook::Relay(0x1400FBAD0 + 7, reinterpret_cast<void*>(SelectStringTableEntryInDvar_Stub))));

    // Skip Steam_Frame()'s checks of the clients of a Steam game server. They
    // run every 10 seconds and drop the clients that Steam hasn't
    // authenticated after 40, and none of ours ever are.
    //
    // Note that x86 has no such checks, only the message they kick with. Our
    // Steam layer never reports the game server as connected to Steam, which
    // the checks also wait for, so strictly speaking they would never run.
    // But what if that changes one day? Then every client would get dropped
    // after 40 seconds, which we would rather not find out the hard way.
    //
    Utils::Hook::Set<WORD>(0x14024BB3D, 0xE990);

    // Patch G_GetClientScore for uninitialized game
    Utils::Hook(0x14019CEE0, G_GetClientScore_Hk, HOOK_JUMP).install()->quick();

    // Requests can be malicious
    Utils::Hook(0x14010CDE3, AtolAdjustPlayerLimit, HOOK_CALL).install()->quick(); // PartyHost_HandleJoinPartyRequest

    // Note that on x86 we also fix the call of G_LogPrintf() in
    // GScr_LogPrint(), which passes the script's text as the format. On x64
    // logprint does nothing at all, so there is nothing to fix.
    //

    // Fix packets causing buffer overflow
    //
    // Note that x64 already drops a packet that is larger than the deferred
    // message itself, so there this may well be redundant. It costs nothing
    // to keep, though.
    //
    Utils::Hook(0x14023B2DD, NET_DeferPacketToClient_Hk, HOOK_CALL).install()->quick();

    // Fix server freezer exploit
    Utils::Hook(0x14023B498, SV_ExecuteClientMessage_Stub, HOOK_CALL).install()->quick();

    // The client can fake the info string
    Utils::Hook::Set<std::uint8_t>(0x1402360DA, 0xEB); // SV_DirectConnect

    // Prevent curl 7_19_4 from running
    // Call to DL_Init from Live_Init
    Utils::Hook::Nop(0x1402A2116, 5);
    // Call to DL_CheckOngoingDownloads from Live_Frame
    Utils::Hook::Nop(0x1402A1DD2, 5);
    // Call to LiveStorage_FetchPlaylists from Live_Frame
    Utils::Hook::Nop(0x1402A1D3B, 5);
    // Call to LiveStorage_FetchPlaylists from Live_Init
    Utils::Hook::Nop(0x1402A23ED, 5);
  }
}
