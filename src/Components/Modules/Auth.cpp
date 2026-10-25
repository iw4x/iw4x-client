#include <Utils/InfoString.hpp>

#include <proto/auth.pb.h>

#include "Auth.hpp"
#include "Bans.hpp"
#include "Bots.hpp"
#include "Friends.hpp"
#include "Toast.hpp"

namespace Components
{
  namespace
  {
    // The size of the buffer SV_DirectConnect() copies argv[2] into (with
    // I_strncpyz()) before reading it. We only check what the engine will
    // actually see.
    //
    constexpr std::size_t ENGINE_USERINFO_SIZE = 1024;

    // Return true if info repeats a key, ignoring case.
    //
    // The engine's Info_ValueForKey() returns the first key that matches
    // ignoring case. If a key is repeated, the value we check may not be the
    // one the engine uses, so we reject such userinfo outright.
    //
    bool HasDuplicateInfoKeys(const std::string& info)
    {
      std::unordered_set<std::string> keys;
      std::size_t pos = (!info.empty() && info[0] == '\\') ? 1 : 0;

      while (pos < info.size())
      {
        const auto keyEnd = info.find('\\', pos);
        if (keyEnd == std::string::npos)
        {
          break; // The engine never matches a key without a value.
        }

        if (!keys.insert(Utils::String::ToLower(info.substr(pos, keyEnd - pos))).second)
        {
          return true;
        }

        const auto valueEnd = info.find('\\', keyEnd + 1);
        if (valueEnd == std::string::npos)
        {
          break;
        }

        pos = valueEnd + 1;
      }

      return false;
    }

    // Parse an id the same way the engine does ("%8x%8x"). Note that the
    // engine writes xuid high dword first and steamid low dword first, so
    // lowFirst says which one value is.
    //
    bool ParseEngineId(const std::string& value, const bool lowFirst, std::uint64_t* id)
    {
      unsigned int first{}, second{};
      if (value.empty() || sscanf_s(value.data(), "%8x%8x", &first, &second) != 2)
      {
        return false;
      }

      *id = lowFirst ? ((static_cast<std::uint64_t>(second) << 32) | first) : ((static_cast<std::uint64_t>(first) << 32) | second);
      return true;
    }
  }

  Auth::TokenIncrementing Auth::TokenContainer;

  Utils::Cryptography::Token Auth::GuidToken;
  Utils::Cryptography::Token Auth::ComputeToken;
  Utils::Cryptography::ECC::Key Auth::GuidKey;

  std::vector<std::uint64_t> Auth::BannedUids =
  {
    0xf4d2c30b712ac6e3,
    0xf7e33c4081337fa3,
    0x6f5597f103cc50e9,
    0xecd542eee54ffccf,
    0xA46B84C54694FD5B,
    0xECD542EEE54FFCCF,
    0x759096E09CB2BECF,
  };

  bool Auth::HasAccessToReservedSlot;

  void Auth::Frame()
  {
    if (TokenContainer.generating)
    {
      static double mseconds = 0;
      static Utils::Time::Interval interval;

      if (interval.elapsed(500ms))
      {
        interval.update();

        int diff = Game::Sys_Milliseconds() - TokenContainer.startTime;
        double hashPMS = (TokenContainer.hashes * 1.0) / diff;
        double requiredHashes = std::pow(2, TokenContainer.targetLevel + 1) - TokenContainer.hashes;
        mseconds = requiredHashes / hashPMS;

        // Pad the estimate. We don't want the timer to just sit at 00:00:00
        // while we are still working. Note that a slightly overestimated ETA is
        // generally preferable to appearing stuck for players.
        //
        mseconds += 2 * 60 * 1000;

        if (mseconds < 0) mseconds = 0;
      }

      Localization::Set("MPUI_SECURITY_INCREASE_MESSAGE", Utils::String::VA("Increasing security level from %d to %d (est. %s)", GetSecurityLevel(), TokenContainer.targetLevel, Utils::String::FormatTimeSpan(static_cast<int>(mseconds)).data()));
    }
    else if (TokenContainer.thread.joinable())
    {
      TokenContainer.thread.join();
      TokenContainer.generating = false;

      StoreKey();
      Logger::Debug("Security level is {}", GetSecurityLevel());
      Command::Execute("closemenu security_increase_popmenu", false);

      if (!TokenContainer.cancel)
      {
        if (TokenContainer.command.empty())
        {
          Game::ShowMessageBox(Utils::String::VA("Your new security level is %d", GetSecurityLevel()), "Success");
        }
        else
        {
          Toast::Show("cardicon_locked", "Success", Utils::String::VA("Your new security level is %d", GetSecurityLevel()), 5000);
          Command::Execute(TokenContainer.command, false);
        }
      }

      TokenContainer.cancel = false;
    }
  }

