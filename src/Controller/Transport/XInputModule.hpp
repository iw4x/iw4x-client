#pragma once

#include "../Types.hpp"

#include "../Context.hpp"

#include <windows.h>
#include <xinput.h>

namespace Controller
{
  namespace transport
  {
    class xinput_module
    {
    public:
      explicit
      xinput_module (const context&);

      ~xinput_module ();

      xinput_module (const xinput_module&) = delete;
      xinput_module& operator= (const xinput_module&) = delete;
      xinput_module (xinput_module&&) = delete;
      xinput_module& operator= (xinput_module&&) = delete;

      bool
      loaded () const noexcept {return handle_ != nullptr;}

      const char*
      dll_name () const noexcept {return name_;}

      bool
      has_guide_button () const noexcept {return get_state_ex_ != nullptr;}

      DWORD
      get_state (DWORD user_index, XINPUT_STATE& out) const noexcept;

      DWORD
      get_capabilities (DWORD user_index,
                        DWORD flags,
                        XINPUT_CAPABILITIES& out) const noexcept;

      DWORD
      set_state (DWORD user_index, XINPUT_VIBRATION& vibration) const noexcept;

    private:
      using get_state_fn = DWORD (WINAPI*) (DWORD, XINPUT_STATE*);
      using get_caps_fn  = DWORD (WINAPI*) (DWORD, DWORD, XINPUT_CAPABILITIES*);
      using set_state_fn = DWORD (WINAPI*) (DWORD, XINPUT_VIBRATION*);

      void
      load (const context&);

      HMODULE handle_ {nullptr};
      const char* name_ {"none"};

      get_state_fn get_state_ {nullptr};
      get_state_fn get_state_ex_ {nullptr};
      get_caps_fn get_capabilities_ {nullptr};
      set_state_fn set_state_ {nullptr};
    };
  }
}
