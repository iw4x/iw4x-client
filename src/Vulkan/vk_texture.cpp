#include <Vulkan/vk_texture.hpp>

#include <bit>       // bit_width()
#include <cassert>
#include <utility>   // move()
#include <algorithm> // max(), min()

#include <Vulkan/vk_error.hpp>
#include <Vulkan/vk_barrier.hpp>

using namespace std;

namespace Vulkan
{
  vk_texture::
  vk_texture (vk_device& d, const vk_texture_desc& desc)
    : vk_object (d), desc_ (desc)
  {
    optional<vk_format_info> f (vk_format (desc_.format));
    if (!f)
      throw vk_error (VK_ERROR_FORMAT_NOT_SUPPORTED,
                      "unsupported texture format {}",
                      static_cast<int> (desc_.format));
    info_ = *f;

    // Zero levels means the whole chain down to 1x1, as in Direct3D 9.
    //
    if (desc_.levels == 0)
      desc_.levels = bit_width (max ({desc_.width, desc_.height, desc_.depth}));

    locks_.resize (faces () * desc_.levels);

    // A system memory texture only lives in host memory, so allocate all
    // its levels in one block and skip the image.
    //
    if (desc_.pool == d3d9::pool_systemmem)
    {
      size_t n (0);
      for (uint32_t face (0); face != faces (); ++face)
      {
        for (uint32_t l (0); l != desc_.levels; ++l)
        {
          host_offsets_.push_back (n);
          n += static_cast<size_t> (info_.pitch (width (l))) *
               info_.rows (height (l)) * depth (l);
        }
      }

      host_ = new unsigned char[n] ();
      return;
    }

    try
    {
      create_image ();
    }
    catch (...)
    {
      // The destructor won't be called, so clean up here.
      //
      destroy ();
      throw;
    }
  }

  vk_texture::
  ~vk_texture ()
  {
    destroy ();
  }

  void vk_texture::
  destroy () noexcept
  {
    VkDevice d (device ().handle ());

    for (VkImageView v: attachment_views_)
    {
      if (v != VK_NULL_HANDLE)
        vkDestroyImageView (d, v, nullptr);
    }
    attachment_views_.clear ();

    for (VkImageView v: srgb_attachment_views_)
    {
      if (v != VK_NULL_HANDLE)
        vkDestroyImageView (d, v, nullptr);
    }
    srgb_attachment_views_.clear ();

    if (view_ != VK_NULL_HANDLE)
      vkDestroyImageView (d, view_, nullptr);

    if (srgb_view_ != VK_NULL_HANDLE)
      vkDestroyImageView (d, srgb_view_, nullptr);

    srgb_view_ = VK_NULL_HANDLE;
    srgb_format_ = VK_FORMAT_UNDEFINED;

    if (image_ != VK_NULL_HANDLE)
      vkDestroyImage (d, image_, nullptr);

    device ().free (memory_);

    view_ = VK_NULL_HANDLE;
    image_ = VK_NULL_HANDLE;
    memory_ = vk_allocation ();

    for (lock_state& l: locks_)
    {
      if (l.staging.buffer != VK_NULL_HANDLE)
        device ().destroy_buffer (l.staging.buffer, l.staging.memory);
    }
    locks_.clear ();

    delete[] host_;
    host_ = nullptr;
  }

