#include "Set.hpp"

#include "../Types.hpp"

#include <cassert>
#include <variant>
#include <algorithm>

#include "XInput.hpp"
#include "DualShock4.hpp"
#include "DualSense.hpp"
#include "DualSenseEdge.hpp"
#include "PlayStation.hpp"

namespace Controller
{
  namespace driver
  {
    set::
    set (const context& ctx, const transport::xinput_module& x)
      : ctx_ (ctx), xinput_ (x)
    {
    }

    set::
    ~set () = default;

    std::unique_ptr<driver>
    set::
    bind (const device_connection& d, std::unique_ptr<transport::hid_device>& hid)
    {
      switch (d.identity.family)
      {
        case family::xbox:
          {
            const auto* b (std::get_if<xinput_binding> (&d.binding));

            if (b == nullptr)
              break;

            return std::make_unique<xinput_driver> (ctx_, xinput_, d.id, b->index);
          }

        case family::dualshock4:
        case family::dualsense:
        case family::dualsense_edge:
          {
            if (hid == nullptr)
              break;

            assert (hid->link () == connection::usb ||
                    hid->link () == connection::bluetooth);

            if (hid->link () == connection::bluetooth)
              enable_extended_reports (ctx_, *hid, d.id);

            switch (d.identity.family)
            {
              case family::dualshock4:
                return std::make_unique<dualshock4_driver> (ctx_, *hid, d.id);

              case family::dualsense:
                return std::make_unique<dualsense_driver> (ctx_, *hid, d.id);

              case family::dualsense_edge:
                return std::make_unique<dualsense_edge_driver> (ctx_, *hid, d.id);

              default:
                break;
            }

            break;
          }

        case family::unknown:
          break;
      }

      return nullptr;
    }

    void
    set::
    reconcile (const registry& r)
    {
      const uint64_t g (r.generation ());

      if (reconciled_ && g == generation_)
        return;

      generation_ = g;
      reconciled_ = true;

      std::vector<device_connection> current;
      r.for_each ([&current] (const device_connection& d) {current.push_back (d);});

      std::erase_if (entries_, [this, &current] (const entry& e)
      {
        const bool gone (
          std::none_of (current.begin (), current.end (),
                        [&e] (const device_connection& d) {return d.id == e.device.id;}));

        if (gone)
          ctx_.report (severity::info, facility::driver, errc::none, e.device.id,
                       std::string ("driver released: ") +
                       to_string (e.device.identity.family));

        return gone;
      });

      for (device_connection& d: current)
      {
        const bool bound (
          std::any_of (entries_.begin (), entries_.end (),
                       [&d] (const entry& e) {return e.device.id == d.id;}));

        if (bound)
          continue;

        std::unique_ptr<transport::hid_device> hid;

        if (const auto* b = std::get_if<hid_binding> (&d.binding))
        {
          hid = transport::open (ctx_, b->path);

          if (hid == nullptr)
            continue;
        }

        std::unique_ptr<driver> drv (bind (d, hid));

        if (drv == nullptr)
        {
          ctx_.report (severity::warning, facility::driver, errc::unsupported_device,
                       d.id,
                       std::string ("no driver binds ") + to_string (d.identity.family) +
                       " over " + to_string (d.transport));
          continue;
        }

        ctx_.report (severity::info, facility::driver, errc::none, d.id,
                     std::string ("driver bound: ") + to_string (d.identity.family) +
                     " over " + to_string (d.transport));

        entries_.push_back (entry {std::move (d), std::move (hid), std::move (drv)});
      }
    }

    void
    set::
    for_each (function_ref<void (driver&, const device_connection&)> fn)
    {
      for (entry& e: entries_)
        fn (*e.drv, e.device);
    }

    void
    set::
    configure (const output_policy& p)
    {
      for (entry& e: entries_)
      {
        if (e.drv != nullptr)
          e.drv->configure (p);
      }
    }

    void
    set::
    stop_haptic (device_id id, uint32_t tag)
    {
      for (entry& e: entries_)
      {
        if (e.device.id == id && e.drv != nullptr)
          e.drv->stop_haptic (tag);
      }
    }

    std::string
    set::
    diagnostics (device_id id) const
    {
      for (const entry& e: entries_)
      {
        if (e.device.id == id && e.drv != nullptr)
          return e.drv->diagnostics ();
      }

      return {};
    }

    void
    set::
    submit (device_id id, const output_request& request)
    {
      for (entry& e: entries_)
      {
        if (e.device.id == id)
        {
          e.drv->submit (request);
          return;
        }
      }
    }
  }
}
