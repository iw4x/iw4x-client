#include <Utils/InfoString.hpp>

#include "Queue.hpp"
#include "Auth.hpp"
#include "Bots.hpp"
#include "Dedicated.hpp"
#include "Events.hpp"
#include "Party.hpp"
#include "Renderer.hpp"

namespace Components
{
  namespace
  {
    constexpr int MAX_QUEUE_ENTRIES = 32;
    constexpr std::size_t HMAC_BLOCK_SIZE = 64;
    constexpr std::size_t COOKIE_LENGTH = 16;

    constexpr std::uint32_t HEARTBEAT_INTERVAL = 2000;
    constexpr std::uint32_t FRONT_HEARTBEAT_INTERVAL = 500;
    constexpr std::size_t FRONT_POSITIONS = 2;
  }

  Dvar::Var Queue::SvQueueMax;
  Dvar::Var Queue::SvQueuePerIp;
  Dvar::Var Queue::SvQueueTimeout;
  Dvar::Var Queue::SvQueueReserveTime;
  Dvar::Var Queue::ClQueue;
  Dvar::Var Queue::ClQueueTimeout;

  QueueState Queue::State;
  std::mutex Queue::StateMutex;
  Queue::Waiting Queue::Client;

  bool Queue::IsEnabled()
  {
    return SvQueueMax.get<int>() > 0;
  }

  std::uint32_t Queue::Now()
  {
    return static_cast<std::uint32_t>(Game::Sys_Milliseconds());
  }

  void Queue::ApplyConfig()
  {
    QueueState::Config config;
    config.maxEntries = std::min<std::size_t>(SvQueueMax.get<int>(), MAX_QUEUE_ENTRIES);
    config.perIp = SvQueuePerIp.get<int>();
    config.timeout = SvQueueTimeout.get<int>();
    config.reserveTime = SvQueueReserveTime.get<int>();
    State.configure(config);
  }

  QueueState::Address Queue::ToStateAddress(const Network::Address& address)
  {
    return {address.getIP().full, address.getPort()};
  }

  /// Players who know the private password skip the queue, as they skip the
  /// full check. Without a private password the engine treats everyone as
  /// privileged, which must not switch the queue off.
  bool Queue::HasPrivateSlotAccess(const char* password)
  {
    return *(*Game::sv_privatePassword)->current.string && Auth::HasReservedSlotAccess(password);
  }

  std::string Queue::Hmac(const std::string& key, const std::string& message)
  {
    std::string inner(HMAC_BLOCK_SIZE, '\0');
    std::string outer(HMAC_BLOCK_SIZE, '\0');

    for (std::size_t i = 0; i < HMAC_BLOCK_SIZE; ++i)
    {
      const auto byte = i < key.size() ? key[i] : '\0';
      inner[i] = static_cast<char>(byte ^ 0x36);
      outer[i] = static_cast<char>(byte ^ 0x5C);
    }

    return Utils::Cryptography::SHA256::Compute(outer + Utils::Cryptography::SHA256::Compute(inner + message), true);
  }

  /// Proves that the client can read replies sent to its address, so spoofed
  /// sources cannot take places in the queue.
  std::string Queue::MakeCookie(const Network::Address& address)
  {
    static const std::string secret = []
    {
      std::string value;
      for (auto i = 0; i < 4; ++i)
      {
        const auto part = Utils::Cryptography::Rand::GenerateLong();
        value.append(reinterpret_cast<const char*>(&part), sizeof(part));
      }

      return value;
    }();

    return Hmac(secret, address.getString()).substr(0, COOKIE_LENGTH);
  }

  std::vector<std::string> Queue::ParseRequest(const std::string& request)
  {
    auto tokens = Utils::String::Split(request.substr(0, request.find('\n')), ' ');
    std::erase_if(tokens, [](const std::string& token)
    {
      return token.empty();
    });

    return tokens;
  }

  bool Queue::AllowConnect(const Network::Address& address, const char* password)
  {
    if (!IsEnabled() || address.isLoopback() || HasPrivateSlotAccess(password))
    {
      return true;
    }

    const auto freeSlots = Bots::FreeSlots();

    std::lock_guard lock(StateMutex);
    ApplyConfig();

    const auto allowed = State.connect(ToStateAddress(address), Now(), freeSlots);
    if (!allowed)
    {
      Logger::Debug("Queue: refused {}, {} free slot(s) held for waiting players", address.getString(), freeSlots);
    }

    return allowed;
  }

