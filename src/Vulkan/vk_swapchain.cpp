#include <Vulkan/vk_swapchain.hpp>

#include <bit>       // countr_zero()
#include <limits>    // numeric_limits
#include <algorithm> // clamp(), max(), min(), ranges::find_if()

#include <Vulkan/vk_error.hpp>
#include <Vulkan/vk_device.hpp>
#include <Vulkan/vk_barrier.hpp>
#include <Vulkan/vk_context.hpp>

using namespace std;

namespace Vulkan
{
  vk_swapchain::
  vk_swapchain (vk_context& c, const options& o)
    : context_ (c), device_ (c.device ()), options_ (o)
  {
    try
    {
      create_swapchain ();
      create_back_buffer ();
    }
    catch (...)
    {
      // The destructor won't be called, so clean up here.
      //
      destroy_swapchain ();
      throw;
    }
  }

  vk_swapchain::
  ~vk_swapchain ()
  {
    try
    {
      device_.wait_idle ();
    }
    catch (const vk_error&)
    {
      // Ignore since the device is probably lost and so idle anyway.
    }

    destroy_swapchain ();
    vk_release (back_buffer_);
  }

  void vk_swapchain::
  present ()
  {
    // Recreate the swapchain if it is out of date.
    //
    // Note that if we can't create one (for example, the window is
    // minimized), then we still submit the frame since IW4 expects its work
    // to complete.
    //
    if (swapchain_ == VK_NULL_HANDLE || outdated_)
    {
      device_.wait_idle ();

      if (!create_swapchain ())
      {
        context_.submit ();
        return;
      }
    }

    VkDevice h (device_.handle ());

    // Acquire the image.
    //
    // If the swapchain is out of date, then skip presenting this frame and
    // recreate it on the next present. Note that we still present with a
    // suboptimal swapchain.
    //
    size_t ai (acquire_semaphore ());
    VkSemaphore acquired (acquired_[ai].first);

    uint32_t i;
    VkResult r (vkAcquireNextImageKHR (h, swapchain_,
                                       numeric_limits<uint64_t>::max (),
                                       acquired, VK_NULL_HANDLE,
                                       &i));

    if (r == VK_ERROR_OUT_OF_DATE_KHR)
    {
      outdated_ = true;
      context_.submit ();
      return;
    }

    vk_check (r, "unable to acquire swapchain image");

    if (r == VK_SUBOPTIMAL_KHR)
      outdated_ = true;

    // Copy the back buffer into the swapchain image. If their sizes differ,
    // then use linear filtering.
    //
    // Note that we don't care about the image's previous contents since the
    // blit overwrites all of it.
    //
    vk_texture& bt (back_buffer_->texture ());
    context_.transition (bt, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    VkCommandBuffer c (context_.commands ());
    VkImage si (images_[i]);
    VkImageSubresourceRange sr {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vk_transition (c, si, sr,
                   VK_IMAGE_LAYOUT_UNDEFINED,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkExtent2D be {back_buffer_->width (), back_buffer_->height ()};

    VkImageBlit b {};
    b.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    b.srcOffsets[1] = {static_cast<int32_t> (be.width),
                       static_cast<int32_t> (be.height),
                       1};
    b.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    b.dstOffsets[1] = {static_cast<int32_t> (extent_.width),
                       static_cast<int32_t> (extent_.height),
                       1};

    bool same (be.width == extent_.width && be.height == extent_.height);

    vkCmdBlitImage (c,
                    bt.image (), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    si, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, &b,
                    same ? VK_FILTER_NEAREST : VK_FILTER_LINEAR);

    vk_transition (c, si, sr,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // Submit the frame and present the image.
    //
    // Note that we remember the submission to know when the acquire
    // semaphore can be reused (see acquire_semaphore()).
    //
    acquired_[ai].second = context_.submit ({acquired, rendered_[i]});

    VkPresentInfoKHR pi {};
    pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = &rendered_[i];
    pi.swapchainCount = 1;
    pi.pSwapchains = &swapchain_;
    pi.pImageIndices = &i;

    VkResult pr (device_.present (pi));

    if (pr == VK_ERROR_OUT_OF_DATE_KHR || pr == VK_SUBOPTIMAL_KHR)
      outdated_ = true;
    else
      vk_check (pr, "unable to present swapchain image");
  }

  void vk_swapchain::
  reset (const options& o)
  {
    device_.wait_idle ();

    options_ = o;

    destroy_swapchain ();
    vk_release (back_buffer_);

    create_swapchain ();
    create_back_buffer ();
  }

  bool vk_swapchain::
  create_swapchain ()
  {
    VkPhysicalDevice p (device_.physical ());
    VkSurfaceKHR s (device_.surface ());
    VkDevice h (device_.handle ());

    VkSurfaceCapabilitiesKHR caps;
    vk_check (vkGetPhysicalDeviceSurfaceCapabilitiesKHR (p, s, &caps),
              "unable to query surface capabilities");

    // Figure out the extent. If the surface leaves it to us, then use the
    // back buffer size. A zero extent means the window is minimized, in which
    // case there can be no swapchain.
    //
    VkExtent2D e (caps.currentExtent);

    if (e.width == numeric_limits<uint32_t>::max ())
      e = {clamp (options_.width,
                  caps.minImageExtent.width,
                  caps.maxImageExtent.width),
           clamp (options_.height,
                  caps.minImageExtent.height,
                  caps.maxImageExtent.height)};

    if (e.width == 0 || e.height == 0)
    {
      destroy_swapchain ();
      return false;
    }

    if ((caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0)
      throw vk_error (VK_ERROR_FEATURE_NOT_PRESENT,
                      "swapchain images can't be copied to");

    // Pick the format. We prefer 8-bit UNORM so that the blit copies the
    // back buffer bytes unchanged, as Direct3D 9 presents them. Failing
    // that, use whatever comes first.
    //
    VkSurfaceFormatKHR f;
    {
      uint32_t n (0);
      vk_check (vkGetPhysicalDeviceSurfaceFormatsKHR (p, s, &n, nullptr),
                "unable to query surface formats");

      vector<VkSurfaceFormatKHR> fs (n);
      vk_check (vkGetPhysicalDeviceSurfaceFormatsKHR (p, s, &n, fs.data ()),
                "unable to query surface formats");

      if (fs.empty ())
        throw vk_error (VK_ERROR_FORMAT_NOT_SUPPORTED,
                        "surface has no formats");

      auto preferred = [] (const VkSurfaceFormatKHR& x)
      {
        return (x.format == VK_FORMAT_B8G8R8A8_UNORM ||
                x.format == VK_FORMAT_R8G8B8A8_UNORM) &&
               x.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      };

      auto i (ranges::find_if (fs, preferred));
      f = i != fs.end () ? *i : fs.front ();
    }

    // Pick the present mode. FIFO is always supported and without vsync we
    // prefer mailbox since it doesn't tear.
    //
    VkPresentModeKHR m (VK_PRESENT_MODE_FIFO_KHR);

    if (!options_.vsync)
    {
      uint32_t n (0);
      vk_check (vkGetPhysicalDeviceSurfacePresentModesKHR (p, s, &n, nullptr),
                "unable to query present modes");

      vector<VkPresentModeKHR> ms (n);
      vk_check (vkGetPhysicalDeviceSurfacePresentModesKHR (p, s,
                                                           &n, ms.data ()),
                "unable to query present modes");

      for (VkPresentModeKHR x: {VK_PRESENT_MODE_MAILBOX_KHR,
                                VK_PRESENT_MODE_IMMEDIATE_KHR})
      {
        if (ranges::find (ms, x) != ms.end ())
        {
          m = x;
          break;
        }
      }
    }

    // Ask for at least 3 images and one more than the minimum so that we
    // normally don't have to wait for the presentation engine.
    //
    uint32_t count (max (caps.minImageCount + 1, 3u));
    if (caps.maxImageCount != 0)
      count = min (count, caps.maxImageCount);

    // Use opaque composition if supported since the back buffer alpha is not
    // meant for the window system.
    //
    VkCompositeAlphaFlagBitsKHR alpha (VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
    if ((caps.supportedCompositeAlpha & alpha) == 0)
      alpha = static_cast<VkCompositeAlphaFlagBitsKHR> (
        1u << countr_zero (caps.supportedCompositeAlpha));

    VkSwapchainCreateInfoKHR ci {};
    ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    ci.surface = s;
    ci.minImageCount = count;
    ci.imageFormat = f.format;
    ci.imageColorSpace = f.colorSpace;
    ci.imageExtent = e;
    ci.imageArrayLayers = 1;
    ci.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.preTransform = caps.currentTransform;
    ci.compositeAlpha = alpha;
    ci.presentMode = m;
    ci.clipped = VK_TRUE;
    ci.oldSwapchain = swapchain_;

    // Note that we can only destroy the old swapchain after creating the new
    // one since we pass it as oldSwapchain.
    //
    VkSwapchainKHR sc;
    vk_check (vkCreateSwapchainKHR (h, &ci, nullptr, &sc),
              "unable to create swapchain");

    destroy_swapchain ();

    swapchain_ = sc;
    format_ = f.format;
    extent_ = e;
    outdated_ = false;

    uint32_t n (0);
    vk_check (vkGetSwapchainImagesKHR (h, swapchain_, &n, nullptr),
              "unable to get swapchain images");

    images_.resize (n);
    vk_check (vkGetSwapchainImagesKHR (h, swapchain_, &n, images_.data ()),
              "unable to get swapchain images");

    // Create a render semaphore for each image. There is no way to know when
    // the present is done waiting on it, but it must be done once the
    // acquire returns the same image again.
    //
    VkSemaphoreCreateInfo si {};
    si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (uint32_t j (0); j != n; ++j)
    {
      VkSemaphore x;
      vk_check (vkCreateSemaphore (h, &si, nullptr, &x),
                "unable to create semaphore");
      rendered_.push_back (x);
    }

    return true;
  }

  void vk_swapchain::
  destroy_swapchain () noexcept
  {
    VkDevice h (device_.handle ());

    for (VkSemaphore s: rendered_)
      vkDestroySemaphore (h, s, nullptr);

    rendered_.clear ();
    images_.clear ();

    for (const auto& a: acquired_)
      vkDestroySemaphore (h, a.first, nullptr);

    acquired_.clear ();
    next_acquire_ = 0;

    if (swapchain_ != VK_NULL_HANDLE)
    {
      vkDestroySwapchainKHR (h, swapchain_, nullptr);
      swapchain_ = VK_NULL_HANDLE;
    }
  }

  void vk_swapchain::
  create_back_buffer ()
  {
    back_buffer_ = vk_surface::create (device_,
                                       options_.width, options_.height,
                                       options_.format,
                                       1,
                                       d3d9::usage_rendertarget,
                                       d3d9::pool_default);
  }

  size_t vk_swapchain::
  acquire_semaphore ()
  {
    // The acquire semaphore has to be picked before we know the image, so
    // we cycle through a few and, before reusing one, wait for the
    // submission that waited on it.
    //
    // @@ Is 4 enough to make this wait rare? Probably depends on the frames
    //    in flight.
    //
    if (acquired_.empty ())
    {
      VkSemaphoreCreateInfo si {};
      si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

      for (uint32_t i (0); i != 4; ++i)
      {
        VkSemaphore s;
        vk_check (vkCreateSemaphore (device_.handle (), &si, nullptr, &s),
                  "unable to create semaphore");
        acquired_.emplace_back (s, 0);
      }
    }

    size_t r (next_acquire_);
    next_acquire_ = (next_acquire_ + 1) % acquired_.size ();

    if (uint64_t s = acquired_[r].second; s != 0)
      device_.wait (s);

    return r;
  }
}
