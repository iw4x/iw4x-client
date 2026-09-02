#include "DeviceNotify.hpp"

#include "../Types.hpp"

#include <windows.h>
#include <dbt.h>
#include <hidsdi.h>

namespace Controller
{
  namespace transport
  {
    namespace
    {
      constexpr wchar_t window_class[] {L"iw4x_controller_devnotify"};

      std::atomic<bool>*
      flag_of (HWND w) noexcept
      {
        return reinterpret_cast<std::atomic<bool>*> (
          GetWindowLongPtrW (w, GWLP_USERDATA));
      }

      LRESULT CALLBACK
      window_proc (HWND w, UINT msg, WPARAM wp, LPARAM lp) noexcept
      {
        switch (msg)
        {
          case WM_DEVICECHANGE:
            {
              if (wp == DBT_DEVICEARRIVAL || wp == DBT_DEVICEREMOVECOMPLETE)
              {
                if (std::atomic<bool>* f = flag_of (w))
                  f->store (true, std::memory_order_release);
              }

              return TRUE;
            }

          case WM_CLOSE:
            DestroyWindow (w);
            return 0;

          case WM_DESTROY:
            PostQuitMessage (0);
            return 0;
        }

        return DefWindowProcW (w, msg, wp, lp);
      }
    }

    device_notifier::
    device_notifier (const context& ctx)
      : thread_ ([this, ctx] (std::stop_token t) {run (std::move (t), ctx);})
    {
    }

    void
    device_notifier::
    run (std::stop_token stop, context ctx) noexcept
    {
      HINSTANCE instance (GetModuleHandleW (nullptr));

      WNDCLASSEXW wc {};
      wc.cbSize = sizeof (wc);
      wc.lpfnWndProc = &window_proc;
      wc.hInstance = instance;
      wc.lpszClassName = window_class;

      if (RegisterClassExW (&wc) == 0 &&
          GetLastError () != ERROR_CLASS_ALREADY_EXISTS)
      {
        ctx.report (severity::warning, facility::discovery, errc::transport_failure,
                    "device-change window class registration failed; discovery "
                    "will poll");
        return;
      }

      HWND window (CreateWindowExW (0, window_class, window_class, 0,
                                   0, 0, 0, 0,
                                   HWND_MESSAGE, nullptr, instance, nullptr));

      if (window == nullptr)
      {
        ctx.report (severity::warning, facility::discovery, errc::transport_failure,
                    "device-change window creation failed; discovery will poll");
        return;
      }

      SetWindowLongPtrW (window, GWLP_USERDATA,
                         reinterpret_cast<LONG_PTR> (&pending_));

      const std::stop_callback wake (stop, [window] () noexcept
      {
        PostMessageW (window, WM_CLOSE, 0, 0);
      });

      GUID hid_guid {};
      HidD_GetHidGuid (&hid_guid);

      DEV_BROADCAST_DEVICEINTERFACE_W filter {};
      filter.dbcc_size = sizeof (filter);
      filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
      filter.dbcc_classguid = hid_guid;

      HDEVNOTIFY notify (
        RegisterDeviceNotificationW (window, &filter,
                                     DEVICE_NOTIFY_WINDOW_HANDLE));

      if (notify == nullptr)
        ctx.report (severity::warning, facility::discovery, errc::transport_failure,
                    "HID device-change registration failed; discovery will poll");

      MSG m;
      while (GetMessageW (&m, nullptr, 0, 0) > 0)
      {
        TranslateMessage (&m);
        DispatchMessageW (&m);
      }

      if (notify != nullptr)
        UnregisterDeviceNotification (notify);
    }
  }
}
