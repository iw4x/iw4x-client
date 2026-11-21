#include "PlayerName.hpp"
#include "ClanTags.hpp"
#include "TextRenderer.hpp"

namespace Components
{
  Dvar::Var PlayerName::sv_allowColoredNames;

  void PlayerName::UserInfoCopy(char* buffer, const char* name, const int size)
  {
    if (!sv_allowColoredNames.get<bool>())
    {
      char nameBuffer[64]{};
      TextRenderer::StripColors(name, nameBuffer, sizeof(nameBuffer));
      TextRenderer::StripAllTextIcons(nameBuffer, buffer, size);
    }
    else
    {
      TextRenderer::StripAllTextIcons(name, buffer, size);
    }

    std::string readablePlayerName(buffer);
    Utils::String::Trim(readablePlayerName);

    if (readablePlayerName.size() < 3)
    {
      strncpy(buffer, "Unknown Soldier", size);
    }
  }

  void PlayerName::ClientCleanName(const char* in, char* out, int outSize)
  {
    UserInfoCopy(out, in, outSize);
  }

  int PlayerName::GetClientName(int localClientNum, int index, char* buf, int size)
  {
    const auto result = Game::CL_GetClientName(localClientNum, index, buf, size);

    // Prepend clanName to username & remove the colors
    strncpy_s(buf, size, TextRenderer::StripColors(ClanTags::GetClanTagWithName(index, buf)).data(), size);

    return result;
  }

  char* PlayerName::CleanStrStub(char* string)
  {
    TextRenderer::StripColors(string, string, std::strlen(string) + 1);
    return string;
  }

  bool PlayerName::IsBadChar(int c)
  {
    if (c == '%')
    {
      return true;
    }

    if (c == '~')
    {
      return true;
    }

    if (c < 32 || c > 126)
    {
      return true;
    }

    return false;
  }

  bool PlayerName::CopyClientNameCheck(char* dest, const char* source, int size)
  {
    Game::I_strncpyz(dest, source, size);

    auto i = 0;
    while (i < size - 1 && dest[i] != '\0')
    {
      // Check for various illegal characters
      const auto c = static_cast<unsigned char>(dest[i]);
      if (IsBadChar(c))
      {
        return false;
      }

      ++i;
    }

    return true;
  }

  void PlayerName::DropClient(Game::client_s* drop)
  {
    const auto* reason = "Invalid name detected";
    Network::SendCommand(drop->header.netchan.remoteAddress, "error", reason);
    Game::SV_DropClient(drop, reason, false);
  }

  // Note that on x86 our stub takes the client from a register. On x64 we
  // take it from the name buffer we're copying to, which is the client's.
  //
  void PlayerName::SV_UserinfoChanged_Hk(char* dest, const char* source, int size)
  {
    if (!CopyClientNameCheck(dest, source, size))
    {
      DropClient(reinterpret_cast<Game::client_s*>(dest - offsetof(Game::client_s, name)));
    }
  }

  PlayerName::PlayerName()
  {
    AssertOffset(Game::client_s, name, 0x212C0);

    sv_allowColoredNames = Dvar::Register<bool>("sv_allowColoredNames", true, Game::DVAR_NONE, "Allow colored names on the server");

    // Disable SV_UpdateUserinfo_f to block changing the name ingame
    Utils::Hook::Set<BYTE>(0x140234F40, 0xC3);

    // Allow colored names ingame. Hook placed in ClientUserinfoChanged
    //
    // Note that x64 also inlines ClientUserinfoChanged() into ClientConnect(),
    // so we hook its copy there as well.
    //
    Utils::Hook(0x140196973, ClientCleanName, HOOK_CALL).install()->quick();
    Utils::Hook(0x1401969BB, ClientCleanName, HOOK_CALL).install()->quick();
    Utils::Hook(0x140196031, ClientCleanName, HOOK_CALL).install()->quick(); // ClientConnect
    Utils::Hook(0x140196079, ClientCleanName, HOOK_CALL).install()->quick(); // ClientConnect

    // Though, don't apply that to overhead names.
    Utils::Hook(0x1400D1C9A, GetClientName, HOOK_CALL).install()->quick();

    // Patch I_CleanStr
    Utils::Hook(0x14028A4E0, CleanStrStub, HOOK_JUMP).install()->quick();

    // Detect invalid characters including '%' to prevent format string vulnerabilities.
    // Kicks the player as soon as possible
    Utils::Hook(0x140237E12, SV_UserinfoChanged_Hk, HOOK_CALL).install()->quick();
  }
}
