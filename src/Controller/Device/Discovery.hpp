#pragma once

#include "../Types.hpp"

#include "../Clock.hpp"
#include "../Context.hpp"
#include "Registry.hpp"
#include "../Transport/XInputModule.hpp"
#include "../Transport/DeviceNotify.hpp"

namespace Controller
{
  class discovery
  {
  public:
    discovery (const context&, registry&, const transport::xinput_module&);

    discovery (const discovery&) = delete;
    discovery& operator= (const discovery&) = delete;

    void
    scan ();

  private:
    void
    scan_now ();

    void
    run (std::stop_token) noexcept;

    void
    scan_xinput (std::vector<transport_binding>& seen);

    void
    scan_hid (std::vector<transport_binding>& seen);

    void
    retire_unseen (const std::vector<transport_binding>& seen);

    const context& ctx_;
    registry& registry_;
    const transport::xinput_module& xinput_;

    transport::device_notifier notifier_;

    bool scanned_ {false};
    timestamp last_scan_ {};

    std::vector<std::wstring> unbound_ {};

    std::atomic<bool> pending_ {false};

    std::jthread thread_;
  };
}
