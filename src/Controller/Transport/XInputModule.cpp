#include "XInputModule.hpp"

#include "../Types.hpp"

namespace Controller
{
  namespace transport
  {
    xinput_module::
    xinput_module (const context& ctx)
    {
      load (ctx);
    }

    xinput_module::
    ~xinput_module ()
    {
      if (handle_ != nullptr)
        FreeLibrary (handle_);
    }

    void
    xinput_module::
    load (const context& ctx)
    {
      static constexpr const char* variants[]
      {
        "xinput1_4.dll",
        "xinput1_3.dll",
        "xinput9_1_0.dll",
      };

      for (const char* name: variants)
      {
        HMODULE h (LoadLibraryA (name));

        if (h == nullptr)
          continue;

        auto get_state (reinterpret_cast<get_state_fn> (
          GetProcAddress (h, "XInputGetState")));
        auto get_caps (reinterpret_cast<get_caps_fn> (
          GetProcAddress (h, "XInputGetCapabilities")));
        auto set_state (reinterpret_cast<set_state_fn> (
          GetProcAddress (h, "XInputSetState")));

        if (get_state == nullptr || get_caps == nullptr || set_state == nullptr)
        {
          FreeLibrary (h);
          continue;
        }

        auto get_state_ex (reinterpret_cast<get_state_fn> (
          GetProcAddress (h, reinterpret_cast<const char*> (100))));

        handle_ = h;
        name_ = name;
        get_state_ = get_state;
        get_state_ex_ = get_state_ex;
        get_capabilities_ = get_caps;
        set_state_ = set_state;

        ctx.report (severity::info, facility::transport, errc::none,
                    std::string ("XInput loaded via ") + name +
                    (get_state_ex != nullptr
                     ? " (guide button available)"
                     : " (no guide button)"));
        return;
      }

      ctx.report (severity::warning, facility::transport, errc::transport_failure,
                  "no XInput runtime could be loaded; "
                  "XInput controllers will not be available");
    }

    DWORD
    xinput_module::
    get_state (DWORD user_index, XINPUT_STATE& out) const noexcept
    {
      if (get_state_ex_ != nullptr)
        return get_state_ex_ (user_index, &out);

      if (get_state_ != nullptr)
        return get_state_ (user_index, &out);

      return ERROR_DEVICE_NOT_CONNECTED;
    }

    DWORD
    xinput_module::
    get_capabilities (DWORD user_index,
                      DWORD flags,
                      XINPUT_CAPABILITIES& out) const noexcept
    {
      if (get_capabilities_ != nullptr)
        return get_capabilities_ (user_index, flags, &out);

      return ERROR_DEVICE_NOT_CONNECTED;
    }

    DWORD
    xinput_module::
    set_state (DWORD user_index, XINPUT_VIBRATION& vibration) const noexcept
    {
      if (set_state_ != nullptr)
        return set_state_ (user_index, &vibration);

      return ERROR_DEVICE_NOT_CONNECTED;
    }
  }
}