  void Auth::SendConnectDataStub(Game::netsrc_t sock, Game::netadr_t adr, const char* format, int len)
  {
    // Ensure our certificate is loaded
    Steam::SteamUser()->GetSteamID();
    if (!GuidKey.isValid())
    {
      Logger::Error(Game::ERR_SERVERDISCONNECT, "Connecting failed: Guid key is invalid!");
      return;
    }

    if (std::find(BannedUids.begin(), BannedUids.end(), Steam::SteamUser()->GetSteamID().bits) != BannedUids.end())
    {
      GenerateKey();
      Logger::Error(Game::ERR_SERVERDISCONNECT, "Your online profile is invalid. A new key has been generated.");
      return;
    }

    std::string connectString(format, len);
    Game::SV_Cmd_TokenizeString(connectString.data());

    Command::ServerParams params;

    if (params.size() < 3)
    {
      Game::SV_Cmd_EndTokenizedString();
      Logger::Error(Game::ERR_SERVERDISCONNECT, "Connecting failed: Command parsing error!");
      return;
    }

    Utils::InfoString infostr(params[2]);
    std::string challenge = infostr.get("challenge");

    if (challenge.empty())
    {
      Game::SV_Cmd_EndTokenizedString();
      Logger::Error(Game::ERR_SERVERDISCONNECT, "Connecting failed: Challenge parsing error!");
      return;
    }

    if (!Friends::IsInvisible() && !Dvar::Var("cl_anonymous").get<bool>() && Steam::Proxy::SteamUser_)
    {
      infostr.set("realsteamId", Utils::String::VA("%llX", Steam::Proxy::SteamUser_->GetSteamID().bits));
    }

    // Build new connect string
    connectString.clear();
    connectString.append(params[0]);
    connectString.append(" ");
    connectString.append(params[1]);
    connectString.append(" ");
    connectString.append("\"" + infostr.build() + "\"");

    Game::SV_Cmd_EndTokenizedString();

    if (GuidToken.toString().empty() && adr.type != Game::NA_LOOPBACK)
    {
      Game::SV_Cmd_EndTokenizedString();
      Logger::Error(Game::ERR_SERVERDISCONNECT, "Connecting failed: Empty GUID token!");
      return;
    }

    Proto::Auth::Connect connectData;
    connectData.set_token(GuidToken.toString());
    connectData.set_publickey(GuidKey.getPublicKey());
    connectData.set_signature(Utils::Cryptography::ECC::SignMessage(GuidKey, challenge));
    connectData.set_infostring(connectString);

    Network::SendCommand(sock, adr, "connect", connectData.SerializeAsString());
  }

