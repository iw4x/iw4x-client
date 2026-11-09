#pragma once

extern "C"
{
  extern bool Auth_HasAccessToReservedSlot;

  void Auth_DirectConnectStub();
  void Auth_ProtocolMismatchStub();
  void Auth_DirectConnectPrivateClientStub();

  void Auth_ParseConnectData(Game::msg_t* msg, Game::netadr_t* addr);
  const char* Auth_ProtocolMismatchMessage(int clientProtocol);
}

namespace Components
{
  class Auth : public Component
  {
  public:
    Auth();

    static void StoreKey();
    static void LoadKey(bool force = false);
    static void GenerateKey();

    static unsigned __int64 GetKeyHash();
    static unsigned __int64 GetKeyHash(const std::string& key);

    static uint32_t GetSecurityLevel();
    static void IncreaseSecurityLevel(uint32_t level, const std::string& command = {});

    static uint32_t GetZeroBits(Utils::Cryptography::Token token, const std::string& publicKey);
    static void IncrementToken(Utils::Cryptography::Token& token, Utils::Cryptography::Token& computeToken, const std::string& publicKey, uint32_t zeroBits, bool* cancel = nullptr, uint64_t* count = nullptr);

    static std::string GetMachineEntropy();

  private:
    friend void (::Auth_ParseConnectData)(Game::msg_t* msg, Game::netadr_t* addr);
    friend const char* (::Auth_ProtocolMismatchMessage)(int clientProtocol);

    class TokenIncrementing
    {
    public:
      bool cancel;
      bool generating;
      std::jthread thread;
      uint32_t targetLevel;
      int startTime;
      std::string command;
      uint64_t hashes;
    };

    static TokenIncrementing TokenContainer;

    static Utils::Cryptography::Token GuidToken;
    static Utils::Cryptography::Token ComputeToken;
    static Utils::Cryptography::ECC::Key GuidKey;
    static std::vector<std::uint64_t> BannedUids;

    static void SendConnectDataStub(Game::netsrc_t sock, Game::netadr_t adr, const char* format, int len);
    static void ParseConnectData(Game::msg_t* msg, Game::netadr_t* addr);
    static const char* ProtocolMismatchMessage(int clientProtocol);
    static char* Info_ValueForKeyStub(const char* s, const char* key);

    static std::string GetGUIDFilePath();

    static void Frame();
  };
}
