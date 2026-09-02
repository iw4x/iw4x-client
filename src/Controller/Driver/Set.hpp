#pragma once

#include "../Types.hpp"

#include "../Support/FunctionRef.hpp"

#include "../Context.hpp"
#include "../Device/Registry.hpp"
#include "Driver.hpp"
#include "../Transport/Hid.hpp"
#include "../Transport/XInputModule.hpp"

namespace Controller
{
  namespace driver
  {
    class set
    {
    public:
      set (const context&, const transport::xinput_module&);

      ~set ();

      set (const set&) = delete;
      set& operator= (const set&) = delete;

      void
      reconcile (const registry&);

      void
      for_each (function_ref<void (driver&, const device_connection&)>);

      void
      submit (device_id, const output_request&);

      size_t
      size () const noexcept {return entries_.size ();}

      void
      configure (const output_policy&);

      void
      stop_haptic (device_id, uint32_t tag);

      std::string
      diagnostics (device_id) const;

    private:
      struct entry
      {
        device_connection device;
        std::unique_ptr<transport::hid_device> hid;
        std::unique_ptr<driver> drv;
      };

      std::unique_ptr<driver>
      bind (const device_connection&, std::unique_ptr<transport::hid_device>&);

      const context& ctx_;
      const transport::xinput_module& xinput_;

      uint64_t generation_ {0};
      bool reconciled_ {false};
      std::vector<entry> entries_;
    };
  }
}
