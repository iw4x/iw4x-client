#include "Presence.hpp"
#include "Auth.hpp"

#include <Utils/WebIO.hpp>

namespace Components
{
  // Tell the master server which server we are playing on.
  //
  // A server reports its own player count and so can report whatever it
  // likes. Some do, so the master server counts the players itself from what
  // the clients report. We sign each report with our key, the one we connect
  // to servers with, so that it can tell players apart and only count those
  // with a high enough security level.
  //
  namespace
  {
    constexpr auto* MASTER_URL = "http://master.iw4x.io";
    constexpr auto REPORT_INTERVAL = 60s;

    std::atomic_bool reporting;
  }

  void Presence::Report(const std::string& server)
  {
    // Get a nonce first. The master server only accepts each one once, so
    // our report can't be replayed.
    //
    const auto reply = Utils::WebIO("IW4x", std::format("{}/v1/presence/challenge", MASTER_URL)).setTimeout(5000)->setMaxResponseSize(1024)->setFollowRedirects(false)->get();

    const auto challenge = nlohmann::json::parse(reply, nullptr, false);
    if (!challenge.is_object() || !challenge.contains("nonce") || !challenge["nonce"].is_string())
    {
      return;
    }

    // We send the nonce back as is, so make sure it's what we expect.
    //
    const auto nonce = challenge["nonce"].get<std::string>();
    if (nonce.empty() || nonce.size() > 64 || !std::ranges::all_of(nonce, [](const unsigned char c) { return std::isxdigit(c) != 0; }))
    {
      return;
    }

    std::string publicKey, token, signature;
    if (!Auth::SignForMaster(std::format("iw4x-presence\n{}\n{}", nonce, server), &publicKey, &token, &signature))
    {
      return;
    }

    // Note that the values aren't URL-encoded, which is fine since they are
    // all either hex or an address.
    //
    const Utils::WebIO::params report =
    {
      {"key", Utils::String::DumpHex(publicKey, "")},
      {"token", Utils::String::DumpHex(token, "")},
      {"server", server},
      {"nonce", nonce},
      {"signature", Utils::String::DumpHex(signature, "")},
    };

    Utils::WebIO("IW4x", std::format("{}/v1/presence", MASTER_URL)).setTimeout(5000)->setMaxResponseSize(1024)->setFollowRedirects(false)->post(report);
  }

  Presence::Presence()
  {
    if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())
    {
      return;
    }

    Scheduler::Loop([]
    {
      if (Game::CL_GetLocalClientConnectionState(0) != Game::CA_ACTIVE)
      {
        return;
      }

      // Only report servers the master server can list. Note that it knows
      // them by the address they send their info from, which is the one we
      // connect to.
      //
      const Network::Address server(&Game::clientConnections->serverAddress);
      if (server.getType() != Game::NA_IP || server.isLocal() || server.isLoopback())
      {
        return;
      }

      // The report blocks on the network, so make it on the async pipeline.
      // Skip this round if the previous one is still going.
      //
      if (reporting.exchange(true))
      {
        return;
      }

      Scheduler::Once([address = server.getString()]
      {
        Report(address);
        reporting = false;
      }, Scheduler::Pipeline::ASYNC);
    }, Scheduler::Pipeline::CLIENT, REPORT_INTERVAL);
  }
}
