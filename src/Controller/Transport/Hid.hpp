#pragma once

#include "../Types.hpp"

#include "../Context.hpp"
#include "../Device/Id.hpp"
#include "../Device/Identity.hpp"

namespace Controller
{
  namespace transport
  {
    struct hid_attributes
    {
      vendor_id vendor;
      product_id product;
      std::optional<uint16_t> version;
    };

    class hid_device
    {
    public:
      virtual
      ~hid_device () = default;

      virtual connection
      link () const noexcept = 0;

      virtual hid_attributes
      attributes () const noexcept = 0;

      virtual const std::wstring&
      path () const noexcept = 0;

      virtual size_t
      feature_report_length () const noexcept = 0;

      virtual std::optional<size_t>
      read (std::span<std::byte> buf) noexcept = 0;

      virtual std::optional<size_t>
      write (std::span<const std::byte> buf) noexcept = 0;

      virtual bool
      get_feature (std::span<std::byte> buf) noexcept = 0;

      virtual bool
      set_feature (std::span<const std::byte> buf) noexcept = 0;
    };

    struct hid_enumeration_entry
    {
      std::wstring path;
      hid_attributes attributes;
      connection link {connection::unknown};
      size_t input_report_length {0};
    };

    connection
    classify_link (size_t input_report_length) noexcept;

    std::vector<hid_enumeration_entry>
    enumerate (const context&);

    std::unique_ptr<hid_device>
    open (const context&, const std::wstring& path);
  }
}
