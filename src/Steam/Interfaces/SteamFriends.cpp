#include "SteamFriends.hpp"

using namespace Components;

namespace Steam
{
  const char* Friends::GetPersonaName()
  {
    return Dvar::Var("name").get<const char*>();
  }

  uint64_t Friends::SetPersonaName(const char *pchPersonaName)
  {
    Dvar::Var("name").set(pchPersonaName);
    return 0;
  }

  int Friends::GetPersonaState()
  {
    return 1;
  }

  int Friends::GetFriendCount(int /*iFriendFlags*/)
  {
    return 0;
  }

  SteamID* Friends::GetFriendByIndex(SteamID* result, int /*iFriend*/, int /*iFriendFlags*/)
  {
    *result = {};
    return result;
  }

  int Friends::GetFriendRelationship(SteamID /*steamIDFriend*/)
  {
    return 0;
  }

  int Friends::GetFriendPersonaState(SteamID /*steamIDFriend*/)
  {
    return 0;
  }

  const char* Friends::GetFriendPersonaName(SteamID /*steamIDFriend*/)
  {
    return "";
  }

  bool Friends::GetFriendGamePlayed(SteamID /*steamIDFriend*/, FriendGameInfo* /*pFriendGameInfo*/)
  {
    return false;
  }

  const char* Friends::GetFriendPersonaNameHistory(SteamID /*steamIDFriend*/, int /*iPersonaName*/)
  {
    return "";
  }

  bool Friends::HasFriend(SteamID /*steamIDFriend*/, int /*iFriendFlags*/)
  {
    return false;
  }

  int Friends::GetClanCount()
  {
    return 0;
  }

  SteamID* Friends::GetClanByIndex(SteamID* result, int /*iClan*/)
  {
    *result = SteamID();
    return result;
  }

  const char *Friends::GetClanName(SteamID /*steamIDClan*/)
  {
    return "3arc";
  }

  const char* Friends::GetClanTag(SteamID /*steamIDClan*/)
  {
    return "";
  }

  bool Friends::GetClanActivityCounts(SteamID /*steamIDClan*/, int* /*pnOnline*/, int* /*pnInGame*/, int* /*pnChatting*/)
  {
    return false;
  }

  uint64_t Friends::DownloadClanActivityCounts(SteamID* /*psteamIDClans*/, int /*cClansToRequest*/)
  {
    return 0;
  }

  int Friends::GetFriendCountFromSource(SteamID /*steamIDSource*/)
  {
    return 0;
  }

  SteamID* Friends::GetFriendFromSourceByIndex(SteamID* result, SteamID /*steamIDSource*/, int /*iFriend*/)
  {
    *result = {};
    return result;
  }

  bool Friends::IsUserInSource(SteamID /*steamIDUser*/, SteamID /*steamIDSource*/)
  {
    return false;
  }

  void Friends::SetInGameVoiceSpeaking(SteamID /*steamIDUser*/, bool /*bSpeaking*/)
  {
  }

  void Friends::ActivateGameOverlay(const char* /*pchDialog*/)
  {
  }

  void Friends::ActivateGameOverlayToUser(const char* /*pchDialog*/, SteamID /*steamID*/)
  {
  }

  void Friends::ActivateGameOverlayToWebPage(const char* /*pchURL*/)
  {
  }

  void Friends::ActivateGameOverlayToStore(unsigned int /*nAppID*/, int /*eFlag*/)
  {
  }

  void Friends::SetPlayedWith(SteamID /*steamIDUserPlayedWith*/)
  {
  }

  void Friends::ActivateGameOverlayInviteDialog(SteamID /*steamIDLobby*/)
  {
  }

  int Friends::GetSmallFriendAvatar(SteamID /*steamIDFriend*/)
  {
    return 0;
  }

  int Friends::GetMediumFriendAvatar(SteamID /*steamIDFriend*/)
  {
    return 0;
  }

  int Friends::GetLargeFriendAvatar(SteamID /*steamIDFriend*/)
  {
    return 0;
  }

  bool Friends::RequestUserInformation(SteamID /*steamIDUser*/, bool /*bRequireNameOnly*/)
  {
    return false;
  }

  uint64_t Friends::RequestClanOfficerList(SteamID /*steamIDClan*/)
  {
    return 0;
  }

