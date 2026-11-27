#pragma once

#include <mutex>
#include <vector>
#include <cstdint>

#include <Vulkan/vk.hpp>
#include <Vulkan/vk_d3d9.hpp>
#include <Vulkan/vk_device.hpp>
#include <Vulkan/vk_format.hpp>
#include <Vulkan/vk_object.hpp>

namespace Vulkan
{
  class vk_surface;

  // Texture description.
  //
  struct vk_texture_desc
  {
    enum class kind {plane, cube, volume};

    kind type = kind::plane;

    std::uint32_t width = 1;
    std::uint32_t height = 1;
    std::uint32_t depth = 1;
    std::uint32_t levels = 1;  // 0 means the whole mip chain.
    std::uint32_t samples = 1;

    d3d9::format format = d3d9::fmt_unknown;
    std::uint32_t usage = 0;   // d3d9::usage_* flags.
    d3d9::pool pool = d3d9::pool_default;
  };

  // Direct3D 9 texture (IDirect3D*Texture9).
  //
  class vk_texture: public vk_object
  {
  public:
    // Throw vk_error on failure, including when the device lacks the
    // format.
    //
    vk_texture (vk_device&, const vk_texture_desc&);

    virtual
    ~vk_texture () override;

  public:

    // Note that the levels are resolved, so never 0.
    //
    const vk_texture_desc&
    desc () const noexcept {return desc_;}

    std::uint32_t
    levels () const noexcept {return desc_.levels;}

    std::uint32_t
    faces () const noexcept
    {
      return desc_.type == vk_texture_desc::kind::cube ? 6 : 1;
    }

    // Level dimensions, never less than 1.
    //
    std::uint32_t
    width (std::uint32_t level) const noexcept;

    std::uint32_t
    height (std::uint32_t level) const noexcept;

    std::uint32_t
    depth (std::uint32_t level) const noexcept;

    // Return the level description, as GetLevelDesc() does.
    //
    d3d9::surface_desc
    level_desc (std::uint32_t level) const noexcept;

    d3d9::volume_desc
    volume_desc (std::uint32_t level) const noexcept;

    const vk_format_info&
    info () const noexcept {return info_;}

    // Return true if this is a system memory texture.
    //
    // Such a texture only lives in host memory and has no image. In
    // Direct3D 9 it can't be used for rendering, only copied to the device
    // (UpdateSurface()) and from it (GetRenderTargetData()). See
    // host_data() for its memory.
    //
    bool
    host () const noexcept {return host_ != nullptr;}

  public:

    // Lock the level, as LockRect() and LockBox() do. A null rectangle
    // (box) means the whole level.
    //
    // A system memory texture is locked in place. Otherwise, the lock
    // returns a staging buffer, and unlock copies it to the image.
    //
    // A level can only have one lock at a time but different levels can be
    // locked concurrently.
    //
    // Note: the staging buffer doesn't have the current contents, so a
    // read-only lock (and a write that doesn't cover the whole region) sees
    // garbage.
    //
    d3d9::locked_rect
    lock_rect (std::uint32_t face, std::uint32_t level,
               const d3d9::rect*, std::uint32_t flags);

    void
    unlock_rect (std::uint32_t face, std::uint32_t level);

    d3d9::locked_box
    lock_box (std::uint32_t level, const d3d9::box*, std::uint32_t flags);

    void
    unlock_box (std::uint32_t level);

    // Return a new surface for the level, as GetSurfaceLevel() does.
    //
    vk_surface*
    surface (std::uint32_t face, std::uint32_t level);

    // System memory texture level and its row pitch.
    //
    // Note: for a cube texture this is only the first face.
    //
    unsigned char*
    host_data (std::uint32_t level) const noexcept;

    std::uint32_t
    host_pitch (std::uint32_t level) const noexcept;

  public:

    VkImage
    image () const noexcept {return image_;}

    VkFormat
    format () const noexcept {return format_;}

    // Sampled view. If the format has no sRGB variant, then the sRGB view is
    // the linear one (here and below).
    //
    VkImageView
    view (bool srgb = false) const noexcept
    {
      return srgb && srgb_view_ != VK_NULL_HANDLE ? srgb_view_ : view_;
    }

