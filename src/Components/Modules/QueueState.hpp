#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace Components
{
  /// Ordering core of the connect queue. It knows nothing about the engine:
  /// callers pass the current time and the number of free slots, so it can be
  /// exercised with a fake clock.
  class QueueState
  {
  public:
    struct Address
    {
      std::uint32_t ip;
      std::uint16_t port;

      bool operator==(const Address& other) const = default;
    };

    struct Config
    {
      std::size_t maxEntries = 10;
      std::size_t perIp = 2;
      std::uint32_t timeout = 20000;
      std::uint32_t reserveTime = 8000;
    };

    enum class Status
    {
      Queued,
      Full,
      IpLimit,
    };

    struct Ticket
    {
      Status status;
      std::size_t position;
      std::size_t length;
      bool admitted;
    };

    void configure(const Config& newConfig)
    {
      config = newConfig;
    }

    /// Adds the address at the back or refreshes it.
    Ticket heartbeat(const Address& address, std::uint32_t now, std::size_t freeSlots)
    {
      update(now, freeSlots);

      auto index = find(address);
      if (index == entries.size())
      {
        if (entries.size() >= config.maxEntries)
        {
          return {Status::Full, 0, entries.size(), false};
        }

        if (countIp(address.ip) >= config.perIp)
        {
          return {Status::IpLimit, 0, entries.size(), false};
        }

        entries.push_back({address, now, false, 0});
        update(now, freeSlots);
      }
      else
      {
        entries[index].lastSeen = now;
      }

      index = find(address);
      return {Status::Queued, index + 1, entries.size(), index < freeSlots};
    }

    void leave(const Address& address)
    {
      const auto index = find(address);
      if (index != entries.size())
      {
        entries.erase(entries.begin() + static_cast<std::ptrdiff_t>(index));
      }
    }

    /// Decides whether a verified connect attempt may reach the engine. It
    /// never creates an entry, and it only refuses while a free slot is held
    /// for someone else.
    bool connect(const Address& address, std::uint32_t now, std::size_t freeSlots)
    {
      if (freeSlots == 0)
      {
        return true;
      }

      update(now, freeSlots);

      const auto held = std::min(freeSlots, entries.size());
      const auto index = find(address);
      if (index < held)
      {
        entries.erase(entries.begin() + static_cast<std::ptrdiff_t>(index));
        return true;
      }

      return freeSlots > held;
    }

    /// Drops silent entries and admitted entries that missed their turn, then
    /// hands a reservation to the first freeSlots entries.
    void update(std::uint32_t now, std::size_t freeSlots)
    {
      std::erase_if(entries, [this, now](const Entry& entry)
      {
        const auto silent = now - entry.lastSeen > config.timeout;
        const auto missedTurn = entry.reserved && static_cast<std::int32_t>(now - entry.reservedUntil) > 0;
        return silent || missedTurn;
      });

      for (std::size_t i = 0; i < entries.size(); ++i)
      {
        auto& entry = entries[i];
        if (i < freeSlots)
        {
          if (!entry.reserved)
          {
            entry.reserved = true;
            entry.reservedUntil = now + config.reserveTime;
          }
        }
        else
        {
          entry.reserved = false;
        }
      }
    }

    [[nodiscard]] std::size_t size() const
    {
      return entries.size();
    }

  private:
    struct Entry
    {
      Address address;
      std::uint32_t lastSeen;
      bool reserved;
      std::uint32_t reservedUntil;
    };

    Config config;
    std::vector<Entry> entries;

    [[nodiscard]] std::size_t find(const Address& address) const
    {
      const auto itr = std::find_if(entries.begin(), entries.end(), [&address](const Entry& entry)
      {
        return entry.address == address;
      });

      return static_cast<std::size_t>(itr - entries.begin());
    }

    [[nodiscard]] std::size_t countIp(std::uint32_t ip) const
    {
      return static_cast<std::size_t>(std::count_if(entries.begin(), entries.end(), [ip](const Entry& entry)
      {
        return entry.address.ip == ip;
      }));
    }
  };
}
