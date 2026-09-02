#pragma once

#include "../Types.hpp"

#include "../Context.hpp"
#include "../Device/Id.hpp"
#include "Hid.hpp"

namespace Controller
{
  namespace transport
  {
    class report_stream
    {
    public:
      using producer =
        std::function<std::optional<size_t> (std::span<std::byte>)>;

      report_stream (const context&,
                     device_id,
                     hid_device&,
                     std::chrono::nanoseconds period,
                     size_t capacity,
                     producer);

      report_stream (const report_stream&) = delete;
      report_stream& operator= (const report_stream&) = delete;

      bool
      running () const noexcept {return running_.load (std::memory_order_acquire);}

      std::string
      status () const;

    private:
      void
      run (std::stop_token, context, device_id) noexcept;

      void
      note (std::string) const;

      hid_device& hid_;
      producer produce_;
      std::chrono::nanoseconds period_;
      size_t capacity_;

      std::atomic<bool> running_ {false};

      mutable std::mutex status_mutex_;
      mutable std::string status_ {"not started"};

      std::jthread thread_;
    };
  }
}