  void Auth::ParseConnectData(Game::msg_t* msg, Game::netadr_t* addr)
  {
    Network::Address address(addr);

    // Parse proto data
    Proto::Auth::Connect connectData;
    if (msg->cursize <= 12 || !connectData.ParseFromString(std::string(reinterpret_cast<char*>(&msg->data[12]), msg->cursize - 12)))
    {
      Logger::PrintFail2Ban("Failed connect attempt from IP address: {}\n", Network::AdrToString(address));
      Network::Send(address, "error\nInvalid connect packet!");
      return;
    }

    // Simply connect, if we're in debug mode, we ignore all security checks
#ifndef DEBUG
    if (address.isLoopback())
#endif
    {
      if (!connectData.infostring().empty())
      {
        Game::SV_Cmd_EndTokenizedString();
        Game::SV_Cmd_TokenizeString(connectData.infostring().data());
        Game::SV_DirectConnect(*address.get());
      }
      else
      {
        Logger::PrintFail2Ban("Failed connect attempt from IP address: {}\n", Network::AdrToString(address));
        Network::Send(address, "error\nInvalid infostring data!");
      }
    }
#ifndef DEBUG
    else
    {
      // Validate proto data
      if (connectData.signature().empty() || connectData.publickey().empty() || connectData.token().empty() || connectData.infostring().empty())
      {
        Logger::PrintFail2Ban("Failed connect attempt from IP address: {}\n", Network::AdrToString(address));
        Network::Send(address, "error\nInvalid connect data!");
        return;
      }

      // Setup new cmd params
      Game::SV_Cmd_EndTokenizedString();
      Game::SV_Cmd_TokenizeString(connectData.infostring().data());

      // Access the params
      Command::ServerParams params;

      // Ensure there are enough params
      if (params.size() < 3)
      {
        Logger::PrintFail2Ban("Failed connect attempt from IP address: {}\n", Network::AdrToString(address));
        Network::Send(address, "error\nInvalid connect string!");
        return;
      }

      // Truncate the userinfo the same way the engine will and make sure no
      // key is ambiguous.
      //
      std::string userInfo = params.get(2);
      if (userInfo.size() >= ENGINE_USERINFO_SIZE)
      {
        userInfo.resize(ENGINE_USERINFO_SIZE - 1);
      }

      if (HasDuplicateInfoKeys(userInfo))
      {
        Logger::PrintFail2Ban("Failed connect attempt from IP address: {}\n", Network::AdrToString(address));
        Network::Send(address, "error\nInvalid connect data!");
        return;
      }

      // Read the required data
      const std::string steamId = Game::Info_ValueForKey(userInfo.data(), "xuid");
      const std::string challenge = Game::Info_ValueForKey(userInfo.data(), "challenge");

      // Parse the id
      std::uint64_t xuid{};
      if (challenge.empty() || !ParseEngineId(steamId, false, &xuid))
      {
        Logger::PrintFail2Ban("Failed connect attempt from IP address: {}\n", Network::AdrToString(address));
        Network::Send(address, "error\nInvalid connect data!");
        return;
      }

      SteamID guid;
      guid.bits = xuid;

      if (Bans::IsBanned({ guid, address.getIP() }))
      {
        Logger::PrintFail2Ban("Failed connect attempt from IP address: {}\n", Network::AdrToString(address));
        Network::Send(address, "error\nEXE_ERR_BANNED_PERM");
        return;
      }

      if (std::find(BannedUids.begin(), BannedUids.end(), xuid) != BannedUids.end())
      {
        Network::Send(address, "error\nYour online profile is invalid. Delete your players folder and restart ^2IW4x^7.");
        return;
      }

      if (xuid != GetKeyHash(connectData.publickey()))
      {
        Network::Send(address, "error\nXUID doesn't match the certificate!");
        return;
      }

      // Make sure the other identity fields match the certificate as well.
      //
      // The engine also takes the player GUID from argv[1] (its first 16
      // characters) and client_t::steamID from the 'steamid' key. Legitimate
      // clients fill both from the same id as 'xuid', so anything else is a
      // spoofing attempt.
      //
      {
        const auto playerGuid = std::string(params.get(1)).substr(0, 16);
        const std::string steamIdKey = Game::Info_ValueForKey(userInfo.data(), "steamid");

        std::uint64_t steamIdValue{};
        if (playerGuid != std::format("{:016x}", xuid) || !ParseEngineId(steamIdKey, true, &steamIdValue) || steamIdValue != xuid)
        {
          Logger::PrintFail2Ban("Failed connect attempt from IP address: {}\n", Network::AdrToString(address));
          Network::Send(address, "error\nConnect identity doesn't match the certificate!");
          return;
        }
      }

      // Verify the signature
      Utils::Cryptography::ECC::Key key;
      key.set(connectData.publickey());

      if (!key.isValid() || !Utils::Cryptography::ECC::VerifyMessage(key, challenge, connectData.signature()))
      {
        Network::Send(address, "error\nChallenge signature was invalid!");
        return;
      }

      // Verify the security level
      auto ourLevel = Dvar::Var("sv_securityLevel").get<unsigned int>();
      auto userLevel = GetZeroBits(connectData.token(), connectData.publickey());

      if (userLevel < ourLevel)
      {
        Network::Send(address, Utils::String::VA("error\nYour security level (%d) is lower than the server's security level (%d)", userLevel, ourLevel));
        return;
      }

      Logger::Debug("Verified XUID {:#X} ({}) from {}", xuid, userLevel, address.getString());
      Game::SV_DirectConnect(*address.get());
    }
#endif
  }

