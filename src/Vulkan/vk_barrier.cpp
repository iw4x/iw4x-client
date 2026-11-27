#include <Vulkan/vk_barrier.hpp>

using namespace std;

namespace Vulkan
{
  namespace
  {
    struct masks
    {
      VkPipelineStageFlags2 stages;
      VkAccessFlags2 access;
    };

    // Return the pipeline stages and access types that use an image in the
    // specified layout. The barrier uses the old layout's masks for the work
    // that has to complete before the transition and the new layout's masks
    // for the work that has to wait for it.
    //
    masks
    layout_masks (VkImageLayout l)
    {
      switch (l)
      {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
          return {VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT,
                  VK_ACCESS_2_TRANSFER_WRITE_BIT};

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
          return {VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT,
                  VK_ACCESS_2_TRANSFER_READ_BIT};

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
          // Vertex shaders sample textures too (vertex texture fetch).
          //
          return {VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT |
                  VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                  VK_ACCESS_2_SHADER_SAMPLED_READ_BIT};

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
          return {VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                  VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT |
                  VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT};

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
          return {VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                  VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                  VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                  VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT};

        case VK_IMAGE_LAYOUT_UNDEFINED:
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
          // Nothing accesses the image in these layouts but we still use the
          // color attachment output stage since vk_device::submit() waits
          // there for the acquired swapchain image. This way the transition
          // happens after the image is acquired.
          //
          return {VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                  VK_ACCESS_2_NONE};

        default:
          // We don't expect other layouts but if we get one, then wait for
          // everything.
          //
          return {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                  VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
      }
    }
  }

  void
  vk_transition (VkCommandBuffer c,
                 VkImage image,
                 const VkImageSubresourceRange& r,
                 VkImageLayout from,
                 VkImageLayout to)
  {
    masks s (layout_masks (from));
    masks d (layout_masks (to));

    VkImageMemoryBarrier2 b {};
    b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    b.srcStageMask = s.stages;
    b.srcAccessMask = s.access;
    b.dstStageMask = d.stages;
    b.dstAccessMask = d.access;
    b.oldLayout = from;
    b.newLayout = to;
    b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // Single queue.
    b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.image = image;
    b.subresourceRange = r;

    VkDependencyInfo di {};
    di.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    di.imageMemoryBarrierCount = 1;
    di.pImageMemoryBarriers = &b;

    vkCmdPipelineBarrier2 (c, &di);
  }
}
