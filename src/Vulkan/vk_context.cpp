#include <Vulkan/vk_context.hpp>

#include <bit>       // countr_zero()
#include <cstring>   // memcpy()
#include <utility>   // swap()
#include <algorithm> // max(), min(), ranges::fill()

#include <Vulkan/vk_error.hpp>
#include <Vulkan/vk_barrier.hpp>

using namespace std;

namespace Vulkan
{
  namespace
  {
    // Note that we add the new reference before releasing the old one in
    // case both point to the same object.
    //
    template <vk_object_type T>
    void
    assign (T*& p, T* x) noexcept
    {
      if (x != nullptr)
        x->add_ref ();

      if (p != nullptr)
        p->release ();

      p = x;
    }

    // Return the number of registers in the set, which follows the shader
    // model 3 limits. Vertex shaders have 256 float constants and pixel
    // shaders 224 while the integer and boolean sets have 16 registers in
    // both stages.
    //
    uint32_t
    set_size (vk_shader_stage s, vk_constant_set c) noexcept
    {
      if (c == vk_constant_set::floats)
        return s == vk_shader_stage::pixel ? 224 : 256;

      return 16;
    }

    // Initial ring size. It grows on demand (see transient()).
    //
    const VkDeviceSize ring_size (32 * 1024 * 1024);

    // Our sampler arrays hold the pixel samplers first and the vertex texture
    // samplers after them (see sampler_slot()).
    //
    const size_t vertex_slot (vk_pixel_samplers);
  }

