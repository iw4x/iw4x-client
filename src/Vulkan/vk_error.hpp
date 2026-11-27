#pragma once

#include <format>
#include <utility>      // forward()
#include <system_error>

#include <Vulkan/vk.hpp>

namespace Vulkan
{
  // Return the error category for VkResult codes. Its messages are the
  // result names (see to_string()).
  //
  const std::error_category&
  vk_category () noexcept;

  // Return the result's enumerator name (for example, "VK_TIMEOUT"),
  // "unknown VkResult" if we don't know it.
  //
  const char*
  to_string (VkResult) noexcept;

  // Vulkan failure. The error code is the failed call's VkResult and the
  // description says what failed (for example, "unable to create Vulkan
  // instance").
  //
  class vk_error: public std::system_error
  {
  public:
    template <std::formattable<char>... A>
    vk_error (VkResult r, std::format_string<A...> f, A&&... a)
      : std::system_error (static_cast<int> (r),
                           vk_category (),
                           std::format (f, std::forward<A> (a)...)) {}

    VkResult
    result () const noexcept {return static_cast<VkResult> (code ().value ());}
  };

  // Throw vk_error with the formatted description if the result is an error
  // (negative). Otherwise, return the result.
  //
  // Note that the positive results (VK_SUBOPTIMAL_KHR, for example) are not
  // errors and come back as is.
  //
  template <std::formattable<char>... A>
  inline VkResult
  vk_check (VkResult r, std::format_string<A...> f, A&&... a)
  {
    if (r < 0)
      throw vk_error (r, f, std::forward<A> (a)...);

    return r;
  }
}