  void Queue::AddInfoKeys(const Network::Address& address, const std::string& request, Utils::InfoString& info)
  {
    if (!IsEnabled())
    {
      return;
    }

    const auto freeSlots = Bots::FreeSlots();

    info.set("queue", "1");
    info.set("freeSlots", std::to_string(freeSlots));

    if (Bots::HasPrivateSlots())
    {
      info.set("freeSlotsPrivileged", std::to_string(Bots::FreeSlots(true)));
    }

    std::lock_guard lock(StateMutex);
    ApplyConfig();

    HandleHeartbeat(address, request, freeSlots, info);
    info.set("queueLength", std::to_string(State.size()));
  }

  /// Handles the queue tokens of a getinfo request: a first request is
  /// answered with a cookie, and a request that returns it joins, refreshes or
  /// leaves the queue.
  void Queue::HandleHeartbeat(const Network::Address& address, const std::string& request, std::size_t freeSlots, Utils::InfoString& info)
  {
    const auto tokens = ParseRequest(request);
    if (tokens.size() < 2 || (tokens[1] != "q" && tokens[1] != "qc") || address.isLoopback())
    {
      return;
    }

    const auto cookie = MakeCookie(address);
    if (tokens.size() < 3 || !Utils::Cryptography::CompareConstantTime(tokens[2], cookie))
    {
      info.set("queueCookie", cookie);
      return;
    }

    if (tokens[1] == "qc")
    {
      State.leave(ToStateAddress(address));
      return;
    }

    const auto ticket = State.heartbeat(ToStateAddress(address), Now(), freeSlots);
    if (ticket.status != QueueState::Status::Queued)
    {
      info.set("queueRejected", "1");
      return;
    }

    info.set("queuePosition", std::to_string(ticket.position));
    info.set("queueAdmit", ticket.admitted ? "1" : "0");
  }

  void Queue::ServerFrame()
  {
    if (!Dedicated::IsRunning() || !IsEnabled())
    {
      return;
    }

    const auto freeSlots = Bots::FreeSlots();

    std::lock_guard lock(StateMutex);
    ApplyConfig();
    State.update(Now(), freeSlots);
  }

  /// A free slot only counts when it is not held for the queue. Players who
  /// set a password may hold the private one, so a slot reserved by
  /// sv_privateClients counts for them and they try to connect.
  bool Queue::IsFull(const Utils::InfoString& info, bool countFull)
  {
    const auto freeSlots = info.get("freeSlots");
    if (freeSlots.empty())
    {
      return countFull;
    }

    const auto available = std::strtoul(freeSlots.data(), nullptr, 10);
    const auto held = std::strtoul(info.get("queueLength").data(), nullptr, 10);
    const auto privileged = std::strtoul(info.get("freeSlotsPrivileged").data(), nullptr, 10);

    if (privileged > available && !Dvar::Var("password").get<std::string>().empty())
    {
      return false;
    }

    return available <= held;
  }

  Queue::Decision Queue::OnServerInfo(const Network::Address& target, const Utils::InfoString& info, bool countFull)
  {
    if (Client.active && Client.target == target)
    {
      return OnWaitingInfo(info, countFull);
    }

    if (!IsFull(info, countFull))
    {
      return Decision::Connect;
    }

    if (!ClQueue.get<bool>() || info.get("queue") != "1")
    {
      return Decision::Full;
    }

    Start(target);
    return Decision::Wait;
  }

  Queue::Decision Queue::OnWaitingInfo(const Utils::InfoString& info, bool countFull)
  {
    Client.lastRequest = Now();

    if (info.get("queue") != "1" || !info.get("queueRejected").empty())
    {
      Stop(false);
      return Decision::Full;
    }

    const auto cookie = info.get("queueCookie");
    if (!cookie.empty())
    {
      Client.cookie = cookie;
      SendHeartbeat();
      return Decision::Wait;
    }

    if (info.get("queueAdmit") == "1")
    {
      Stop(false);
      return Decision::Connect;
    }

    const auto position = info.get("queuePosition");
    if (position.empty())
    {
      Stop(false);
      return countFull ? Decision::Full : Decision::Connect;
    }

    const auto newPosition = std::strtoul(position.data(), nullptr, 10);
    if (newPosition != Client.position)
    {
      Logger::Print("Server queue: position {} of {}\n", newPosition, info.get("queueLength"));
    }

    Client.position = newPosition;
    Client.status = std::format("In queue: position {} of {}", newPosition, info.get("queueLength"));

    return Decision::Wait;
  }