  vk_context::
  vk_context (vk_device& d, bool depth_buffer)
    : device_ (d),
      pipelines_ (d),
      samplers_ (d),
      render_ (depth_buffer),
      ring_ (ring_size)
  {
    VkDevice h (d.handle ());

    for (vk_shader_stage s: {vk_shader_stage::vertex, vk_shader_stage::pixel})
    {
      for (vk_constant_set c: {vk_constant_set::floats,
                               vk_constant_set::ints,
                               vk_constant_set::bools})
      {
        constant_set& cs (constants_[static_cast<size_t> (s)]
                                    [static_cast<size_t> (c)]);
        cs.registers.resize (set_size (s, c), {0, 0, 0, 0});
      }
    }

    try
    {
      // Each slot gets a command pool so that we can reset it as a whole once
      // the slot's submission completes (see begin_commands()).
      //
      for (slot& s: slots_)
      {
        VkCommandPoolCreateInfo pi {};
        pi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pi.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        pi.queueFamilyIndex = d.queue_family ();

        vk_check (vkCreateCommandPool (h, &pi, nullptr, &s.pool),
                  "unable to create command pool");

        VkCommandBufferAllocateInfo ai {};
        ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        ai.commandPool = s.pool;
        ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        ai.commandBufferCount = 1;

        vk_check (vkAllocateCommandBuffers (h, &ai, &s.commands),
                  "unable to allocate command buffer");
      }

      create_ring (ring_size);

      // Default vertex data. The pipeline reads vertex attributes that are
      // missing from the declaration from this buffer with zero stride (see
      // vk_match_vertex_input()), so they all come out as (0, 0, 0, 1). This
      // seems to be what Direct3D 9 drivers return as well.
      //
      {
        auto r (d.create_buffer (16,
                                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
        default_buffer_ = r.first;
        default_memory_ = r.second;

        const float v[4] {0.0f, 0.0f, 0.0f, 1.0f};
        memcpy (default_memory_.mapped, v, sizeof (v));
      }

      create_placeholders ();
    }
    catch (...)
    {
      destroy ();
      throw;
    }
  }

  vk_context::
  ~vk_context ()
  {
    // Wait for the GPU to finish with our command buffers before we destroy
    // anything.
    //
    try
    {
      device_.wait_idle ();
    }
    catch (const vk_error&)
    {
      // Ignore. The device is most likely lost and there is nothing better
      // to do than destroy everything anyway.
    }

    destroy ();
  }

  void vk_context::
  destroy () noexcept
  {
    VkDevice h (device_.handle ());

    for (vk_texture*& t: textures_)
      vk_release (t);

    for (vk_texture*& t: placeholders_)
      vk_release (t);

    for (stream& s: streams_)
      vk_release (s.buffer);

    for (vk_surface*& s: targets_)
      vk_release (s);

    vk_release (depth_);
    vk_release (indices_);
    vk_release (vertex_shader_);
    vk_release (pixel_shader_);
    vk_release (declaration_);

    if (default_buffer_ != VK_NULL_HANDLE)
    {
      device_.destroy_buffer (default_buffer_, default_memory_);
      default_buffer_ = VK_NULL_HANDLE;
    }

    if (ring_buffer_ != VK_NULL_HANDLE)
    {
      device_.destroy_buffer (ring_buffer_, ring_memory_);
      ring_buffer_ = VK_NULL_HANDLE;
    }

    for (slot& s: slots_)
    {
      if (s.pool != VK_NULL_HANDLE)
        vkDestroyCommandPool (h, s.pool, nullptr);

      s.pool = VK_NULL_HANDLE;
    }

  }

  void vk_context::
  set_render_state (d3d9::render_state_type s, uint32_t v)
  {
    if (static_cast<size_t> (s) >= vk_render_states::size ||
        render_[s] == v)
      return;

    render_[s] = v;

    // The sRGB write state selects the attachment views (see
    // begin_rendering()) and so changing it requires a new rendering
    // instance.
    //
    if (s == d3d9::rs_srgbwriteenable)
      end_rendering ();

    // We don't keep track of which states go into the pipeline key and which
    // are dynamic, so we dirty both.
    //
    dirty_ |= dirty_pipeline | dirty_dynamic;

    // The scissor rectangle depends on the scissor test (see
    // set_dynamic_state()).
    //
    if (s == d3d9::rs_scissortestenable)
      dirty_ |= dirty_scissor;
  }

  optional<size_t> vk_context::
  sampler_slot (uint32_t s) noexcept
  {
    if (s < vk_pixel_samplers)
      return s;

    if (s >= d3d9::vertex_texture_sampler0 &&
        s < d3d9::vertex_texture_sampler0 + vk_vertex_samplers)
      return vertex_slot + (s - d3d9::vertex_texture_sampler0);

    return nullopt;
  }

  void vk_context::
  set_sampler_state (uint32_t sampler, d3d9::sampler_state_type s, uint32_t v)
  {
    if (optional<size_t> i = sampler_slot (sampler);
        i && static_cast<size_t> (s) < vk_sampler_states::size)
      sampler_states_[*i][s] = v;
  }

  uint32_t vk_context::
  sampler_state (uint32_t sampler, d3d9::sampler_state_type s) const noexcept
  {
    optional<size_t> i (sampler_slot (sampler));
    return i && static_cast<size_t> (s) < vk_sampler_states::size
      ? sampler_states_[*i][s]
      : 0;
  }

  void vk_context::
  set_texture (uint32_t sampler, vk_texture* t)
  {
    if (optional<size_t> i = sampler_slot (sampler))
      assign (textures_[*i], t);
  }

  vk_texture* vk_context::
  texture (uint32_t sampler) const noexcept
  {
    optional<size_t> i (sampler_slot (sampler));
    return i ? textures_[*i] : nullptr;
  }

  void vk_context::
  set_vertex_shader (vk_shader* s)
  {
    if (s != vertex_shader_)
    {
      assign (vertex_shader_, s);
      dirty_ |= dirty_pipeline;
    }
  }

  void vk_context::
  set_pixel_shader (vk_shader* s)
  {
    if (s != pixel_shader_)
    {
      assign (pixel_shader_, s);
      dirty_ |= dirty_pipeline;
    }
  }

  void vk_context::
  set_constants (vk_shader_stage s,
                 vk_constant_set c,
                 uint32_t start,
                 const void* data,
                 uint32_t count)
  {
    constant_set& cs (constants_[static_cast<size_t> (s)]
                                [static_cast<size_t> (c)]);

    uint32_t n (static_cast<uint32_t> (cs.registers.size ()));
    if (start >= n)
      return;

    count = min (count, n - start);

    // Boolean constants come in as 32-bit BOOLs. We normalize each to 0 and 1
    // and store it in a 16-byte register so that all the sets have the same
    // layout.
    //
    if (c == vk_constant_set::bools)
    {
      const int32_t* bs (static_cast<const int32_t*> (data));

      for (uint32_t i (0); i != count; ++i)
        cs.registers[start + i] = {bs[i] != 0 ? 1u : 0u, 0, 0, 0};
    }
    else
      memcpy (cs.registers[start].data (), data, count * 16);

    ++cs.version; // See copy_constants().
  }

  void vk_context::
  set_vertex_declaration (vk_vertex_declaration* d)
  {
    if (d != declaration_)
    {
      assign (declaration_, d);
      dirty_ |= dirty_pipeline;
    }
  }

  void vk_context::
  set_stream_source (uint32_t i, vk_buffer* b, uint32_t offset, uint32_t stride)
  {
    if (i >= streams_.size ())
      return;

    stream& s (streams_[i]);
    assign (s.buffer, b);
    s.offset = offset;
    s.stride = stride;
    s.memory = VK_NULL_HANDLE; // Forget set_stream_memory() data, if any.
    s.memory_offset = 0;

    dirty_ |= dirty_streams;
  }

  void vk_context::
  set_stream_memory (uint32_t i, span<const unsigned char> d, uint32_t stride)
  {
    if (i >= streams_.size ())
      return;

    if (d.empty ())
    {
      set_stream_source (i, nullptr, 0, 0);
      return;
    }

    // Copy the data to the ring since the caller is free to change it once we
    // return.
    //
    transient_memory m (transient (d.size (), 16));
    memcpy (m.data, d.data (), d.size ());

    stream& s (streams_[i]);
    assign<vk_buffer> (s.buffer, nullptr);
    s.offset = 0;
    s.stride = stride;
    s.memory = m.buffer;
    s.memory_offset = m.offset;

    dirty_ |= dirty_streams;
  }

  void vk_context::
  set_indices (vk_buffer* b)
  {
    assign (indices_, b);
    dirty_ |= dirty_indices;
  }

  void vk_context::
  set_render_target (uint32_t i, vk_surface* s)
  {
    if (i >= targets_.size ())
      return;

    // A different surface means different attachments and, since their
    // formats go into the pipeline key, a different pipeline.
    //
    if (!same (targets_[i], s))
    {
      end_rendering ();
      dirty_ |= dirty_pipeline;
    }

    assign (targets_[i], s);

    // Like Direct3D 9, reset the viewport and the scissor rectangle to cover
    // the whole surface when render target 0 is set.
    //
    if (i == 0 && s != nullptr)
    {
      viewport_ = {0, 0, s->width (), s->height (), 0.0f, 1.0f};
      scissor_ = {0, 0,
                  static_cast<int32_t> (s->width ()),
                  static_cast<int32_t> (s->height ())};

      dirty_ |= dirty_viewport | dirty_scissor;
    }
  }

  void vk_context::
  set_depth_stencil (vk_surface* s)
  {
    if (!same (depth_, s))
    {
      end_rendering ();
      dirty_ |= dirty_pipeline | dirty_dynamic;
    }

    assign (depth_, s);
  }

  void vk_context::
  set_viewport (const d3d9::viewport& v)
  {
    viewport_ = v;
    dirty_ |= dirty_viewport;
  }

  void vk_context::
  set_scissor (const d3d9::rect& r)
  {
    scissor_ = r;
    dirty_ |= dirty_scissor;
  }

  void vk_context::
  clear (span<const d3d9::clear_rect> rects,
         uint32_t flags,
         d3d9::color color,
         float z,
         uint32_t stencil)
  {
    // D3DCOLOR is 0xAARRGGBB.
    //
    auto channel = [color] (uint32_t shift)
    {
      return static_cast<float> ((color >> shift) & 0xff) / 255.0f;
    };

    clear (rects,
           flags,
           {channel (16), channel (8), channel (0), channel (24)},
           z,
           stencil);
  }

  void vk_context::
  clear (span<const d3d9::clear_rect> rects,
         uint32_t flags,
         const array<float, 4>& color,
         float z,
         uint32_t stencil)
  {
    if (targets_[0] == nullptr && depth_ == nullptr)
      return;

    // We clear with vkCmdClearAttachments() since a Direct3D 9 clear may be
    // limited to rectangles and may come in the middle of rendering. Load
    // operations only clear whole attachments when rendering begins.
    //
    vector<VkClearAttachment> as;

    if ((flags & d3d9::clear_target) != 0)
    {
      VkClearValue v {};
      for (size_t i (0); i != color.size (); ++i)
        v.color.float32[i] = color[i];

      for (uint32_t i (0); i != targets_.size (); ++i)
      {
        if (targets_[i] != nullptr)
          as.push_back ({VK_IMAGE_ASPECT_COLOR_BIT, i, v});
      }
    }

    if (depth_ != nullptr)
    {
      const vk_format_info& fi (depth_->texture ().info ());

      VkImageAspectFlags a (0);
      if ((flags & d3d9::clear_zbuffer) != 0)
        a |= VK_IMAGE_ASPECT_DEPTH_BIT;

      if ((flags & d3d9::clear_stencil) != 0 && fi.stencil) // If has one.
        a |= VK_IMAGE_ASPECT_STENCIL_BIT;

      if (a != 0)
      {
        VkClearValue v {};
        v.depthStencil = {z, stencil};
        as.push_back ({a, 0, v});
      }
    }

    if (as.empty ())
      return;

    // Direct3D 9 limits the clear to the viewport and, with the scissor test
    // enabled, to the scissor rectangle. We also keep the rectangles within
    // the render area since Vulkan requires that.
    //
    VkExtent2D e (target_extent ());

    int32_t x0 (static_cast<int32_t> (viewport_.x));
    int32_t y0 (static_cast<int32_t> (viewport_.y));
    int32_t x1 (x0 + static_cast<int32_t> (viewport_.width));
    int32_t y1 (y0 + static_cast<int32_t> (viewport_.height));

    if (render_[d3d9::rs_scissortestenable])
    {
      x0 = max (x0, scissor_.left);
      y0 = max (y0, scissor_.top);
      x1 = min (x1, scissor_.right);
      y1 = min (y1, scissor_.bottom);
    }

    x0 = max<int32_t> (x0, 0);
    y0 = max<int32_t> (y0, 0);
    x1 = min<int32_t> (x1, static_cast<int32_t> (e.width));
    y1 = min<int32_t> (y1, static_cast<int32_t> (e.height));

    vector<VkClearRect> rs;

    auto add = [&rs, x0, y0, x1, y1] (int32_t l,
                                      int32_t t,
                                      int32_t r,
                                      int32_t b)
    {
      l = max (l, x0);
      t = max (t, y0);
      r = min (r, x1);
      b = min (b, y1);

      if (l < r && t < b)
      {
        VkClearRect c {};
        c.rect.offset = {l, t};
        c.rect.extent = {static_cast<uint32_t> (r - l),
                         static_cast<uint32_t> (b - t)};
        c.layerCount = 1;
        rs.push_back (c);
      }
    };

    if (rects.empty ()) // Clear the whole area.
      add (x0, y0, x1, y1);
    else
    {
      for (const d3d9::clear_rect& r: rects)
        add (r.x1, r.y1, r.x2, r.y2);
    }

    if (rs.empty ())
      return;

    begin_rendering ();

    vkCmdClearAttachments (slots_[slot_].commands,
                           static_cast<uint32_t> (as.size ()), as.data (),
                           static_cast<uint32_t> (rs.size ()), rs.data ());
  }

  void vk_context::
  draw (d3d9::primitive_type t, uint32_t start, uint32_t primitives)
  {
    if (primitives == 0 || !prepare (t))
      return;

    vkCmdDraw (slots_[slot_].commands,
               vk_vertex_count (t, primitives), 1,
               start, 0);
  }

  void vk_context::
  draw_indexed (d3d9::primitive_type t,
                int32_t base,
                uint32_t start,
                uint32_t primitives)
  {
    if (primitives == 0 || indices_ == nullptr || !prepare (t))
      return;

    VkCommandBuffer c (slots_[slot_].commands);

    // Compare the handles since renaming the buffer gives it a new one (see
    // vk_buffer::lock()).
    //
    if (VkBuffer b = indices_->handle (); b != bound_indices_ ||
                                          (dirty_ & dirty_indices) != 0)
    {
      vkCmdBindIndexBuffer (c, b, 0, indices_->index_type ());
      bound_indices_ = b;
      dirty_ &= ~dirty_indices;
    }

    indices_->use (device_.frame ()); // See vk_buffer::lock().

    vkCmdDrawIndexed (c,
                      vk_vertex_count (t, primitives), 1,
                      start, base, 0);
  }

  void vk_context::
  draw_user (d3d9::primitive_type t,
             uint32_t primitives,
             const void* vertices,
             uint32_t stride)
  {
    if (primitives == 0)
      return;

    uint32_t n (vk_vertex_count (t, primitives));
    VkDeviceSize size (static_cast<VkDeviceSize> (n) * stride);

    transient_memory m (transient (size, 16));
    memcpy (m.data, vertices, static_cast<size_t> (size));

    // Direct3D 9 leaves stream 0 unset after DrawPrimitiveUP() and so do we.
    // The vertices are bound directly below and we dirty the streams so that
    // the next draw binds the stream sources again.
    //
    set_stream_source (0, nullptr, 0, 0);

    if (!prepare (t))
      return;

    VkCommandBuffer c (slots_[slot_].commands);
    VkDeviceSize s (stride);

    vkCmdBindVertexBuffers2 (c, 0, 1, &m.buffer, &m.offset, nullptr, &s);
    vkCmdDraw (c, n, 1, 0, 0);

    dirty_ |= dirty_streams;
  }

  void vk_context::
  issue (vk_query& q, uint32_t flags)
  {
    // We consider an event query signaled once its frame completes. This
    // seems to be good enough for IW4.
    //
    if (q.type () == d3d9::querytype_event)
    {
      if ((flags & d3d9::issue_end) != 0)
      {
        q.status = vk_query::state::issued;
        q.frame = device_.frame ();
      }

      return;
    }

    VkDevice h (device_.handle ());

    if ((flags & d3d9::issue_begin) != 0)
    {
      // Beginning a query that is still building restarts it.
      //
      if (q.status == vk_query::state::building)
      {
        auto i (ranges::find (active_queries_, &q));
        if (i != active_queries_.end ())
        {
          vkCmdEndQuery (slots_[slot_].commands, q.pool (), 0);
          active_queries_.erase (i);
        }
      }

      // The query has to be reset before it begins. If the GPU is done with
      // it, then we reset it from the host. Otherwise, we record the reset
      // into the command buffer, outside rendering (see commands()).
      //
      device_.completed ();

      if (q.frame <= device_.completed_frame () &&
          q.status != vk_query::state::building)
        vkResetQueryPool (h, q.pool (), 0, 1);
      else
      {
        VkCommandBuffer c (commands ());
        vkCmdResetQueryPool (c, q.pool (), 0, 1);
      }

      // In Vulkan a query that begins in a rendering instance has to end in
      // it, so end_rendering() ends the active queries.
      //
      // @@ This means a query that spans a render target change misses the
      //    samples drawn after the change. Should we restart such queries?
      //    Probably not an issue for IW4, but we haven't checked.
      //
      begin_rendering ();

      vkCmdBeginQuery (slots_[slot_].commands, q.pool (), 0, 0);
      active_queries_.push_back (&q);

      q.status = vk_query::state::building;
      q.frame = device_.frame ();
    }
    else if ((flags & d3d9::issue_end) != 0)
    {
      auto i (ranges::find (active_queries_, &q));
      if (i != active_queries_.end ())
      {
        vkCmdEndQuery (slots_[slot_].commands, q.pool (), 0);
        active_queries_.erase (i);
      }

      if (q.status == vk_query::state::building)
      {
        q.status = vk_query::state::issued;
        q.frame = device_.frame ();
      }
    }
  }

  bool vk_context::
  query_data (vk_query& q, void* data, uint32_t size, bool flush)
  {
    if (q.status != vk_query::state::issued)
      return q.status == vk_query::state::idle;

    if (q.frame > device_.completed_frame ())
    {
      device_.completed ();

      if (q.frame > device_.completed_frame ())
      {
        // With D3DGETDATA_FLUSH, submit the frame being recorded. Otherwise a
        // caller that polls for a query from this frame would wait forever.
        //
        if (flush && q.frame == device_.frame ())
          submit ();

        return false;
      }
    }

    if (q.type () == d3d9::querytype_occlusion)
    {
      uint64_t n (0);
      VkResult r (vkGetQueryPoolResults (device_.handle (),
                                         q.pool (), 0, 1,
                                         sizeof (n), &n, sizeof (n),
                                         VK_QUERY_RESULT_64_BIT));

      if (r == VK_NOT_READY)
        return false;

      vk_check (r, "unable to read occlusion query");
      q.result = static_cast<uint32_t> (min<uint64_t> (n, ~0u)); // DWORD.

      if (data != nullptr)
        memcpy (data, &q.result, min<size_t> (size, sizeof (q.result)));
    }
    else if (data != nullptr && size != 0)
    {
      int32_t b (1); // Event queries return TRUE.
      memcpy (data, &b, min<size_t> (size, sizeof (b)));
    }

    return true;
  }

  VkCommandBuffer vk_context::
  commands ()
  {
    VkCommandBuffer c (begin_commands ());
    end_rendering ();
    return c;
  }

  uint64_t vk_context::
  submit (const vk_device::frame_sync& fs)
  {
    VkCommandBuffer c (begin_commands ());
    end_rendering ();

    vk_check (vkEndCommandBuffer (c), "unable to end command buffer");
    recording_ = false;

    slot& s (slots_[slot_]);
    s.serial = device_.submit_frame (c, fs);
    // Move to the next slot (begin_commands() waits for its previous
    // submission).
    //
    slot_ = (slot_ + 1) % slots_.size ();

    device_.completed ();
    ring_.reclaim (device_.completed_frame ());

    // Every 1024 frames, trim the pipelines that haven't been used for 4096
    // frames. Otherwise the cache keeps growing as IW4 goes through new state
    // combinations.
    //
    uint64_t f (device_.frame ());
    if (f % 1024 == 0 && f > 4096)
      pipelines_.trim (f - 4096);

    return s.serial;
  }

  void vk_context::
  transition (vk_texture& t, VkImageLayout l)
  {
    if (t.layout () == l)
      return;

    vk_transition (commands (), t.image (), t.range (), t.layout (), l);
    t.layout () = l;
  }

  vk_context::transient_memory vk_context::
  transient (VkDeviceSize size, VkDeviceSize alignment)
  {
    uint64_t f (device_.frame ());
    optional<uint64_t> o (ring_.allocate (size, alignment, f));

    // If the ring is full, switch to a new one that is twice as large (and at
    // least twice the request). The old ring is retired (see create_ring()).
    //
    if (!o)
    {
      create_ring (max (ring_.size () * 2, size * 2));
      o = ring_.allocate (size, alignment, f);
    }

    return {ring_buffer_,
            *o,
            static_cast<unsigned char*> (ring_memory_.mapped) + *o};
  }

  VkCommandBuffer vk_context::
  begin_commands ()
  {
    slot& s (slots_[slot_]);

    if (!recording_)
    {
      // Wait for the slot's previous submission to complete before resetting
      // its pool.
      //
      if (s.serial != 0)
        device_.wait (s.serial);

      vk_check (vkResetCommandPool (device_.handle (), s.pool, 0),
                "unable to reset command pool");

      VkCommandBufferBeginInfo bi {};
      bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

      vk_check (vkBeginCommandBuffer (s.commands, &bi),
                "unable to begin command buffer");

      // A new command buffer has nothing bound, so everything is dirty.
      //
      recording_ = true;
      dirty_ = dirty_all;
      bound_pipeline_ = VK_NULL_HANDLE;
      bound_indices_ = VK_NULL_HANDLE;
      topology_ = VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
    }

    return s.commands;
  }

  VkExtent2D vk_context::
  target_extent () const noexcept
  {
    if (const vk_surface* s = targets_[0] != nullptr ? targets_[0] : depth_)
      return {s->width (), s->height ()};

    return {0, 0};
  }

  void vk_context::
  begin_rendering ()
  {
    if (rendering_)
      return;

    VkCommandBuffer c (begin_commands ());

    // We always load and store the attachments since Direct3D 9 has no
    // notion of render passes and we can't tell what IW4 will do with the
    // contents next.
    //
    VkRenderingAttachmentInfo colors[vk_render_targets] {};
    uint32_t n (0);

    for (vk_surface* s: targets_)
    {
      if (s == nullptr) // Assume contiguous targets.
        break;

      vk_texture& t (s->texture ());
      transition (t, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

      VkRenderingAttachmentInfo& a (colors[n++]);
      a.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
      a.imageView = t.attachment_view (s->face (),
                                       s->level (),
                                       render_[d3d9::rs_srgbwriteenable] != 0);
      a.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      a.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      a.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    }

    VkRenderingAttachmentInfo depth {};
    bool stencil (false);

    if (depth_ != nullptr)
    {
      vk_texture& t (depth_->texture ());
      transition (t, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

      depth.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
      depth.imageView = t.attachment_view (depth_->face (), depth_->level ());
      depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      depth.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

      stencil = t.info ().stencil; // Same attachment.
    }

    VkRenderingInfo ri {};
    ri.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    ri.renderArea.extent = target_extent ();
    ri.layerCount = 1;
    ri.colorAttachmentCount = n;
    ri.pColorAttachments = colors;
    ri.pDepthAttachment = depth_ != nullptr ? &depth : nullptr;
    ri.pStencilAttachment = stencil ? &depth : nullptr;

    vkCmdBeginRendering (c, &ri);
    rendering_ = true;
  }

  void vk_context::
  end_rendering ()
  {
    if (!rendering_)
      return;

    VkCommandBuffer c (slots_[slot_].commands);

    // See issue().
    //
    for (vk_query* q: active_queries_)
      vkCmdEndQuery (c, q->pool (), 0);

    active_queries_.clear ();

    vkCmdEndRendering (c);
    rendering_ = false;
  }

  void vk_context::
  collect_textures (array<vk_texture*, 20>& ts)
  {
    // Collect the textures that the current shaders sample. Where the sampler
    // is unset and where the texture type doesn't match the shader's
    // declaration, we substitute a black placeholder. We do the same for a
    // texture that is also an attachment since Vulkan doesn't allow sampling
    // it (Direct3D 9 drivers seem to tolerate this).
    //
    ts.fill (nullptr);

    auto rendered = [this] (const vk_texture& t)
    {
      if (depth_ != nullptr && &depth_->texture () == &t)
        return true;

      for (const vk_surface* s: targets_)
      {
        if (s != nullptr && &s->texture () == &t)
          return true;
      }

      return false;
    };

    auto collect = [this, &ts, &rendered] (const vk_shader* s, size_t first)
    {
      const vk_d3dbc_info& i (s->info ());

      for (uint32_t m (i.samplers); m != 0; m &= m - 1)
      {
        uint32_t n (static_cast<uint32_t> (countr_zero (m)));
        size_t slot (first + n);

        if (slot >= ts.size ())
          continue;

        vk_texture* t (textures_[slot]);

        auto matches = [] (const vk_texture& x, vk_sampler_type s)
        {
          switch (s)
          {
            case vk_sampler_type::plane:  return x.desc ().type == vk_texture_desc::kind::plane;
            case vk_sampler_type::cube:   return x.desc ().type == vk_texture_desc::kind::cube;
            case vk_sampler_type::volume: return x.desc ().type == vk_texture_desc::kind::volume;
            case vk_sampler_type::none:   return true; // Undeclared.
          }

          return true;
        };

        if (t == nullptr || rendered (*t) || !matches (*t, i.sampler_types[n]))
          t = placeholders_[static_cast<size_t> (i.sampler_types[n])];

        ts[slot] = t;
      }
    };

    collect (pixel_shader_, 0);
    collect (vertex_shader_, vertex_slot);
  }

  bool vk_context::
  prepare (d3d9::primitive_type t)
  {
    if (vertex_shader_ == nullptr ||
        pixel_shader_ == nullptr  ||
        declaration_ == nullptr   ||
        (targets_[0] == nullptr && depth_ == nullptr))
      return false;

    array<vk_texture*, 20> ts;
    collect_textures (ts);

    // Direct3D 9 drivers sample depth textures with comparison (hardware PCF)
    // and IW4's shadow maps seem to rely on this. So depth textures in pixel
    // samplers get compare samplers (see push_descriptors()) and the
    // pipeline key records which samplers they are.
    //
    uint32_t shadow (0);
    for (size_t i (0); i != vertex_slot; ++i)
    {
      if (ts[i] != nullptr && ts[i]->info ().depth)
        shadow |= 1u << i;
    }

    if (shadow != shadow_samplers_)
    {
      shadow_samplers_ = shadow;
      dirty_ |= dirty_pipeline;
    }

    // Do the layout transitions before we begin rendering since a transition
    // ends the rendering instance (see commands()). Textures in system memory
    // have no image to transition.
    //
    for (vk_texture* x: ts)
    {
      if (x != nullptr &&
          !x->host () &&
          x->layout () != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        transition (*x, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    begin_rendering ();

    bind_pipeline (t);
    set_dynamic_state ();
    push_descriptors (ts);
    bind_streams ();

    return true;
  }

  void vk_context::
  bind_pipeline (d3d9::primitive_type t)
  {
    VkCommandBuffer c (slots_[slot_].commands);

    // The topology is dynamic state so the pipeline only depends on the
    // topology class.
    //
    vk_topology_class tc (vk_topology_class_of (t));

    if ((dirty_ & dirty_pipeline) != 0 ||
        pipeline_ == nullptr            ||
        pipeline_class_ != tc)
    {
      vk_pipeline_key k;
      k.vertex_shader = vertex_shader_->id ();
      k.pixel_shader = pixel_shader_->id ();
      k.declaration = declaration_->id ();
      k.alpha = vk_alpha_test_state (render_);
      k.shadow_samplers = shadow_samplers_;
      k.topology = tc;

      for (uint32_t i (0); i != targets_.size () && targets_[i] != nullptr; ++i)
      {
        const vk_texture& x (targets_[i]->texture ());

        bool alpha (x.info ().swizzle.a != VK_COMPONENT_SWIZZLE_ONE);

        k.colors[i] = x.attachment_format (
          render_[d3d9::rs_srgbwriteenable] != 0);
        k.blend[i] = vk_blend_key (vk_blend_attachment (render_, i, alpha));
        k.samples = x.desc ().samples;
      }

      if (depth_ != nullptr)
      {
        const vk_texture& x (depth_->texture ());

        k.depth = x.format ();
        k.stencil = x.info ().stencil;
        k.samples = x.desc ().samples;
      }

      pipeline_ = &pipelines_.get (k,
                                   *vertex_shader_,
                                   *pixel_shader_,
                                   *declaration_,
                                   device_.frame ());
      pipeline_class_ = tc;
      dirty_ &= ~dirty_pipeline;
    }

    if (pipeline_->handle != bound_pipeline_)
    {
      vkCmdBindPipeline (c, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_->handle);
      bound_pipeline_ = pipeline_->handle;

      dirty_ |= dirty_streams; // Bindings may differ (see bind_streams()).
    }

    if (VkPrimitiveTopology p = vk_topology (t); p != topology_)
    {
      vkCmdSetPrimitiveTopology (c, p);
      topology_ = p;
    }
  }

  void vk_context::
  set_dynamic_state ()
  {
    VkCommandBuffer c (slots_[slot_].commands);
    VkExtent2D e (target_extent ());

    if ((dirty_ & dirty_viewport) != 0)
    {
      // Vulkan puts pixel centers halfway between integer coordinates while
      // Direct3D 9 puts them on the integers, hence the half pixel shift. The
      // negative height flips Y to match Direct3D's clip space.
      //
      VkViewport v {};
      v.x = static_cast<float> (viewport_.x) + 0.5f;
      v.y = static_cast<float> (viewport_.y + viewport_.height) + 0.5f;
      v.width = static_cast<float> (viewport_.width);
      v.height = -static_cast<float> (viewport_.height);
      v.minDepth = viewport_.min_z;
      v.maxDepth = viewport_.max_z;

      vkCmdSetViewport (c, 0, 1, &v);
      dirty_ &= ~dirty_viewport;
    }

    if ((dirty_ & dirty_scissor) != 0)
    {
      // Vulkan doesn't allow negative scissor offsets, so clamp the rectangle
      // to the target.
      //
      int32_t x0 (0), y0 (0);
      int32_t x1 (static_cast<int32_t> (e.width));
      int32_t y1 (static_cast<int32_t> (e.height));

      if (render_[d3d9::rs_scissortestenable])
      {
        x0 = clamp (scissor_.left, 0, x1);
        y0 = clamp (scissor_.top, 0, y1);
        x1 = clamp (scissor_.right, x0, x1);
        y1 = clamp (scissor_.bottom, y0, y1);
      }

      VkRect2D r {};
      r.offset = {x0, y0};
      r.extent = {static_cast<uint32_t> (x1 - x0),
                  static_cast<uint32_t> (y1 - y0)};

      vkCmdSetScissor (c, 0, 1, &r);
      dirty_ &= ~dirty_scissor;
    }

    if ((dirty_ & dirty_dynamic) == 0)
      return;

    // Without a depth buffer we disable the depth test and without a stencil
    // aspect the stencil test.
    //
    bool depth (depth_ != nullptr);
    bool stencil (depth && depth_->texture ().info ().stencil);

    vkCmdSetDepthTestEnable (
      c, depth && render_[d3d9::rs_zenable] != d3d9::zb_false);
    vkCmdSetDepthWriteEnable (c, depth && render_[d3d9::rs_zwriteenable]);
    vkCmdSetDepthCompareOp (
      c, vk_compare_op (static_cast<d3d9::cmp_func> (render_[d3d9::rs_zfunc])));

    vk_stencil s (vk_stencil_state (render_));
    vkCmdSetStencilTestEnable (c, stencil && s.enable);

    auto face = [c] (VkStencilFaceFlags f, const VkStencilOpState& o)
    {
      vkCmdSetStencilOp (c, f, o.failOp, o.passOp, o.depthFailOp, o.compareOp);
      vkCmdSetStencilCompareMask (c, f, o.compareMask);
      vkCmdSetStencilWriteMask (c, f, o.writeMask);
      vkCmdSetStencilReference (c, f, o.reference);
    };

    face (VK_STENCIL_FACE_FRONT_BIT, s.front);
    face (VK_STENCIL_FACE_BACK_BIT, s.back);

    vkCmdSetCullMode (
      c, vk_cull_mode (static_cast<d3d9::cull> (render_[d3d9::rs_cullmode])));
    vkCmdSetFrontFace (c, vk_front_face);

    array<float, 2> b (vk_depth_bias (render_,
                                      depth
                                      ? depth_->texture ().format ()
                                      : VK_FORMAT_UNDEFINED));

    vkCmdSetDepthBiasEnable (c, b[0] != 0.0f || b[1] != 0.0f);
    vkCmdSetDepthBias (c, b[0], 0.0f, b[1]);

    array<float, 4> k (vk_blend_constants (render_));
    vkCmdSetBlendConstants (c, k.data ());

    dirty_ &= ~dirty_dynamic;
  }

  VkDescriptorBufferInfo vk_context::
  copy_constants (vk_shader_stage s, vk_constant_set c, uint32_t count)
  {
    constant_set& cs (constants_[static_cast<size_t> (s)]
                                [static_cast<size_t> (c)]);

    uint64_t f (device_.frame ());

    // Copy the set into the ring unless it is unchanged and was already
    // copied this frame with enough registers. We have to copy again in a new
    // frame since the ring may have reclaimed the old copy.
    //
    if (cs.copied != cs.version  ||
        cs.copied_frame != f     ||
        cs.copied_count < count)
    {
      VkDeviceSize size (static_cast<VkDeviceSize> (count) * 16);

      transient_memory m (
        transient (size,
                   device_.limits ().minUniformBufferOffsetAlignment));

      memcpy (m.data, cs.registers.data (), static_cast<size_t> (size));

      cs.copied = cs.version;
      cs.copied_frame = f;
      cs.copied_count = count;
      cs.buffer = m.buffer;
      cs.offset = m.offset;
    }

    return {cs.buffer, cs.offset, static_cast<VkDeviceSize> (count) * 16};
  }

  void vk_context::
  push_descriptors (const array<vk_texture*, 20>& ts)
  {
    VkCommandBuffer c (slots_[slot_].commands);

    // We use push descriptors so there are no descriptor sets to allocate and
    // recycle.
    //
    array<VkDescriptorBufferInfo, 6> buffers;
    array<VkDescriptorImageInfo, 20> images;
    array<VkWriteDescriptorSet, 26> writes;
    uint32_t nb (0), ni (0), nw (0);

    auto write = [&writes, &nw] (uint32_t binding, VkDescriptorType type)
      -> VkWriteDescriptorSet&
    {
      VkWriteDescriptorSet& w (writes[nw++]);
      w = {};
      w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      w.dstBinding = binding;
      w.descriptorCount = 1;
      w.descriptorType = type;
      return w;
    };

    auto constants = [this, &write, &buffers, &nb] (vk_shader_stage s,
                                                     const vk_shader& x)
    {
      const vk_d3dbc_info& i (x.info ());

      const pair<vk_constant_set, uint32_t> sets[] =
      {
        {vk_constant_set::floats, i.floats},
        {vk_constant_set::ints,   i.ints},
        {vk_constant_set::bools,  i.bools}
      };

      for (const auto& p: sets)
      {
        if (p.second == 0)
          continue;

        VkDescriptorBufferInfo& b (buffers[nb++]);
        b = copy_constants (s, p.first, p.second);

        write (vk_constant_binding (s, p.first),
               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER).pBufferInfo = &b;
      }
    };

    constants (vk_shader_stage::vertex, *vertex_shader_);
    constants (vk_shader_stage::pixel, *pixel_shader_);

    for (size_t i (0); i != ts.size (); ++i)
    {
      vk_texture* t (ts[i]);
      if (t == nullptr)
        continue;

      if (t->host ()) // No image (see prepare()).
        continue;

      VkDescriptorImageInfo& ii (images[ni++]);
      vk_sampler_key sk (vk_sampler_key_of (sampler_states_[i]));
      sk.compare = i < vertex_slot && t->info ().depth; // See prepare().

      ii.sampler = samplers_.get (sk);
      ii.imageView = t->view (
        sampler_states_[i][d3d9::samp_srgbtexture] != 0);
      ii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

      bool pixel (i < vertex_slot);
      uint32_t b (vk_sampler_binding (
                    pixel ? vk_shader_stage::pixel : vk_shader_stage::vertex,
                    static_cast<uint32_t> (pixel ? i : i - vertex_slot)));

      write (b, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER).pImageInfo = &ii;
    }

    if (nw != 0)
      device_.push_descriptor_set (c,
                                   VK_PIPELINE_BIND_POINT_GRAPHICS,
                                   pipelines_.layout (),
                                   0,
                                   nw, writes.data ());
  }

  void vk_context::
  bind_streams ()
  {
    VkCommandBuffer c (slots_[slot_].commands);
    uint64_t f (device_.frame ());

    for (uint32_t m (pipeline_->bindings); m != 0; m &= m - 1)
    {
      uint32_t b (static_cast<uint32_t> (countr_zero (m)));

      VkBuffer h;
      VkDeviceSize o, s;

      if (b == vk_default_binding)
      {
        h = default_buffer_;
        o = 0;
        s = 0;
      }
      else
      {
        const stream& x (streams_[b]);

        if (x.buffer == nullptr && x.memory != VK_NULL_HANDLE)
        {
          h = x.memory;
          o = x.memory_offset;
          s = x.stride;
        }
        else if (x.buffer == nullptr) // Unset, as for missing elements.
        {
          h = default_buffer_;
          o = 0;
          s = 0;
        }
        else
        {
          h = x.buffer->handle ();
          o = x.offset;
          s = x.stride;
          x.buffer->use (f); // See vk_buffer::lock().
        }
      }

      vkCmdBindVertexBuffers2 (c, b, 1, &h, &o, nullptr, &s);
    }

    dirty_ &= ~dirty_streams;
  }

  void vk_context::
  create_placeholders ()
  {
    using kind = vk_texture_desc::kind;

    const pair<vk_sampler_type, kind> types[] =
    {
      {vk_sampler_type::plane,  kind::plane},
      {vk_sampler_type::cube,   kind::cube},
      {vk_sampler_type::volume, kind::volume}
    };

    // Create the placeholders for collect_textures(). Opaque black seems the
    // least noticeable choice.
    //
    const uint32_t black (0xff000000);

    for (const auto& p: types)
    {
      vk_texture_desc d;
      d.type = p.second;
      d.format = d3d9::fmt_a8r8g8b8;
      d.levels = 1;

      vk_texture* t (new vk_texture (device_, d));
      placeholders_[static_cast<size_t> (p.first)] = t;

      if (p.second == kind::volume)
      {
        d3d9::locked_box b (t->lock_box (0, nullptr, 0));
        memcpy (b.bits, &black, sizeof (black));
        t->unlock_box (0);
      }
      else
      {
        for (uint32_t f (0); f != t->faces (); ++f)
        {
          d3d9::locked_rect r (t->lock_rect (f, 0, nullptr, 0));
          memcpy (r.bits, &black, sizeof (black));
          t->unlock_rect (f, 0);
        }
      }
    }
  }

  void vk_context::
  create_ring (VkDeviceSize size)
  {
    // Besides constants and vertices, the ring stages texture uploads, hence
    // the transfer usage.
    //
    // @@ Nothing puts indices into the ring currently. Maybe drop the index
    //    usage?
    //
    // @@ vk_buffer prefers cached system memory for dynamic buffers since CPU
    //    writes into device-local memory seem slow. Should the ring do the
    //    same?
    //
    auto r (device_.create_buffer (size,
                                   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

    // Frames in flight may still read the old ring, so retire it.
    //
    if (ring_buffer_ != VK_NULL_HANDLE)
    {
      vk_device& d (device_);
      VkBuffer b (ring_buffer_);
      vk_allocation m (ring_memory_);

      device_.retire ([&d, b, m] () {d.destroy_buffer (b, m);});
    }

    ring_buffer_ = r.first;
    ring_memory_ = r.second;
    ring_ = vk_ring (size);
  }
}
