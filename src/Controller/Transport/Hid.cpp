#include "Hid.hpp"

#include "../Types.hpp"

#include <cassert>
#include <cstring>
#include <algorithm>

#include <windows.h>
#include <hidsdi.h>
#include <setupapi.h>

namespace Controller
{
  namespace transport
  {
    namespace
    {
      constexpr size_t max_report_size {128};

      constexpr size_t usb_input_report_length {64};
      constexpr size_t bluetooth_input_report_length {78};
    }

    connection
    classify_link (size_t input_length) noexcept
    {
      if (input_length == bluetooth_input_report_length)
        return connection::bluetooth;

      if (input_length == usb_input_report_length)
        return connection::usb;

      return connection::unknown;
    }

    namespace
    {
      class windows_hid_device: public hid_device
      {
      public:
        windows_hid_device (HANDLE handle,
                            std::wstring path,
                            hid_attributes attributes,
                            connection link,
                            size_t input_length,
                            size_t feature_length) noexcept
          : handle_ (handle),
            path_ (std::move (path)),
            attributes_ (attributes),
            link_ (link),
            input_length_ (std::min (input_length, max_report_size)),
            feature_length_ (std::min (feature_length, max_report_size))
        {
          overlapped_.hEvent = CreateEventW (nullptr, TRUE, FALSE, nullptr);
        }

        ~windows_hid_device () override
        {
          if (read_pending_)
          {
            CancelIo (handle_);

            DWORD n (0);
            GetOverlappedResult (handle_, &overlapped_, &n, TRUE);
          }

          if (overlapped_.hEvent != nullptr)
            CloseHandle (overlapped_.hEvent);

          if (handle_ != INVALID_HANDLE_VALUE)
            CloseHandle (handle_);
        }

        windows_hid_device (const windows_hid_device&) = delete;
        windows_hid_device& operator= (const windows_hid_device&) = delete;

        connection
        link () const noexcept override {return link_;}

        hid_attributes
        attributes () const noexcept override {return attributes_;}

        const std::wstring&
        path () const noexcept override {return path_;}

        size_t
        feature_report_length () const noexcept override
        {
          return feature_length_;
        }

        std::optional<size_t>
        read (std::span<std::byte> out) noexcept override
        {
          if (overlapped_.hEvent == nullptr)
            return std::nullopt;

          if (!read_pending_ && !issue_read ())
            return std::nullopt;

          DWORD n (0);

          if (!GetOverlappedResult (handle_, &overlapped_, &n, FALSE))
          {
            if (GetLastError () == ERROR_IO_INCOMPLETE)
              return size_t (0);

            read_pending_ = false;
            return std::nullopt;
          }

          read_pending_ = false;

          const size_t count (std::min (static_cast<size_t> (n), out.size ()));
          std::memcpy (out.data (), buf_.data (), count);

          issue_read ();

          return count;
        }

        std::optional<size_t>
        write (std::span<const std::byte> buf) noexcept override
        {
          if (!writable_ || buf.empty ())
            return std::nullopt;

          OVERLAPPED ov {};
          ov.hEvent = CreateEventW (nullptr, TRUE, FALSE, nullptr);

          if (ov.hEvent == nullptr)
            return std::nullopt;

          std::optional<size_t> result;
          DWORD n (0);

          if (WriteFile (handle_,
                         buf.data (),
                         static_cast<DWORD> (buf.size ()),
                         &n,
                         &ov) ||
              (GetLastError () == ERROR_IO_PENDING &&
               GetOverlappedResult (handle_, &ov, &n, TRUE)))
            result = static_cast<size_t> (n);

          CloseHandle (ov.hEvent);
          return result;
        }

        bool
        get_feature (std::span<std::byte> buf) noexcept override
        {
          return !buf.empty () && buf.size () >= feature_length_ &&
            HidD_GetFeature (handle_,
                             buf.data (),
                             static_cast<ULONG> (buf.size ())) != FALSE;
        }

        bool
        set_feature (std::span<const std::byte> buf) noexcept override
        {
          return writable_ && !buf.empty () &&
            HidD_SetFeature (handle_,
                             const_cast<std::byte*> (buf.data ()),
                             static_cast<ULONG> (buf.size ())) != FALSE;
        }

        void
        writable (bool w) noexcept {writable_ = w;}

      private:
        bool
        issue_read () noexcept
        {
          assert (!read_pending_);

          ResetEvent (overlapped_.hEvent);
          overlapped_.Internal = 0;
          overlapped_.InternalHigh = 0;

          DWORD n (0);

          if (ReadFile (handle_,
                        buf_.data (),
                        static_cast<DWORD> (input_length_),
                        &n,
                        &overlapped_))
          {
            read_pending_ = true;
            return true;
          }

          if (GetLastError () == ERROR_IO_PENDING)
          {
            read_pending_ = true;
            return true;
          }

          return false;
        }

        HANDLE handle_;
        std::wstring path_;
        hid_attributes attributes_;
        connection link_;
        size_t input_length_;
        size_t feature_length_;

        bool writable_ {true};
        bool read_pending_ {false};
        OVERLAPPED overlapped_ {};

        std::array<std::byte, max_report_size> buf_ {};
      };

      std::optional<std::wstring>
      interface_path (HDEVINFO set, SP_DEVICE_INTERFACE_DATA& iface)
      {
        DWORD size (0);
        SetupDiGetDeviceInterfaceDetailW (set, &iface, nullptr, 0, &size, nullptr);

        if (size == 0)
          return std::nullopt;

        std::vector<uint8_t> storage (size);

        auto* detail (
          reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*> (storage.data ()));
        detail->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA_W);

