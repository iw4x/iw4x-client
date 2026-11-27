#include <Vulkan/vk_context.hpp>

#include <array>
#include <cstring> // memcpy()

#include <Vulkan/vk_error.hpp>

using namespace std;

namespace Vulkan
{
  namespace
  {
    // Return the rectangle's corners as blit offsets. As in Direct3D 9, a
    // NULL rectangle means the whole surface.
    //
    array<VkOffset3D, 2>
    corners (const vk_surface& s, const d3d9::rect* r) noexcept
    {
      if (r == nullptr)
        return {VkOffset3D {0, 0, 0},
                VkOffset3D {static_cast<int32_t> (s.width ()),
                            static_cast<int32_t> (s.height ()),
                            1}};

      return {VkOffset3D {r->left, r->top, 0},
              VkOffset3D {r->right, r->bottom, 1}};
    }

    VkImageSubresourceLayers
    layers (const vk_surface& s, VkImageAspectFlags aspect) noexcept
    {
      return {aspect, s.level (), s.face (), 1};
    }

    // Note that we only copy the depth aspect of depth/stencil textures.
    //
    // @@ Should we copy stencil as well? Nothing in IW4 seems to need it.
    //
    VkImageAspectFlags
    copy_aspect (const vk_texture& t) noexcept
    {
      return t.info ().depth ? VK_IMAGE_ASPECT_DEPTH_BIT
                             : VK_IMAGE_ASPECT_COLOR_BIT;
    }
  }