  SteamID* Friends::GetClanOwner(SteamID* result, SteamID /*steamIDClan*/)
  {
    *result = {};
    return result;
  }

  int Friends::GetClanOfficerCount(SteamID /*steamIDClan*/)
  {
    return 0;
  }

  SteamID* Friends::GetClanOfficerByIndex(SteamID* result, SteamID /*steamIDClan*/, int /*iOfficer*/)
  {
    *result = {};
    return result;
  }

  unsigned int Friends::GetUserRestrictions()
  {
    return 0;
  }

  bool Friends::SetRichPresence(const char* /*pchKey*/, const char* /*pchValue*/)
  {
    return false;
  }

  void Friends::ClearRichPresence()
  {
  }

  const char* Friends::GetFriendRichPresence(SteamID /*steamIDFriend*/, const char* /*pchKey*/)
  {
    return "";
  }

  int Friends::GetFriendRichPresenceKeyCount(SteamID /*steamIDFriend*/)
  {
    return 0;
  }

  const char* Friends::GetFriendRichPresenceKeyByIndex(SteamID /*steamIDFriend*/, int /*iKey*/)
  {
    return "";
  }

  void Friends::RequestFriendRichPresence(SteamID /*steamIDFriend*/)
  {
  }

  bool Friends::InviteUserToGame(SteamID /*steamIDFriend*/, const char* /*pchConnectString*/)
  {
    return false;
  }

  int Friends::GetCoplayFriendCount()
  {
    return 0;
  }

  SteamID* Friends::GetCoplayFriend(SteamID* result, int /*iCoplayFriend*/)
  {
    *result = {};
    return result;
  }

  int Friends::GetFriendCoplayTime(SteamID /*steamIDFriend*/)
  {
    return 0;
  }

  unsigned int Friends::GetFriendCoplayGame(SteamID /*steamIDFriend*/)
  {
    return 0;
  }

  uint64_t Friends::JoinClanChatRoom(SteamID /*steamIDClan*/)
  {
    return 0;
  }

  bool Friends::LeaveClanChatRoom(SteamID /*steamIDClan*/)
  {
    return false;
  }

  int Friends::GetClanChatMemberCount(SteamID /*steamIDClan*/)
  {
    return 0;
  }

  SteamID* Friends::GetChatMemberByIndex(SteamID* result, SteamID /*steamIDClan*/, int /*iUser*/)
  {
    *result = {};
    return result;
  }

  bool Friends::SendClanChatMessage(SteamID /*steamIDClanChat*/, const char* /*pchText*/)
  {
    return false;
  }

  int Friends::GetClanChatMessage(SteamID /*steamIDClanChat*/, int /*iMessage*/, void* /*prgchText*/, int /*cchTextMax*/, int* /*peChatEntryType*/, SteamID* /*pSteamIDChatter*/)
  {
    return 0;
  }

  bool Friends::IsClanChatAdmin(SteamID /*steamIDClanChat*/, SteamID /*steamIDUser*/)
  {
    return false;
  }

  bool Friends::IsClanChatWindowOpenInSteam(SteamID /*steamIDClanChat*/)
  {
    return false;
  }

  bool Friends::OpenClanChatWindowInSteam(SteamID /*steamIDClanChat*/)
  {
    return false;
  }

  bool Friends::CloseClanChatWindowInSteam(SteamID /*steamIDClanChat*/)
  {
    return false;
  }

  bool Friends::SetListenForFriendsMessages(bool /*bInterceptEnabled*/)
  {
    return false;
  }

  bool Friends::ReplyToFriendMessage(SteamID /*steamIDFriend*/, const char* /*pchMsgToSend*/)
  {
    return false;
  }

  int Friends::GetFriendMessage(SteamID /*steamIDFriend*/, int /*iMessageID*/, void* /*pvData*/, int /*cubData*/, int* /*peChatEntryType*/)
  {
    return 0;
  }

  uint64_t Friends::GetFollowerCount(SteamID /*steamID*/)
  {
    return 0;
  }

  uint64_t Friends::IsFollowing(SteamID /*steamID*/)
  {
    return 0;
  }

  uint64_t Friends::EnumerateFollowingList(unsigned int /*unStartIndex*/)
  {
    return 0;
  }
}
