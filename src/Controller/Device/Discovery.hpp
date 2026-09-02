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

    void
    scan ();

    void
    scan_now ();

    static constexpr clock::duration interval {
      std::chrono::milliseconds (1000)};

  private:
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
  };
}