  void vk_context::
  stretch_rect (vk_surface& src, const d3d9::rect* sr,
                vk_surface& dst, const d3d9::rect* dr,
                d3d9::texture_filter_type filter)
  {
    vk_texture& st (src.texture ());
    vk_texture& dt (dst.texture ());

    array<VkOffset3D, 2> s (corners (src, sr));
    array<VkOffset3D, 2> d (corners (dst, dr));

    bool same_size (s[1].x - s[0].x == d[1].x - d[0].x &&
                    s[1].y - s[0].y == d[1].y - d[0].y);

    VkImageAspectFlags aspect (copy_aspect (st));
    bool depth (st.info ().depth);

    VkImageLayout sl (VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    VkImageLayout dl (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // An image can only be in one layout so copying within the same texture
    // has to use the general layout for both sides.
    //
    if (&st == &dt)
    {
      sl = dl = VK_IMAGE_LAYOUT_GENERAL;
      transition (st, sl);
    }
    else
    {
      transition (st, sl);
      transition (dt, dl);
    }

    VkCommandBuffer c (commands ());

    vk_texture* tmp (nullptr);
    VkImage si (st.image ());
    VkImageSubresourceLayers ss (layers (src, aspect));

    // A multisampled source has to be resolved first since vkCmdBlitImage()
    // only accepts single-sampled images.
    //
    if (st.desc ().samples > 1)
    {
      // @@ vkCmdResolveImage() only handles color, so we silently skip
      //    multisampled depth. Maybe we should resolve it with a shader?
      //
      if (depth)
        return;

      VkImageResolve r {};
      r.srcSubresource = ss;
      r.srcOffset = s[0];
      r.extent = {static_cast<uint32_t> (s[1].x - s[0].x),
                  static_cast<uint32_t> (s[1].y - s[0].y),
                  1};

      // If no scaling (and no format conversion) is involved, then resolve
      // straight into the destination.
      //
      if (same_size &&
          st.format () == dt.format () &&
          dt.desc ().samples == 1)
      {
        r.dstSubresource = layers (dst, aspect);
        r.dstOffset = d[0];

        vkCmdResolveImage (c, si, sl, dt.image (), dl, 1, &r);
        return;
      }

      // Otherwise resolve into a temporary texture and blit from that. We
      // keep the source coordinates, so the temporary is as large as the
      // whole source surface.
      //
      vk_texture_desc td;
      td.width = src.width ();
      td.height = src.height ();
      td.format = st.desc ().format;
      td.usage = d3d9::usage_rendertarget;

      tmp = new vk_texture (device_, td);
      transition (*tmp, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

      r.dstSubresource = {aspect, 0, 0, 1};
      r.dstOffset = s[0];

      vkCmdResolveImage (c, si, sl,
                         tmp->image (), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         1, &r);

      transition (*tmp, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

      si = tmp->image ();
      ss = {aspect, 0, 0, 1};
    }

    VkImageBlit b {};
    b.srcSubresource = ss;
    b.srcOffsets[0] = s[0];
    b.srcOffsets[1] = s[1];
    b.dstSubresource = layers (dst, aspect);
    b.dstOffsets[0] = d[0];
    b.dstOffsets[1] = d[1];

    // Depth formats don't support linear filtering in blits.
    //
    VkFilter f (depth || filter == d3d9::texf_none || filter == d3d9::texf_point
                ? VK_FILTER_NEAREST
                : VK_FILTER_LINEAR);

    vkCmdBlitImage (c,
                    si,
                    tmp != nullptr ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : sl,
                    dt.image (), dl,
                    1, &b,
                    f);

    // Fine even though the blit hasn't executed yet since the deletion is
    // retired (see vk_object::release()).
    //
    vk_release (tmp);
  }

  void vk_context::
  update_surface (vk_surface& src, const d3d9::rect* sr,
                  vk_surface& dst, const d3d9::point* dp)
  {
    vk_texture& st (src.texture ());
    vk_texture& dt (dst.texture ());

    // Like Direct3D 9's UpdateSurface(), we only copy from system memory.
    //
    if (!st.host ())
      return;

    d3d9::rect r (sr != nullptr
                  ? *sr
                  : d3d9::rect {0, 0,
                                static_cast<int32_t> (src.width ()),
                                static_cast<int32_t> (src.height ())});

    // As in Direct3D 9, a NULL point means the source rectangle's origin.
    //
    d3d9::point p (dp != nullptr ? *dp : d3d9::point {r.left, r.top});

    const vk_format_info& fi (dt.info ());

    uint32_t w (static_cast<uint32_t> (r.right - r.left));
    uint32_t h (static_cast<uint32_t> (r.bottom - r.top));
    uint32_t row (fi.pitch (w));
    uint32_t rows (fi.rows (h));

    // Note that the rows and offsets are counted in format blocks, which
    // span several texels with compressed formats.
    //
    uint32_t pitch (st.host_pitch (src.level ()));
    const unsigned char* from (
      st.host_data (src.level ()) +
      static_cast<size_t> (r.top) / fi.block_size * pitch +
      static_cast<size_t> (r.left) / fi.block_size * fi.block_bytes);

    // Stage the rows tightly packed in the ring.
    //
    transient_memory m (transient (static_cast<VkDeviceSize> (row) * rows,
                                   16));

    unsigned char* to (static_cast<unsigned char*> (m.data));
    for (uint32_t i (0); i != rows; ++i)
      memcpy (to + static_cast<size_t> (i) * row,
              from + static_cast<size_t> (i) * pitch,
              row);

    transition (dt, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy bc {};
    bc.bufferOffset = m.offset;
    bc.imageSubresource = layers (dst, VK_IMAGE_ASPECT_COLOR_BIT);
    bc.imageOffset = {p.x, p.y, 0};
    bc.imageExtent = {w, h, 1};

    vkCmdCopyBufferToImage (commands (),
                            m.buffer,
                            dt.image (), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            1, &bc);
  }

  void vk_context::
  read_surface (vk_surface& src, vk_surface& dst)
  {
    vk_texture& st (src.texture ());
    vk_texture& dt (dst.texture ());

    // Like GetRenderTargetData(), we only copy single-sampled surfaces to
    // system memory.
    //
    if (!dt.host () || st.desc ().samples > 1)
      return;

    const vk_format_info& fi (st.info ());

    uint32_t row (fi.pitch (src.width ()));
    uint32_t rows (fi.rows (src.height ()));
    VkDeviceSize size (static_cast<VkDeviceSize> (row) * rows);

    // The CPU reads this buffer, so prefer cached memory.
    //
    auto b (device_.create_buffer (size,
                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   VK_MEMORY_PROPERTY_HOST_CACHED_BIT));
    try
    {
      transition (st, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

      VkBufferImageCopy bc {};
      bc.imageSubresource = layers (src, copy_aspect (st));
      bc.imageExtent = {src.width (), src.height (), 1};

      vkCmdCopyImageToBuffer (commands (),
                              st.image (), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                              b.first,
                              1, &bc);

      // The caller expects the data right away, so submit and wait.
      //
      // @@ This stalls the frame. Does IW4 read surfaces often enough for
      //    this to show? Probably only for screenshots.
      //
      device_.wait (submit ());

      const unsigned char* from (
        static_cast<const unsigned char*> (b.second.mapped));

      unsigned char* to (dt.host_data (dst.level ()));
      uint32_t pitch (dt.host_pitch (dst.level ()));

      for (uint32_t i (0); i != rows; ++i)
        memcpy (to + static_cast<size_t> (i) * pitch,
                from + static_cast<size_t> (i) * row,
                row);
    }
    catch (...)
    {
      // The GPU may still be writing to the buffer, so wait before we
      // destroy it.
      //
      try
      {
        device_.wait_idle ();
      }
      catch (const vk_error&)
      {
        // Ignore. We are already failing with the original exception and
        // if the wait fails too, then the device is most likely lost and
        // destroying the buffer is all that's left to do.
      }

      device_.destroy_buffer (b.first, b.second);
      throw;
    }

    device_.destroy_buffer (b.first, b.second);
  }
}
