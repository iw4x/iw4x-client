#pragma once

#include "../Types.hpp"

#include "../Support/FunctionRef.hpp"

#include "../Context.hpp"
#include "Connection.hpp"

namespace Controller
{
  class registry
  {
  public:
    explicit
    registry (const context&);

    device_id
    add (device_identity,
         Controller::transport_kind,
         connection link,
         capabilities,
         transport_binding);

    bool
    remove (device_id);

    std::optional<device_connection>
    find (device_id) const;

    void
    for_each (function_ref<void (const device_connection&)>) const;

    size_t
    size () const;

    uint64_t
    generation () const noexcept {return generation_.load ();}

  private:
    const context& ctx_;
    mutable std::mutex mutex_;
    uint32_t next_ {1};
    std::atomic<uint64_t> generation_ {0};
    std::vector<device_connection> devices_;
  };
}
