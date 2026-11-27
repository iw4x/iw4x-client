#pragma once

#include <Vulkan/vk.hpp>

namespace Vulkan
{
  // Record a barrier into the command buffer that transitions the image
  // subresource range from one layout to another. The pipeline stages and
  // access types on both sides are derived from the layouts (see the
  // implementation for details). The command buffer must not be within a
  // rendering instance since Vulkan doesn't allow layout transitions there.
  //
  // Note that the caller is expected to track the image's current layout
  // (vk_texture does this for its images).
  //
  void
  vk_transition (VkCommandBuffer,
                 VkImage,
                 const VkImageSubresourceRange&,
                 VkImageLayout from,
                 VkImageLayout to);
}