  void Queue::Start(const Network::Address& target)
  {
    Client = {};
    Client.active = true;
    Client.target = target;
    Client.startedAt = Now();
    Client.lastRequest = Client.startedAt;

    Logger::Print("Server is full, waiting in the queue\n");
    SendHeartbeat();
  }

  void Queue::Stop(bool leave)
  {
    if (!Client.active)
    {
      return;
    }

    if (leave)
    {
      Party::CancelInfoRequest();

      if (!Client.cookie.empty())
      {
        Network::SendCommand(Client.target, "getinfo", Utils::Cryptography::Rand::GenerateChallenge() + " qc " + Client.cookie);
      }
    }

    Client = {};
  }

  void Queue::Cancel()
  {
    Stop(true);
  }

  void Queue::SendHeartbeat()
  {
    Party::SendInfoRequest(Client.cookie.empty() ? " q" : " q " + Client.cookie);
  }

  bool Queue::IsPopupOpen()
  {
    auto* menu = Game::Menus_FindByName(Game::uiContext, "popup_reconnectingtoparty");
    return menu && Game::Menus_MenuIsInStack(Game::uiContext, menu);
  }

  void Queue::ClientFrame()
  {
    if (!Client.active)
    {
      return;
    }

    const auto now = Now();
    const auto timeout = static_cast<std::uint32_t>(ClQueueTimeout.get<int>());

    if (timeout && now - Client.startedAt > timeout)
    {
      Stop(true);
      Party::ConnectError("@EXE_SERVERISFULL");
      return;
    }

    if (IsPopupOpen())
    {
      Client.popupSeen = true;
    }
    else if (Client.popupSeen)
    {
      Stop(true);
      return;
    }

    const auto interval = Client.position && Client.position <= FRONT_POSITIONS ? FRONT_HEARTBEAT_INTERVAL : HEARTBEAT_INTERVAL;
    if (now - Client.lastRequest >= interval)
    {
      Client.lastRequest = now;
      SendHeartbeat();
    }
  }

  /// Draws the place in the queue inside the "connecting" popup, which has no
  /// room for it in its menu definition.
  void Queue::DrawStatus()
  {
    if (!Client.active || Client.status.empty())
    {
      return;
    }

    auto* font = Game::R_RegisterFont("fonts/smallFont", 0);
    const auto scale = std::max(1.0f, Renderer::Height() / 720.0f);
    const auto width = static_cast<float>(Game::R_TextWidth(Client.status.data(), std::numeric_limits<int>::max(), font)) * scale;

    float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    Game::R_AddCmdDrawText(Client.status.data(), std::numeric_limits<int>::max(), font, (static_cast<float>(Renderer::Width()) - width) / 2.0f, Renderer::Height() * 0.49f, scale, scale, 0.0f, color, Game::ITEM_TEXTSTYLE_SHADOWED);
  }

  Queue::Queue()
  {
    Events::OnDvarInit([]
    {
      SvQueueMax = Dvar::Register<int>("sv_queueMax", 10, 0, MAX_QUEUE_ENTRIES, Game::DVAR_NONE, "Players that can wait for a slot on a full server, 0 disables the queue");
      SvQueuePerIp = Dvar::Register<int>("sv_queuePerIp", 2, 1, MAX_QUEUE_ENTRIES, Game::DVAR_NONE, "Queue places a single IP address can hold");
      SvQueueTimeout = Dvar::Register<int>("sv_queueTimeout", 20000, 1000, 600000, Game::DVAR_NONE, "Milliseconds without a heartbeat before a waiting player is dropped");
      SvQueueReserveTime = Dvar::Register<int>("sv_queueReserveTime", 8000, 1000, 60000, Game::DVAR_NONE, "Milliseconds a freed slot is held for the player whose turn it is");
      ClQueue = Dvar::Register<bool>("cl_queue", true, Game::DVAR_ARCHIVE, "Wait in the queue when the server is full");
      ClQueueTimeout = Dvar::Register<int>("cl_queueTimeout", 900000, 0, 3600000, Game::DVAR_ARCHIVE, "Milliseconds to wait in a server queue before giving up, 0 waits forever");
    });

    Scheduler::Loop(ServerFrame, Scheduler::Pipeline::SERVER);

    if (!Dedicated::IsEnabled() && !ZoneBuilder::IsEnabled())
    {
      Scheduler::Loop(ClientFrame, Scheduler::Pipeline::CLIENT);
      Scheduler::Loop(DrawStatus, Scheduler::Pipeline::RENDERER);
    }
  }
}