  __declspec(naked) void Auth::DirectConnectStub()
  {
    __asm
    {
      pushad
      lea eax, [esp + 20h]
      push eax
      push esi
      call ParseConnectData
      pop esi
      pop eax
      popad

      push 6265FEh
      retn
    }
  }

  const char* Auth::ProtocolMismatchMessage(const int clientProtocol)
  {
    // Tell the player what to do about it. Note that we can't say which side
    // is out of date: the protocol number went down once (0x96 to 0x95 in
    // 2020, see 9047e7be), so a higher number doesn't necessarily mean a newer
    // version. So we cover both cases instead.
    //
    return Utils::String::VA("error\n\x15Your IW4x version does not match this server's (protocol %i, server %i). "
                             "Update IW4x and try again. If you are already on the latest version, the server is out of date.\n",
                             clientProtocol, PROTOCOL);
  }

  __declspec(naked) void Auth::ProtocolMismatchStub()
  {
    __asm
    {
      push esi // Client protocol, as parsed by SV_DirectConnect.
      call ProtocolMismatchMessage
      add esp, 4
      retn
    }
  }

  char* Auth::Info_ValueForKeyStub(const char* s, const char* key)
  {
    auto* value = Game::Info_ValueForKey(s, key);

    HasAccessToReservedSlot = std::strcmp((*Game::sv_privatePassword)->current.string, value) == 0;

    // This stub runs right before the 'server is full check' so we can call this here
    Bots::SV_DirectConnect_Full_Check();

    return value;
  }

  __declspec(naked) void Auth::DirectConnectPrivateClientStub()
  {
    __asm
    {
      push eax

      mov al, HasAccessToReservedSlot
      test al, al

      pop eax

      je noAccess

      // Set the number of private clients to 0 if the client has the right password
      xor eax, eax
      jmp safeContinue

      noAccess :
      mov eax, dword ptr[edx + 0x10]

        safeContinue :
        // Game code skipped by hook
        add esp, 0xC

        push 0x460FB3
        ret
    }
  }

  std::string Auth::GetGUIDFilePath()
  {
    const auto appdata = Components::FileSystem::GetAppdataPath();
    Utils::IO::CreateDir(appdata.string());

    const auto guidPath = appdata / "guid.dat";

    return guidPath.string();
  }

  void ClientConnectFailedStub(Game::netsrc_t sock, Game::netadr_t adr, const char* data)
  {
    Logger::PrintFail2Ban("Failed connect attempt from IP address: {}\n", Network::AdrToString(adr));
    Game::NET_OutOfBandPrint(sock, adr, data);
  }

  unsigned __int64 Auth::GetKeyHash(const std::string& key)
  {
    std::string hash = Utils::Cryptography::SHA1::Compute(key);

    if (hash.size() >= 8)
    {
      return *reinterpret_cast<unsigned __int64*>(const_cast<char*>(hash.data()));
    }

    return 0;
  }

  unsigned __int64 Auth::GetKeyHash()
  {
    LoadKey();
    return GetKeyHash(GuidKey.getPublicKey());
  }

  void Auth::StoreKey()
  {
    if (!Dedicated::IsEnabled() && !ZoneBuilder::IsEnabled() && GuidKey.isValid())
    {
      Proto::Auth::Certificate cert;
      cert.set_token(GuidToken.toString());
      cert.set_ctoken(ComputeToken.toString());
      cert.set_privatekey(GuidKey.serialize(PK_PRIVATE));

      const auto guidPath = GetGUIDFilePath();
      Utils::IO::WriteFile(guidPath, cert.SerializeAsString());
    }
  }

  void Auth::GenerateKey()
  {
    GuidToken.clear();
    ComputeToken.clear();
    // Generate the identity key from the CSPRNG only. Hardware identifiers
    // are neither secret nor random enough: if we derived the key from them,
    // anyone who learns them could rebuild it.
    //
    GuidKey = Utils::Cryptography::ECC::GenerateKey(512);
    StoreKey();
  }

  void Auth::LoadKey(bool force)
  {
    if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled()) return;
    if (!force && GuidKey.isValid()) return;

    const auto guidPath = GetGUIDFilePath();
    const auto guidFile = Utils::IO::ReadFile(guidPath);

