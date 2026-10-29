#pragma once

#include <Utils/InfoString.hpp>

#include "QueueState.hpp"

namespace Components
{
  /// Lets players wait for a slot on a full server instead of failing to join.
  ///
  /// The server keeps the order. Waiting clients keep a place by sending
  /// getinfo heartbeats, the server reports their position in the
  /// infoResponse and the gate in Auth refuses connect attempts that would
  /// take a slot held for someone else. No packet types are added.
  class Queue : public Component
  {
  public:
    enum class Decision
    {
      Connect,
      Wait,
      Full,
    };

    Queue();

    /// Server: decides whether a verified connect attempt may reach the engine.
    static bool AllowConnect(const Network::Address& address, const char* password);

    /// Server: appends the queue keys to the infoResponse for a getinfo request.
    static void AddInfoKeys(const Network::Address& address, const std::string& request, Utils::InfoString& info);

    /// Client: decides what to do with the info of the dedicated server the
    /// player is joining. countFull is the verdict of the plain clients count,
    /// used when the server does not report its free slots.
    static Decision OnServerInfo(const Network::Address& target, const Utils::InfoString& info, bool countFull);

    /// Client: stops waiting and leaves the queue on the server.
    static void Cancel();

  private:
    struct Waiting
    {
      bool active;
      Network::Address target;
      std::string cookie;
      std::uint32_t startedAt;
      std::uint32_t lastRequest;
      std::size_t position;
      std::string status;
      bool popupSeen;
    };

    static Dvar::Var SvQueueMax;
    static Dvar::Var SvQueuePerIp;
    static Dvar::Var SvQueueTimeout;
    static Dvar::Var SvQueueReserveTime;
    static Dvar::Var ClQueue;
    static Dvar::Var ClQueueTimeout;

    static QueueState State;
    static std::mutex StateMutex;
    static Waiting Client;

    static bool IsEnabled();
    static std::uint32_t Now();
    static void ApplyConfig();
    static QueueState::Address ToStateAddress(const Network::Address& address);

    static bool HasPrivateSlotAccess(const char* password);
    static bool IsFull(const Utils::InfoString& info, bool countFull);

    static std::string Hmac(const std::string& key, const std::string& message);
    static std::string MakeCookie(const Network::Address& address);
    static std::vector<std::string> ParseRequest(const std::string& request);

    static void ServerFrame();
    static void HandleHeartbeat(const Network::Address& address, const std::string& request, std::size_t freeSlots, Utils::InfoString& info);

    static Decision OnWaitingInfo(const Utils::InfoString& info, bool countFull);
    static void Start(const Network::Address& target);
    static void Stop(bool leave);
    static void SendHeartbeat();
    static void ClientFrame();
    static void DrawStatus();
    static bool IsPopupOpen();
  };
}