  void vk_texture::
  create_image ()
  {
    using kind = vk_texture_desc::kind;

    vk_device& d (device ());

    bool rt (desc_.usage & d3d9::usage_rendertarget);
    bool ds (info_.depth);
    bool ms (desc_.samples > 1);

    // Pick the format. If the device doesn't support the Direct3D 9
    // format's natural match for this usage, then fall back to the
    // alternative (see vk_format()). Note that multisampled images can't be
    // sampled in Direct3D 9, so we don't ask for that.
    //
    VkFormatFeatureFlags need (0);
    if (!ms)
      need |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
    if (rt)
      need |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    if (ds)
      need |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    auto supported = [&d, need] (VkFormat f)
    {
      VkFormatProperties p;
      vkGetPhysicalDeviceFormatProperties (d.physical (), f, &p);
      return (p.optimalTilingFeatures & need) == need;
    };

    if (supported (info_.format))
      format_ = info_.format;
    else if (info_.alternative != VK_FORMAT_UNDEFINED &&
             supported (info_.alternative))
      format_ = info_.alternative;
    else
      throw vk_error (VK_ERROR_FORMAT_NOT_SUPPORTED,
                      "texture format {} is not supported by the device",
                      static_cast<int> (desc_.format));

    // If the format has an sRGB variant, then make the image mutable so that
    // we can also view it as sRGB (for sRGB reads and writes).
    //
    if (VkFormat f = vk_srgb_format (format_);
        f != VK_FORMAT_UNDEFINED && supported (f))
      srgb_format_ = f;

    VkImageCreateInfo ci {};
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ci.flags = desc_.type == kind::cube
      ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
      : 0;

    if (srgb_format_ != VK_FORMAT_UNDEFINED)
      ci.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    ci.imageType = desc_.type == kind::volume
      ? VK_IMAGE_TYPE_3D
      : VK_IMAGE_TYPE_2D;
    ci.format = format_;
    ci.extent = {desc_.width, desc_.height, desc_.depth};
    ci.mipLevels = desc_.levels;
    ci.arrayLayers = faces ();
    ci.samples = static_cast<VkSampleCountFlagBits> (desc_.samples);
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
               VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (!ms)
      ci.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (rt)
      ci.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (ds)
      ci.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    vk_check (vkCreateImage (d.handle (), &ci, nullptr, &image_),
              "unable to create {}x{} image", desc_.width, desc_.height);

    VkMemoryRequirements r;
    vkGetImageMemoryRequirements (d.handle (), image_, &r);

    memory_ = d.allocate (r, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vk_check (vkBindImageMemory (d.handle (), image_, memory_.memory,
                                 memory_.offset),
              "unable to bind image memory");

    // Create the sampled views. Note that a depth view only has the depth
    // aspect since that is what a shadow sampler reads.
    //
    if (!ms)
    {
      VkImageViewCreateInfo vi {};
      vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      vi.image = image_;
      vi.viewType = desc_.type == kind::cube   ? VK_IMAGE_VIEW_TYPE_CUBE :
                    desc_.type == kind::volume ? VK_IMAGE_VIEW_TYPE_3D   :
                                                 VK_IMAGE_VIEW_TYPE_2D;
      vi.format = format_;
      vi.components = info_.swizzle;
      vi.subresourceRange = range ();

      if (ds)
        vi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

      vk_check (vkCreateImageView (d.handle (), &vi, nullptr, &view_),
                "unable to create image view");

      if (srgb_format_ != VK_FORMAT_UNDEFINED)
      {
        vi.format = srgb_format_;
        vk_check (vkCreateImageView (d.handle (), &vi, nullptr, &srgb_view_),
                  "unable to create sRGB image view");
      }
    }

    // The attachment views are created on first use (see attachment_view()).
    //
    attachment_views_.resize (faces () * desc_.levels, VK_NULL_HANDLE);
    srgb_attachment_views_.resize (faces () * desc_.levels, VK_NULL_HANDLE);

    // Transition an ordinary texture to the shader read layout right away,
    // which is where it stays between uploads (see unlock()). This way it can
    // be sampled even before IW4 fills it. Render targets get their layout
    // on first use.
    //
    if (!rt && !ds && !ms)
    {
      VkImage i (image_);
      VkImageSubresourceRange sr (range ());

      d.record ([i, sr] (VkCommandBuffer c)
      {
        vk_transition (c, i, sr,
                       VK_IMAGE_LAYOUT_UNDEFINED,
                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      });

      layout_ = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
  }

  uint32_t vk_texture::
  width (uint32_t l) const noexcept
  {
    return max (1u, desc_.width >> l);
  }

  uint32_t vk_texture::
  height (uint32_t l) const noexcept
  {
    return max (1u, desc_.height >> l);
  }

  uint32_t vk_texture::
  depth (uint32_t l) const noexcept
  {
    return max (1u, desc_.depth >> l);
  }

  d3d9::surface_desc vk_texture::
  level_desc (uint32_t l) const noexcept
  {
    d3d9::surface_desc r {};
    r.format = desc_.format;
    r.type = d3d9::rtype_surface;
    r.usage = desc_.usage;
    r.pool = desc_.pool;
    r.multisample = static_cast<d3d9::multisample_type> (
      desc_.samples > 1 ? desc_.samples : 0);
    r.multisample_quality = 0;
    r.width = width (l);
    r.height = height (l);
    return r;
  }

  d3d9::volume_desc vk_texture::
  volume_desc (uint32_t l) const noexcept
  {
    d3d9::volume_desc r {};
    r.format = desc_.format;
    r.type = d3d9::rtype_volume;
    r.usage = desc_.usage;
    r.pool = desc_.pool;
    r.width = width (l);
    r.height = height (l);
    r.depth = depth (l);
    return r;
  }

  unsigned char* vk_texture::
  host_data (uint32_t l) const noexcept
  {
    assert (host_ != nullptr);
    return host_ + host_offsets_[l];
  }

  uint32_t vk_texture::
  host_pitch (uint32_t l) const noexcept
  {
    return info_.pitch (width (l));
  }

  void* vk_texture::
  lock (uint32_t face, uint32_t level,
        VkOffset3D o, VkExtent3D e, uint32_t flags,
        uint32_t& row_pitch, uint32_t& slice_pitch)
  {
    assert (face < faces () && level < desc_.levels);

    if (host_ != nullptr)
    {
      row_pitch = info_.pitch (width (level));
      slice_pitch = row_pitch * info_.rows (height (level));

      return host_ + host_offsets_[subresource (face, level)] +
             static_cast<size_t> (o.z) * slice_pitch +
             static_cast<size_t> (o.y / info_.block_size) * row_pitch +
             static_cast<size_t> (o.x / info_.block_size) * info_.block_bytes;
    }

    // Otherwise, lock into a staging buffer for the region and copy it to
    // the image on unlock.
    //
    // @@ We don't read the image back, so the buffer doesn't have the
    //    current contents. That's fine for a write that covers the whole
    //    region but not for a read (lock_readonly) and a partial write. Does
    //    IW4 ever do that?
    //
    row_pitch = info_.pitch (e.width);
    slice_pitch = row_pitch * info_.rows (e.height);

    vk_staging s (
      device ().stage (static_cast<VkDeviceSize> (slice_pitch) * e.depth));
    void* r (s.data ());

    // Note that several threads may lock different subresources in this
    // texture at the same time, thus the mutex.
    //
    scoped_lock l (lock_mutex_);

    lock_state& ls (locks_[subresource (face, level)]);
    assert (!ls.locked);

    ls.locked = true;
    ls.staging = move (s);
    ls.offset = o;
    ls.extent = e;
    ls.flags = flags;

    return r;
  }

  void vk_texture::
  unlock (uint32_t face, uint32_t level)
  {
    if (host_ != nullptr)
      return;

    lock_state ls;
    {
      scoped_lock l (lock_mutex_);
      lock_state& s (locks_[subresource (face, level)]);
      assert (s.locked);

      ls = move (s);
      s = lock_state ();
    }

    // Nothing to upload for a read-only lock.
    //
    if (ls.flags & d3d9::lock_readonly)
    {
      device ().destroy_buffer (ls.staging.buffer, ls.staging.memory);
      return;
    }

    // Copy the staging buffer into the image. A lockable texture is never a
    // render target, so it is always in the shader read layout (see
    // create_image()).
    //
    assert (layout_ == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkImageSubresourceRange sr {};
    sr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    sr.baseMipLevel = level;
    sr.levelCount = 1;
    sr.baseArrayLayer = face;
    sr.layerCount = 1;

    VkBufferImageCopy bc {};
    bc.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bc.imageSubresource.mipLevel = level;
    bc.imageSubresource.baseArrayLayer = face;
    bc.imageSubresource.layerCount = 1;
    bc.imageOffset = ls.offset;
    bc.imageExtent = ls.extent;

    VkImage i (image_);

    device ().upload (move (ls.staging),
                      [i, sr, bc] (VkCommandBuffer c, const vk_staging& s)
    {
      vk_transition (c, i, sr,
                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

      vkCmdCopyBufferToImage (c, s.buffer, i,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bc);

      vk_transition (c, i, sr,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });
  }

  d3d9::locked_rect vk_texture::
  lock_rect (uint32_t face, uint32_t level, const d3d9::rect* r, uint32_t flags)
  {
    VkOffset3D o {0, 0, 0};
    VkExtent3D e {width (level), height (level), 1};

    if (r != nullptr)
    {
      o = {r->left, r->top, 0};
      e = {static_cast<uint32_t> (r->right - r->left),
           static_cast<uint32_t> (r->bottom - r->top), 1};
    }

    uint32_t row, slice;
    void* p (lock (face, level, o, e, flags, row, slice));

    d3d9::locked_rect lr;
    lr.pitch = static_cast<int32_t> (row);
    lr.bits = p;
    return lr;
  }

  void vk_texture::
  unlock_rect (uint32_t face, uint32_t level)
  {
    unlock (face, level);
  }

  d3d9::locked_box vk_texture::
  lock_box (uint32_t level, const d3d9::box* b, uint32_t flags)
  {
    VkOffset3D o {0, 0, 0};
    VkExtent3D e {width (level), height (level), depth (level)};

    if (b != nullptr)
    {
      o = {static_cast<int32_t> (b->left),
           static_cast<int32_t> (b->top),
           static_cast<int32_t> (b->front)};
      e = {b->right - b->left, b->bottom - b->top, b->back - b->front};
    }

    uint32_t row, slice;
    void* p (lock (0, level, o, e, flags, row, slice));

    d3d9::locked_box lb;
    lb.row_pitch = static_cast<int32_t> (row);
    lb.slice_pitch = static_cast<int32_t> (slice);
    lb.bits = p;
    return lb;
  }

  void vk_texture::
  unlock_box (uint32_t level)
  {
    unlock (0, level);
  }

  vk_surface* vk_texture::
  surface (uint32_t face, uint32_t level)
  {
    assert (face < faces () && level < desc_.levels);

    // As in Direct3D 9, each call returns a new reference, here a new
    // surface object that holds a reference to the texture.
    //
    return new vk_surface (*this, face, level);
  }

  VkImageView vk_texture::
  attachment_view (uint32_t face, uint32_t level, bool srgb)
  {
    // Fall back to the linear view if there is no sRGB format.
    //
    srgb = srgb && srgb_format_ != VK_FORMAT_UNDEFINED;

    VkImageView& v ((srgb ? srgb_attachment_views_
                          : attachment_views_)[subresource (face, level)]);

    if (v == VK_NULL_HANDLE)
    {
      VkImageViewCreateInfo vi {};
      vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      vi.image = image_;
      vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
      vi.format = srgb ? srgb_format_ : format_;
      vi.subresourceRange.aspectMask = info_.aspect ();
      vi.subresourceRange.baseMipLevel = level;
      vi.subresourceRange.levelCount = 1;
      vi.subresourceRange.baseArrayLayer = face;
      vi.subresourceRange.layerCount = 1;

      vk_check (vkCreateImageView (device ().handle (), &vi, nullptr, &v),
                "unable to create attachment view");
    }

    return v;
  }

  VkImageSubresourceRange vk_texture::
  range () const noexcept
  {
    VkImageSubresourceRange r {};
    r.aspectMask = info_.aspect ();
    r.baseMipLevel = 0;
    r.levelCount = desc_.levels;
    r.baseArrayLayer = 0;
    r.layerCount = faces ();
    return r;
  }

  vk_surface::
  vk_surface (vk_texture& t, uint32_t face, uint32_t level)
    : vk_object (t.device ()), texture_ (t), face_ (face), level_ (level)
  {
    texture_.add_ref ();
  }

  vk_surface::
  ~vk_surface ()
  {
    texture_.release ();
  }

  vk_surface* vk_surface::
  create (vk_device& d,
          uint32_t width, uint32_t height,
          d3d9::format format, uint32_t samples,
          uint32_t usage, d3d9::pool pool)
  {
    vk_texture_desc td;
    td.width = width;
    td.height = height;
    td.levels = 1;
    td.samples = max (1u, samples);
    td.format = format;
    td.usage = usage;
    td.pool = pool;

    // A standalone surface (render target, depth stencil) is backed by a
    // single-level texture, which only the surface references.
    //
    vk_texture* t (new vk_texture (d, td));
    vk_surface* s (new vk_surface (*t, 0, 0));
    t->release ();
    return s;
  }

  d3d9::surface_desc vk_surface::
  desc () const noexcept
  {
    return texture_.level_desc (level_);
  }
}
