#include "DeviceNotify.hpp"

#include "../Types.hpp"

#include <windows.h>
#include <dbt.h>

namespace Controller
{
  namespace transport
  {
    namespace
    {
      constexpr wchar_t  window_class[] {L"iw4x_controller_devnotify"};
      constexpr UINT_PTR rescan_timer   {1};
      constexpr UINT_PTR retry_timer    {2};
      constexpr UINT     rescan_delay   {300};
      constexpr UINT     retry_delay    {2000};

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
              const DEV_BROADCAST_HDR* change (
                reinterpret_cast<const DEV_BROADCAST_HDR*> (lp));

              if ((wp == DBT_DEVICEARRIVAL || wp == DBT_DEVICEREMOVECOMPLETE) &&
                  change != nullptr &&
                  change->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
              {
                const UINT_PTR first (
                  SetTimer (w, rescan_timer, rescan_delay, nullptr));
                const UINT_PTR retry (
                  SetTimer (w, retry_timer, retry_delay, nullptr));

                if (first == 0 || retry == 0)
                {
                  if (std::atomic<bool>* f = flag_of (w))
                    f->store (true, std::memory_order_release);
                }
              }

              return TRUE;
            }

          case WM_TIMER:
					  {
							if (wp == rescan_timer || wp == retry_timer)
							{
								KillTimer (w, wp);

								if (std::atomic<bool>* f = flag_of (w))
									f->store (true, std::memory_order_release);

								return 0;
							}
						}
            break;

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
        failed_.store (true, std::memory_order_release);
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
        failed_.store (true, std::memory_order_release);
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

      DEV_BROADCAST_DEVICEINTERFACE_W filter {};
      filter.dbcc_size = sizeof (filter);
      filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

      HDEVNOTIFY notify (
        RegisterDeviceNotificationW (window, &filter,
                                     DEVICE_NOTIFY_WINDOW_HANDLE |
                                     DEVICE_NOTIFY_ALL_INTERFACE_CLASSES));

      if (notify == nullptr)
      {
        failed_.store (true, std::memory_order_release);
        ctx.report (severity::warning, facility::discovery, errc::transport_failure,
                    "device-change registration failed; discovery will poll");
      }

      pending_.store (true, std::memory_order_release);

      MSG m;
      while (GetMessageW (&m, nullptr, 0, 0) > 0)
      {
        TranslateMessage (&m);
        DispatchMessageW (&m);
      }

      if (!stop.stop_requested ())
        failed_.store (true, std::memory_order_release);

      if (notify != nullptr)
        UnregisterDeviceNotification (notify);
    }
  }
}
