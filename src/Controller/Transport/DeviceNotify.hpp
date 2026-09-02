#pragma once

#include "../Types.hpp"

#include <thread>
#include <atomic>

#include "../Context.hpp"

namespace Controller
{
  namespace transport
  {
    class device_notifier
    {
    public:
      explicit
      device_notifier (const context&);

      device_notifier (const device_notifier&) = delete;
      device_notifier& operator= (const device_notifier&) = delete;

      bool
      consume () noexcept {return pending_.exchange (false, std::memory_order_acquire);}

    private:
      void
      run (std::stop_token, context) noexcept;

      std::atomic<bool> pending_ {false};

      std::jthread thread_;
    };
  }
}
