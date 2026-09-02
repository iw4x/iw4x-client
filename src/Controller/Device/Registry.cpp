#include "Registry.hpp"

#include "../Types.hpp"

#include <algorithm>

namespace Controller
{
  registry::
  registry (const context& ctx)
    : ctx_ (ctx)
  {
  }

  device_id
  registry::
  add (device_identity identity,
       Controller::transport_kind t,
       connection link,
       capabilities caps,
       transport_binding binding)
  {
    device_id id;

    {
      std::lock_guard<std::mutex> l (mutex_);

      for (device_connection& d: devices_)
      {
        if (same_binding (d.binding, binding))
        {
          d.identity = identity;
          d.transport = t;
          d.link = link;
          d.caps = caps;
          return d.id;
        }
      }

      id = device_id (next_++);
      devices_.push_back (
        device_connection {id, identity, t, link, caps, std::move (binding)});

      generation_.fetch_add (1);
    }

    ctx_.report (severity::info, facility::discovery, errc::none, id,
                 std::string ("device connected: ") + to_string (identity.family) +
                 " over " + to_string (t) + '/' + to_string (link));
    return id;
  }

  bool
  registry::
  remove (device_id id)
  {
    Controller::family family {Controller::family::unknown};

    {
      std::lock_guard<std::mutex> l (mutex_);

      auto i (std::find_if (devices_.begin (), devices_.end (),
                            [id] (const device_connection& d)
                            {
                              return d.id == id;
                            }));

      if (i == devices_.end ())
        return false;

      family = i->identity.family;
      devices_.erase (i);

      generation_.fetch_add (1);
    }

    ctx_.report (severity::info, facility::discovery, errc::none, id,
                 std::string ("device disconnected: ") + to_string (family));
    return true;
  }

  std::optional<device_connection>
  registry::
  find (device_id id) const
  {
    std::lock_guard<std::mutex> l (mutex_);

    for (const device_connection& d: devices_)
    {
      if (d.id == id)
        return d;
    }

    return std::nullopt;
  }

  void
  registry::
  for_each (function_ref<void (const device_connection&)> fn) const
  {
    std::lock_guard<std::mutex> l (mutex_);

    for (const device_connection& d: devices_)
      fn (d);
  }

  size_t
  registry::
  size () const
  {
    std::lock_guard<std::mutex> l (mutex_);
    return devices_.size ();
  }
}