    Proto::Auth::Certificate cert;
    if (cert.ParseFromString(guidFile))
    {
      GuidKey.deserialize(cert.privatekey());
      GuidToken = cert.token();
      ComputeToken = cert.ctoken();
    }
    else
    {
      GuidKey.free();
    }

    if (GuidKey.isValid())
    {
      //All good, nothing to do
    }
    else
    {
      Auth::GenerateKey();
    }
  }

  bool Auth::SignForMaster(const std::string& message, std::string* publicKey, std::string* token, std::string* signature)
  {
    if (!GuidKey.isValid() || GuidToken.toString().empty())
    {
      return false;
    }

    // Note that, unlike when connecting to a server, we sign the message's
    // SHA-512 digest rather than the message itself since that's what other
    // ECDSA implementations (such as the master server's) expect.
    //
    *publicKey = GuidKey.getPublicKey();
    *token = GuidToken.toString();
    *signature = Utils::Cryptography::ECC::SignMessage(GuidKey, Utils::Cryptography::SHA512::Compute(message));

    return !signature->empty();
  }

  uint32_t Auth::GetSecurityLevel()
  {
    return GuidToken.toString().empty() ? 0 : GetZeroBits(GuidToken, GuidKey.getPublicKey());
  }

  void Auth::IncreaseSecurityLevel(uint32_t level, const std::string& command)
  {
    if (GetSecurityLevel() >= level) return;

    if (!TokenContainer.generating)
    {
      TokenContainer.cancel = false;
      TokenContainer.targetLevel = level;
      TokenContainer.command = command;

      // Open menu
      Command::Execute("openmenu security_increase_popmenu", true);

      // Start thread
      TokenContainer.thread = std::jthread([&level]()
        {
          TokenContainer.generating = true;
          TokenContainer.hashes = 0;
          TokenContainer.startTime = Game::Sys_Milliseconds();
          IncrementToken(GuidToken, ComputeToken, GuidKey.getPublicKey(), TokenContainer.targetLevel, &TokenContainer.cancel, &TokenContainer.hashes);
          TokenContainer.generating = false;

          if (TokenContainer.cancel)
          {
            Logger::Print("Token incrementation thread terminated\n");
          }
        });
    }
  }

  uint32_t Auth::GetZeroBits(Utils::Cryptography::Token token, const std::string& publicKey)
  {
    std::string message = publicKey + token.toString();
    std::string hash = Utils::Cryptography::SHA512::Compute(message, false);

    uint32_t bits = 0;

    for (unsigned int i = 0; i < hash.size(); ++i)
    {
      if (hash[i] == '\0')
      {
        bits += 8;
        continue;
      }

      uint8_t value = static_cast<uint8_t>(hash[i]);
      for (int j = 7; j >= 0; --j)
      {
        if ((value >> j) & 1)
        {
          return bits;
        }

        ++bits;
      }
    }

    return bits;
  }

  void Auth::IncrementToken(Utils::Cryptography::Token& token, Utils::Cryptography::Token& computeToken, const std::string& publicKey, uint32_t zeroBits, bool* cancel, uint64_t* count)
  {
    if (zeroBits > 512) return; // Not possible, due to SHA512

    if (computeToken < token)
    {
      computeToken = token;
    }

    // Check if we already have the desired security level
    uint32_t lastLevel = token.toString().empty() ? 0 : GetZeroBits(token, publicKey);
    uint32_t level = lastLevel;
    if (level >= zeroBits) return;

    do
    {
      ++computeToken;
      if (count) ++(*count);
      level = GetZeroBits(computeToken, publicKey);

      // Store level if higher than the last one
      if (level >= lastLevel)
      {
        token = computeToken;
        lastLevel = level;
      }

      // Allow canceling that shit
      if (cancel && *cancel) return;
    } while (level < zeroBits);

    token = computeToken;
  }

  // A somewhat hardware tied 48 bit value
  std::string Auth::GetMachineEntropy()
  {
    std::string entropy{};
    DWORD volumeID;
    if (GetVolumeInformationA("C:\\",
      NULL,
      NULL,
      &volumeID,
      NULL,
      NULL,
      NULL,
      NULL
    ))
    {
      // Drive info
      entropy += std::to_string(volumeID);
    }

    // MAC Address
    {
      unsigned long outBufLen = 0;
      DWORD dwResult = GetAdaptersInfo(NULL, &outBufLen);
      if (dwResult == ERROR_BUFFER_OVERFLOW)  // This is what we're expecting
      {
        // Now allocate a structure of the required size.
        std::vector<std::uint8_t> buffer(outBufLen);
        auto* pIpAdapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(buffer.data());
        {
          dwResult = GetAdaptersInfo(pIpAdapterInfo, &outBufLen);
          if (dwResult == ERROR_SUCCESS)
          {
            for (auto* adapter = pIpAdapterInfo; adapter; adapter = adapter->Next)
            {
              switch (adapter->Type)
              {
                case IF_TYPE_IEEE80211:
                case MIB_IF_TYPE_ETHERNET:
                {
                  for (UINT i = 0; i < adapter->AddressLength; i++)
                  {
                    entropy += std::to_string(adapter->Address[i]);
                  }

                  break;
                }
              }
            }
          }
        }
      }

    }

    if (entropy.empty())
    {
      // ultimate fallback
      return std::to_string(Utils::Cryptography::Rand::GenerateLong());
    }
    else
    {
      return entropy;
    }
  }

  Auth::Auth()
  {
    TokenContainer.cancel = false;
    TokenContainer.generating = false;

    HasAccessToReservedSlot = false;

    Localization::Set("MPUI_SECURITY_INCREASE_MESSAGE", "");

    // Load the key
    LoadKey(true);
    Steam::SteamUser()->GetSteamID();

    Scheduler::Loop(Frame, Scheduler::Pipeline::MAIN);

    // Register dvar
    Dvar::Register<int>("sv_securityLevel", 23, 0, 512, Game::DVAR_SERVERINFO, "Security level for GUID certificates (POW)");

    // Install registration hook
    Utils::Hook(0x6265F9, DirectConnectStub, HOOK_JUMP).install()->quick();
    Utils::Hook(0x460EF5, Info_ValueForKeyStub, HOOK_CALL).install()->quick();
    Utils::Hook(0x460FAD, DirectConnectPrivateClientStub, HOOK_JUMP).install()->quick();
    Utils::Hook::Nop(0x460FAD + 5, 1);

    // Replace the protocol mismatch error SV_DirectConnect sends. Its stock
    // message is EXE_SERVER_IS_DIFFERENT_VER with a hardcoded "1.0" argument,
    // which tells the player nothing. We keep the engine's check and only
    // change what it says.
    //
    Utils::Hook(0x46058E, ProtocolMismatchStub, HOOK_CALL).install()->quick();

    Utils::Hook(0x41D3E3, SendConnectDataStub, HOOK_CALL).install()->quick();

    // Hook for Fail2Ban (Hook near client connect to detect password brute forcing)
    Utils::Hook(0x4611CA, ClientConnectFailedStub, HOOK_CALL).install()->quick(); // NET_OutOfBandPrint (Grab IP super easy)

    // SteamIDs can only contain 31 bits of actual 'id' data.
    // The other 33 bits are steam internal data like universe and so on.
    // Using only 31 bits for fingerprints is pretty insecure.
    // The function below verifies the integrity steam's part of the SteamID.
    // Patching that check allows us to use 64 bit for fingerprints.
    Utils::Hook::Set<std::uint32_t>(0x4D0D60, 0xC301B0);

    // Guid command
    Command::Add("guid", []
      {
        Logger::Print("Your guid: {:#X}\n", Steam::SteamUser()->GetSteamID().bits);
      });

    if (!Dedicated::IsEnabled() && !ZoneBuilder::IsEnabled())
    {
      Command::Add("securityLevel", [](const Command::Params* params)
        {
          if (params->size() < 2)
          {
            const auto level = GetZeroBits(GuidToken, GuidKey.getPublicKey());
            Logger::Print("Your current security level is {}\n", level);
            Logger::Print("Your security token is: {}\n", Utils::String::DumpHex(GuidToken.toString(), ""));
            Logger::Print("Your computation token is: {}\n", Utils::String::DumpHex(ComputeToken.toString(), ""));

            Toast::Show("cardicon_locked", "^5Security Level", Utils::String::VA("Your security level is %d", level), 3000);
          }
          else
          {
            const auto level = std::strtoul(params->get(1), nullptr, 10);
            IncreaseSecurityLevel(level);
          }
        });
    }

    UIScript::Add("security_increase_cancel", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
      {
        TokenContainer.cancel = true;
        Logger::Print("Token incrementation process canceled!\n");
      });
  }
}
