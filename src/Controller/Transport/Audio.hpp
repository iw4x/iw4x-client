#pragma once

#include "../Types.hpp"

#include "../Context.hpp"
#include "../Device/Id.hpp"
#include "../Haptic/Effect.hpp"

namespace Controller
{
  namespace transport
  {
    class audio_endpoint
    {
    public:
      using source = std::function<void (std::span<haptic::frame>)>;

      audio_endpoint (const context&, device_id, const std::wstring& hid_path,
                      source);

      audio_endpoint (const audio_endpoint&) = delete;
      audio_endpoint& operator= (const audio_endpoint&) = delete;

      bool
      running () const noexcept {return running_.load (std::memory_order_acquire);}

      uint32_t
      sample_rate () const noexcept {return rate_.load (std::memory_order_acquire);}

      std::string
      status () const;

    private:
      void
      run (std::stop_token, context, device_id, std::wstring) noexcept;

      void
      note (std::string) const;

      source source_;

      std::atomic<uint32_t> rate_ {0};
      std::atomic<bool> running_ {false};

      mutable std::mutex status_mutex_;
      mutable std::string status_ {"not started"};

      std::jthread thread_;
    };
  }
}