    // Return the level's render target (depth stencil) view, creating it on
    // first use.
    //
    VkImageView
    attachment_view (std::uint32_t face, std::uint32_t level,
                     bool srgb = false);

    VkFormat
    attachment_format (bool srgb = false) const noexcept
    {
      return srgb && srgb_format_ != VK_FORMAT_UNDEFINED ? srgb_format_
                                                         : format_;
    }

    // Current image layout.
    //
    // Note: whoever records a transition must also update it (see
    // vk_context::transition()).
    //
    VkImageLayout&
    layout () noexcept {return layout_;}

    // Return the range that covers all the levels and faces.
    //
    VkImageSubresourceRange
    range () const noexcept;

  private:
    struct lock_state
    {
      bool locked = false;
      vk_staging staging;
      VkOffset3D offset {};
      VkExtent3D extent {};
      std::uint32_t flags = 0;
    };

    void
    create_image ();

    void
    destroy () noexcept;

    std::uint32_t
    subresource (std::uint32_t face, std::uint32_t level) const noexcept
    {
      return face * desc_.levels + level;
    }

    void*
    lock (std::uint32_t face, std::uint32_t level,
          VkOffset3D, VkExtent3D, std::uint32_t flags,
          std::uint32_t& row_pitch, std::uint32_t& slice_pitch);

    void
    unlock (std::uint32_t face, std::uint32_t level);

  private:
    vk_texture_desc desc_;
    vk_format_info info_;
    VkFormat format_ = VK_FORMAT_UNDEFINED;

    VkImage image_ = VK_NULL_HANDLE;
    vk_allocation memory_;
    VkImageView view_ = VK_NULL_HANDLE;
    std::vector<VkImageView> attachment_views_;

    VkFormat srgb_format_ = VK_FORMAT_UNDEFINED;
    VkImageView srgb_view_ = VK_NULL_HANDLE;
    std::vector<VkImageView> srgb_attachment_views_;

    VkImageLayout layout_ = VK_IMAGE_LAYOUT_UNDEFINED;

    unsigned char* host_ = nullptr;
    std::vector<std::size_t> host_offsets_;

    std::vector<lock_state> locks_;
    std::mutex lock_mutex_;
  };

  // Direct3D 9 surface (IDirect3DSurface9), that is, a texture level.
  //
  class vk_surface: public vk_object
  {
  public:
    // Note that the surface holds a reference to the texture.
    //
    vk_surface (vk_texture&, std::uint32_t face, std::uint32_t level);

    // Create a standalone surface, as CreateRenderTarget() and
    // CreateDepthStencilSurface() do. Throw vk_error on failure.
    //
    static vk_surface*
    create (vk_device&,
            std::uint32_t width, std::uint32_t height,
            d3d9::format, std::uint32_t samples,
            std::uint32_t usage, d3d9::pool);

    virtual
    ~vk_surface () override;

    vk_texture&
    texture () const noexcept {return texture_;}

    std::uint32_t
    face () const noexcept {return face_;}

    std::uint32_t
    level () const noexcept {return level_;}

    std::uint32_t
    width () const noexcept {return texture_.width (level_);}

    std::uint32_t
    height () const noexcept {return texture_.height (level_);}

    d3d9::surface_desc
    desc () const noexcept;

    d3d9::locked_rect
    lock_rect (const d3d9::rect* r, std::uint32_t flags)
    {
      return texture_.lock_rect (face_, level_, r, flags);
    }

    void
    unlock_rect ()
    {
      texture_.unlock_rect (face_, level_);
    }

    // Return true if the two surfaces are the same texture level. Note that
    // the objects can differ since surface() returns a new one every time.
    //
    friend bool
    same (const vk_surface* x, const vk_surface* y) noexcept
    {
      return x == y ||
             (x != nullptr && y != nullptr &&
              &x->texture_ == &y->texture_ &&
              x->face_ == y->face_ && x->level_ == y->level_);
    }

  private:
    vk_texture& texture_;
    std::uint32_t face_;
    std::uint32_t level_;
  };
}