        if (!SetupDiGetDeviceInterfaceDetailW (set, &iface, detail, size, nullptr,
                                               nullptr))
          return std::nullopt;

        return std::wstring (detail->DevicePath);
      }

      HANDLE
      open_for_query (const std::wstring& path) noexcept
      {
        return CreateFileW (path.c_str (),
                            0,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            nullptr,
                            OPEN_EXISTING,
                            0,
                            nullptr);
      }

      struct collection_caps
      {
        size_t input_report_length;
        size_t feature_report_length;
        uint16_t usage_page;
        uint16_t usage;
      };

      std::optional<collection_caps>
      query_caps (HANDLE h) noexcept
      {
        PHIDP_PREPARSED_DATA pp (nullptr);

        if (!HidD_GetPreparsedData (h, &pp))
          return std::nullopt;

        HIDP_CAPS caps {};
        const bool ok (HidP_GetCaps (pp, &caps) == HIDP_STATUS_SUCCESS);
        HidD_FreePreparsedData (pp);

        if (!ok)
          return std::nullopt;

        return collection_caps {caps.InputReportByteLength,
                                caps.FeatureReportByteLength,
                                caps.UsagePage,
                                caps.Usage};
      }

      constexpr uint16_t usage_page_generic_desktop {0x01};
      constexpr uint16_t usage_game_pad {0x05};
    }

    std::vector<hid_enumeration_entry>
    enumerate (const context& ctx)
    {
      std::vector<hid_enumeration_entry> found;

      GUID hid_guid {};
      HidD_GetHidGuid (&hid_guid);

      HDEVINFO set (SetupDiGetClassDevsW (&hid_guid,
                                          nullptr,
                                          nullptr,
                                          DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

      if (set == INVALID_HANDLE_VALUE)
      {
        ctx.report (severity::warning, facility::transport, errc::transport_failure,
                    "unable to enumerate HID device interfaces");
        return found;
      }

      SP_DEVICE_INTERFACE_DATA iface {};
      iface.cbSize = sizeof (iface);

      for (DWORD i (0);
           SetupDiEnumDeviceInterfaces (set, nullptr, &hid_guid, i, &iface);
           ++i)
      {
        std::optional<std::wstring> path (interface_path (set, iface));

        if (!path)
          continue;

        HANDLE h (open_for_query (*path));

        if (h == INVALID_HANDLE_VALUE)
          continue;

        HIDD_ATTRIBUTES attrs {};
        attrs.Size = sizeof (attrs);

        std::optional<collection_caps> caps;

        if (HidD_GetAttributes (h, &attrs))
          caps = query_caps (h);

        CloseHandle (h);

        if (!caps)
          continue;

        if (caps->usage_page != usage_page_generic_desktop ||
            caps->usage != usage_game_pad)
          continue;

        const vendor_id vendor (attrs.VendorID);
        const product_id product (attrs.ProductID);

        if (classify (vendor, product) == family::unknown)
          continue;

        const connection link (classify_link (caps->input_report_length));

        found.push_back (
          hid_enumeration_entry {
            std::move (*path),
            hid_attributes {vendor, product, attrs.VersionNumber},
            link,
            caps->input_report_length});
      }

      SetupDiDestroyDeviceInfoList (set);
      return found;
    }

    std::unique_ptr<hid_device>
    open (const context& ctx, const std::wstring& path)
    {
      bool writable (true);

      HANDLE h (CreateFileW (path.c_str (),
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             nullptr,
                             OPEN_EXISTING,
                             FILE_FLAG_OVERLAPPED,
                             nullptr));

      if (h == INVALID_HANDLE_VALUE)
      {
        writable = false;

        h = CreateFileW (path.c_str (),
                         GENERIC_READ,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         nullptr,
                         OPEN_EXISTING,
                         FILE_FLAG_OVERLAPPED,
                         nullptr);
      }

      if (h == INVALID_HANDLE_VALUE)
      {
        ctx.report (severity::warning, facility::transport, errc::transport_failure,
                    "unable to open a HID device interface");
        return nullptr;
      }

      HIDD_ATTRIBUTES attrs {};
      attrs.Size = sizeof (attrs);

      std::optional<collection_caps> caps;

      if (HidD_GetAttributes (h, &attrs))
        caps = query_caps (h);

      if (!caps)
      {
        CloseHandle (h);
        ctx.report (severity::warning, facility::transport, errc::transport_failure,
                    "opened HID device does not answer its capabilities");
        return nullptr;
      }

      const vendor_id vendor (attrs.VendorID);
      const product_id product (attrs.ProductID);
      const connection link (classify_link (caps->input_report_length));

      if (classify (vendor, product) == family::unknown ||
          link == connection::unknown)
      {
        CloseHandle (h);
        ctx.report (severity::warning, facility::transport, errc::ambiguous_identity,
                    "opened HID device is not a controller we can decode");
        return nullptr;
      }

      auto d (std::make_unique<windows_hid_device> (
        h,
        path,
        hid_attributes {vendor, product, attrs.VersionNumber},
        link,
        caps->input_report_length,
        caps->feature_report_length));

      d->writable (writable);

      if (!writable)
        ctx.report (severity::info, facility::transport, errc::none,
                    "HID device opened read-only; output reports unavailable");

      return d;
    }
  }
}
