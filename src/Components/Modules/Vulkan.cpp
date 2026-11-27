#include "Vulkan.hpp"

#include "Events.hpp"

#include <Vulkan/vk_buffer.hpp>
#include <Vulkan/vk_context.hpp>
#include <Vulkan/vk_device.hpp>
#include <Vulkan/vk_error.hpp>
#include <Vulkan/vk_format.hpp>
#include <Vulkan/vk_material_state.hpp>
#include <Vulkan/vk_shader.hpp>
#include <Vulkan/vk_swapchain.hpp>
#include <Vulkan/vk_texture.hpp>
#include <Vulkan/vk_vertex_declaration.hpp>
#include <Vulkan/vk_window.hpp>

#include <span>
#include <concepts>  // invocable, same_as
#include <vector>
#include <cstdint>
#include <cstring>   // memcpy()
#include <array>
#include <mutex>
#include <atomic>
#include <memory>
#include <optional>
#include <exception>
#include <algorithm> // clamp(), ranges::{find,any_of,upper_bound}()
#include <string_view>
#include <unordered_map>

using namespace std;

namespace Components
{
  namespace
  {
    using namespace Vulkan;

    constexpr uintptr_t r_create_device (0x140031D70);
    constexpr uintptr_t r_shutdown      (0x140032BF0);

    constexpr uintptr_t rb_swap_buffers  (0x14004FCF0);
    constexpr uintptr_t rb_swap_present  (0x14004FD3E);

    constexpr uintptr_t r_draw_tess_technique (0x14003A570);
    constexpr uintptr_t r_get_code_matrix     (0x140068770);

    constexpr uintptr_t r_set_render_target (0x140069340);
    constexpr uintptr_t r_set_viewport      (0x1400690E0);
    constexpr uintptr_t r_clear_screen      (0x1400694F0);

    // IW4's render target (see render_targets).
    //
    struct render_target
    {
      Game::GfxImage* image;
      IDirect3DSurface9* surface;
      IDirect3DSurface9* depth_stencil;
      uint32_t width;
      uint32_t height;
    };

    static_assert (sizeof (render_target) == 32);

    constexpr uintptr_t render_targets (0x149139100);

    constexpr uintptr_t r_polygon_offset_scale  (0x148CC4848);
    constexpr uintptr_t r_polygon_offset_bias   (0x148CC4930);
    constexpr uintptr_t sm_polygon_offset_scale (0x148CC49F0);
    constexpr uintptr_t sm_polygon_offset_bias  (0x148CC47C0);
    constexpr uintptr_t slope_scaled_depth_bias (0x148CCA85D);

    // Return the depth bias for IW4's polygon offset dvars.
    //
    // We try to match what IW4 sets for D3DRS_DEPTHBIAS. As far as we can
    // tell, the bias dvars are in 1/65536 units and, if the device doesn't
    // support slope-scaled depth bias, IW4 doubles the constant bias and
    // uses no slope bias.
    //
    vk_polygon_offsets
    polygon_offsets ()
    {
      auto value = [] (uintptr_t a)
      {
        return (*reinterpret_cast<Game::dvar_t* const*> (a))->current.value;
      };

      const float unit (1.0f / 65536.0f);

      vk_polygon_offsets r;
      r.bias = value (r_polygon_offset_bias) * unit;
      r.scale = value (r_polygon_offset_scale);
      r.shadow_bias = value (sm_polygon_offset_bias) * unit;
      r.shadow_scale = value (sm_polygon_offset_scale);

      if (!*reinterpret_cast<const bool*> (slope_scaled_depth_bias))
      {
        r.bias *= 2.0f;
        r.shadow_bias *= 2.0f;
        r.scale = 0.0f;
        r.shadow_scale = 0.0f;
      }

      return r;
    }
    constexpr uint32_t render_target_count (12);

    constexpr uintptr_t r_setup_pass (0x14006BD30);

    // Call sites that we replace (see replace_call()). The draw calls are
    // DrawIndexedPrimitive() calls and the custom texture calls are
    // SetTexture() calls for samplers 1-3 (see renderer::custom_texture()).
    // The other names say which method is called.
    //
    // Note that we only see IW4's draws at these call sites and in
    // R_DrawTessTechnique(). If IW4 draws from anywhere else, then the draw
    // goes to the stubbed out device (see patch_device()) and doesn't show
    // up.
    //
    constexpr uintptr_t draw_calls[]
    {
      0x1400550D9, 0x140073DD6, 0x140073EFD, 0x14007415D, 0x14007435A,
      0x140074494, 0x14007450C, 0x1400746F7, 0x1400748AF, 0x140074931,
      0x140074FEB, 0x1400750DB, 0x1400751DB, 0x1400764A2, 0x1400768DC,
      0x1400773DF, 0x1400775BC, 0x140077652, 0x140077A92, 0x14007ADC9,
      0x14007B040, 0x14007B226, 0x14007B3F1, 0x14007B5CA, 0x14007BB9A,
      0x14007C0B1, 0x14007C583, 0x14007C763, 0x14007C896, 0x14007C9ED,
      0x14007CA93, 0x14007CC7C
    };

    constexpr uintptr_t custom_texture_calls[]
    {
      0x14007617A, 0x140077298, 0x1400772D7, 0x140077302, 0x14007B888,
      0x14007C7B2, 0x14007C7EE, 0x14007C81E
    };

    constexpr uintptr_t viewport_calls[]
    {
      0x14004794E, 0x140076293
    };

    constexpr uintptr_t create_device_call (0x140031B68);

    constexpr uintptr_t copy_calls[]
    {
      0x14004BEA4, 0x14006357D, 0x1400738CC
    };

    constexpr uintptr_t update_texture_call (0x14003E384);

    constexpr uintptr_t gamma_ramp_call (0x14004FF9E);

    constexpr uintptr_t r_alloc_static_vertex_buffer (0x140038A30);
    constexpr uintptr_t r_alloc_static_index_buffer  (0x140038B20);

    // IW4's tess buffers, which hold immediate geometry for
    // R_DrawTessTechnique() (see renderer::draw_tess()).
    //
    constexpr uintptr_t tess_vertices     (0x148F0E7A0);
    constexpr uintptr_t tess_indices      (0x148F390E0);
    constexpr uintptr_t tess_index_count  (0x1491390E8);
    constexpr uintptr_t tess_vertex_count (0x1491390EC);

    constexpr int swap_present (0x2); // RB_SwapBuffers() flag.

    using r_create_device_t       = bool         (Game::GfxWindowParms*);
    using r_shutdown_t            = void         (int destroy_window);
    using rb_swap_buffers_t       = void         (int flags);
    using r_draw_tess_technique_t = void         (const Game::GfxCmdBufContext*,
                                                  const Game::GfxDrawPrimArgs*);
    using r_get_code_matrix_t     = const float* (Game::GfxCmdBufSourceState*,
                                                  unsigned int index,
                                                  unsigned int first_row);

    r_create_device_t*       r_create_device_original       (nullptr);
    r_shutdown_t*            r_shutdown_original            (nullptr);
    rb_swap_buffers_t*       rb_swap_buffers_original       (nullptr);
    r_draw_tess_technique_t* r_draw_tess_technique_original (nullptr);

    using r_set_render_target_t = void (const Game::GfxCmdBufContext*,
                                        Game::GfxRenderTargetId);
    using r_set_viewport_t      = void (Game::GfxCmdBufState*,
                                        const Game::GfxViewport*);
    using r_clear_screen_t      = void (IDirect3DDevice9*,
                                        uint8_t which,
                                        const float* color,
                                        float depth,
                                        uint8_t stencil);

    r_set_render_target_t* r_set_render_target_original (nullptr);
    r_set_viewport_t*      r_set_viewport_original      (nullptr);
    r_clear_screen_t*      r_clear_screen_original      (nullptr);

    using r_setup_pass_t                 = void  (const Game::GfxCmdBufContext*, unsigned int pass);
    using r_alloc_static_vertex_buffer_t = void* (IDirect3DVertexBuffer9**,      int size);
    using r_alloc_static_index_buffer_t  = void* (IDirect3DIndexBuffer9**,       int size);

    r_setup_pass_t*                 r_setup_pass_original                 (nullptr);
    r_alloc_static_vertex_buffer_t* r_alloc_static_vertex_buffer_original (nullptr);
    r_alloc_static_index_buffer_t*  r_alloc_static_index_buffer_original  (nullptr);

    using vertex_buffer_lock_t = HRESULT STDMETHODCALLTYPE (IDirect3DVertexBuffer9*,
                                                            UINT offset,
                                                            UINT size,
                                                            void** data,
                                                            DWORD flags);
    using index_buffer_lock_t  = HRESULT STDMETHODCALLTYPE (IDirect3DIndexBuffer9*,
                                                            UINT offset,
                                                            UINT size,
                                                            void** data,
                                                            DWORD flags);
    using buffer_unlock_t      = HRESULT STDMETHODCALLTYPE (IUnknown*);
    using texture_unlock_t     = HRESULT STDMETHODCALLTYPE (IDirect3DTexture9*,
                                                            UINT level);
    using cube_unlock_t        = HRESULT STDMETHODCALLTYPE (IDirect3DCubeTexture9*,
                                                            D3DCUBEMAP_FACES,
                                                            UINT level);
    using volume_unlock_t      = HRESULT STDMETHODCALLTYPE (IDirect3DVolumeTexture9*,
                                                            UINT level);

    texture_unlock_t* texture_unlock_original (nullptr);
    cube_unlock_t*    cube_unlock_original    (nullptr);
    volume_unlock_t*  volume_unlock_original  (nullptr);

    vertex_buffer_lock_t* vertex_buffer_lock_original   (nullptr);
    buffer_unlock_t*      vertex_buffer_unlock_original (nullptr);
    index_buffer_lock_t*  index_buffer_lock_original    (nullptr);
    buffer_unlock_t*      index_buffer_unlock_original  (nullptr);

    template <typename S>
    concept d3d9_shader = same_as<S, IDirect3DVertexShader9> ||
                          same_as<S, IDirect3DPixelShader9>;

    template <typename B>
    concept d3d9_buffer = same_as<B, IDirect3DVertexBuffer9> ||
                          same_as<B, IDirect3DIndexBuffer9>;

    // IW4's tess buffer vertex.
    //
    struct tess_vertex
    {
      float xyzw[4];
      uint32_t color;
      float texcoord[2];
      uint32_t normal;
    };

    static_assert (sizeof (tess_vertex) == 32);
    static_assert (sizeof (d3d9::vertex_element) == sizeof (D3DVERTEXELEMENT9));

    // Vulkan objects created from Direct3D 9 objects, keyed by the
    // Direct3D 9 object.
    //
    // Each entry holds a reference to its Direct3D 9 object. Otherwise, IW4
    // could release the object and get a new one at the same address, which
    // would then find the old entry. If the entry's reference is the only
    // one left, then IW4 has released the object and sweep() removes the
    // entry.
    //
    template <vk_object_type V>
    class object_cache
    {
    public:
      object_cache () = default;

      object_cache (const object_cache&) = delete;
      object_cache& operator= (const object_cache&) = delete;

      ~object_cache () {clear ();}

      template <invocable<> F>
      V*
      find (IUnknown* d, F&& make)
      {
        auto i (map_.find (d));

        if (i == map_.end ())
        {
          d->AddRef ();
          i = map_.emplace (d, vk_ref<V> (make ())).first;
        }

        return i->second.get ();
      }

      bool
      contains (IUnknown* d) const noexcept {return map_.contains (d);}

      void
      sweep ()
      {
        for (auto i (map_.begin ()); i != map_.end (); )
        {
          IUnknown* d (i->first);

          // Release() returns the new reference count and COM has no other
          // way to get it.
          //
          d->AddRef ();
          if (d->Release () == 1)
          {
            d->Release ();
            i = map_.erase (i);
          }
          else
            ++i;
        }
      }

      void
      clear ()
      {
        for (auto& [d, v]: map_)
          d->Release ();

        map_.clear ();
      }

    private:
      unordered_map<IUnknown*, vk_ref<V>> map_;
    };

    // Contents of IW4's static vertex and index buffers.
    //
    // IW4's allocator returns a new static buffer locked (see
    // r_alloc_static_vertex_buffer_hook()). IW4 then writes the contents
    // and unlocks the buffer, and on unlock we copy the contents (see
    // finished()). We create the Vulkan buffer from this copy when a draw
    // first uses it.
    //
    // IW4 may load buffers on other threads, thus the mutex. The generation
    // changes on every addition and removal so that the renderer can tell
    // when its cached lookups are stale (see renderer::buffer()).
    //
    // @@ We keep the copy after the upload. Can we free it?
    //
    class static_buffer_store
    {
    public:
      struct contents
      {
        vk_buffer::kind kind;
        vector<unsigned char> data;

        vk_ref<vk_buffer> buffer;
      };

      void
      allocated (IUnknown* b, const void* data, uint32_t size, vk_buffer::kind k)
      {
        lock_guard l (mutex_);
        pending_[b] = pending {data, size, k};
      }

      void
      finished (IUnknown* b)
      {
        lock_guard l (mutex_);

        auto i (pending_.find (b));
        if (i == pending_.end ())
          return;

        auto c (make_unique<contents> ());
        c->kind = i->second.kind;
        c->data.assign (static_cast<const unsigned char*> (i->second.data),
                        static_cast<const unsigned char*> (i->second.data) + i->second.size);

        pending_.erase (i);

        auto j (filled_.find (b));
        if (j == filled_.end ())
        {
          b->AddRef ();
          filled_.emplace (b, move (c));
        }
        else
          j->second = move (c);

        generation_.fetch_add (1, memory_order_release);
      }

      uint32_t
      generation () const noexcept
      {
        return generation_.load (memory_order_acquire);
      }

      contents*
      find (IUnknown* b)
      {
        lock_guard l (mutex_);

        auto i (filled_.find (b));
        return i != filled_.end () ? i->second.get () : nullptr;
      }

      void
      sweep ()
      {
        lock_guard l (mutex_);

        for (auto i (filled_.begin ()); i != filled_.end (); )
        {
          IUnknown* b (i->first);

          b->AddRef ();
          if (b->Release () == 1)
          {
            b->Release ();
            i = filled_.erase (i);
            generation_.fetch_add (1, memory_order_release);
          }
          else
            ++i;
        }
      }

      void
      release_buffers ()
      {
        lock_guard l (mutex_);

        for (auto& [b, c]: filled_)
          c->buffer.reset ();
      }

      void
      clear ()
      {
        lock_guard l (mutex_);

        for (auto& [b, c]: filled_)
          b->Release ();

        filled_.clear ();
        pending_.clear ();
        generation_.fetch_add (1, memory_order_release);
      }

    private:
      struct pending
      {
        const void* data;
        uint32_t size;
        vk_buffer::kind kind;
      };

      mutex mutex_;
      unordered_map<IUnknown*, pending> pending_;
      unordered_map<IUnknown*, unique_ptr<contents>> filled_;
      atomic<uint32_t> generation_ {0};
    };

    static_buffer_store static_buffers;

    // As static_buffer_store but for IW4's dynamic buffers.
    //
    // IW4 locks these buffers many times per frame, normally with discard
    // and no-overwrite, and we can't read a write-only Direct3D 9 buffer.
    // So the lock hook returns a pointer into our copy (see locked()) and
    // the unlock hook records the write.
    //
    // We could update the Vulkan buffer on unlock, but the unlock may happen
    // on another thread. More importantly, a discard must rename the Vulkan
    // buffer between the right draws and only the render thread knows where
    // they are. So we apply the recorded writes when a draw uses the buffer
    // (see update()) and only copy the dirty bytes that this draw reads.
    //
    // Note that we apply each write with its lock flags, which gives the
    // Vulkan buffer the same renaming as in Direct3D 9 (see
    // vk_buffer::lock()).
    //
    class dynamic_buffer_store
    {
    public:
      template <d3d9_buffer B>
      void*
      locked (B* b, uint32_t offset, uint32_t size, uint32_t flags, void* data)
      {
        // A read-only lock doesn't change the contents, so return the
        // pointer as is.
        //
        if (data == nullptr || (flags & d3d9::lock_readonly) != 0)
          return data;

        lock_guard l (mutex_);

        auto i (map_.find (b));

        if (i == map_.end ())
        {
          // If this is the first lock, then add an entry, but only for a
          // dynamic buffer (static buffers are in static_buffer_store).
          // Note that we get the size from the description since the lock
          // may not cover the whole buffer.
          //
          conditional_t<same_as<B, IDirect3DIndexBuffer9>,
                        D3DINDEXBUFFER_DESC,
                        D3DVERTEXBUFFER_DESC> desc;
          if (FAILED (b->GetDesc (&desc)) || (desc.Usage & D3DUSAGE_DYNAMIC) == 0)
            return data;

          auto e (make_unique<entry> ());
          e->size = desc.Size;
          e->contents.resize (desc.Size);

          if constexpr (same_as<B, IDirect3DIndexBuffer9>)
            e->kind = desc.Format == D3DFMT_INDEX32 ? vk_buffer::kind::index32
                                                    : vk_buffer::kind::index16;
          else
            e->kind = vk_buffer::kind::vertex;

          b->AddRef ();
          i = map_.emplace (b, move (e)).first;
          generation_.fetch_add (1, memory_order_release);
        }

        entry& e (*i->second);

        // As in Direct3D 9, zero size means the whole buffer.
        //
        if (size == 0)
        {
          offset = 0;
          size = e.size;
        }

        if (offset >= e.size)
          return data;

        e.lock = {offset, min (size, e.size - offset), flags, true};
        return e.contents.data () + offset;
      }

      void
      unlocking (IUnknown* b)
      {
        lock_guard l (mutex_);

        auto i (map_.find (b));
        if (i == map_.end () || !i->second->lock.active)
          return;

        entry& e (*i->second);
        const lock_range& r (e.lock);

        // After a discard the earlier writes are no longer needed.
        //
        if ((r.flags & d3d9::lock_discard) != 0)
          e.writes.clear ();

        e.writes.push_back (write {r.offset,
                                   r.size,
                                   r.flags & (d3d9::lock_discard |
                                              d3d9::lock_nooverwrite)});
        e.lock = {};
        e.pending.store (true, memory_order_release);
      }

      struct entry;

      entry*
      find (IUnknown* b)
      {
        lock_guard l (mutex_);

        auto i (map_.find (b));
        return i != map_.end () ? i->second.get () : nullptr;
      }

      vk_buffer*
      update (entry& e, vk_device& d, uint32_t first, uint32_t last)
      {
        if (e.buffer.get () == nullptr)
        {
          e.buffer = vk_ref<vk_buffer> (
            new vk_buffer (d,
                           e.size,
                           d3d9::usage_dynamic | d3d9::usage_writeonly,
                           e.kind));
          e.dirty.clear ();
        }

        // Apply the recorded writes. We swap them out under the mutex so that
        // other threads can record new writes while we apply these.
        //
        // For a write without no-overwrite we lock the Vulkan buffer with the
        // same flags, which renames it if the GPU may still be reading it.
        // Note that the written bytes are only marked dirty here.
        //
        if (e.pending.load (memory_order_acquire))
        {
          {
            lock_guard l (mutex_);
            e.pending.store (false, memory_order_relaxed);
            swap (e.writes, e.applying);
          }

          for (const write& w: e.applying)
          {
            if ((w.flags & d3d9::lock_nooverwrite) == 0)
            {
              e.buffer->lock (0, 0, w.flags);
              e.buffer->unlock ();
            }

            if ((w.flags & d3d9::lock_discard) != 0)
              e.dirty.clear ();

            mark_dirty (e.dirty, w.offset, w.offset + w.size);
          }

          e.applying.clear ();
        }

        // Copy the dirty bytes in the range that this draw reads. The rest
        // stays dirty until a draw reads it. IW4 probably writes much more
        // than one draw uses, so this should save copying.
        //
        first = min (first, e.size);
        last = min (last, e.size);

        if (first < last && !e.dirty.empty ())
          copy_dirty (e, first, last);

        return e.buffer.get ();
      }

      uint32_t
      generation () const noexcept
      {
        return generation_.load (memory_order_acquire);
      }

      void
      sweep ()
      {
        lock_guard l (mutex_);

        for (auto i (map_.begin ()); i != map_.end (); )
        {
          IUnknown* b (i->first);

          b->AddRef ();
          if (b->Release () == 1)
          {
            b->Release ();
            i = map_.erase (i);
            generation_.fetch_add (1, memory_order_release);
          }
          else
            ++i;
        }
      }

      void
      release_buffers ()
      {
        lock_guard l (mutex_);

        for (auto& [b, e]: map_)
        {
          e->buffer.reset ();
          e->dirty.clear ();
        }
      }

      void
      clear ()
      {
        lock_guard l (mutex_);

        for (auto& [b, e]: map_)
          b->Release ();

        map_.clear ();
        generation_.fetch_add (1, memory_order_release);
      }

      struct range
      {
        uint32_t first;
        uint32_t last;
      };

      // Add [first, last) to the dirty ranges, which are sorted and
      // disjoint, merging it with the adjacent and overlapping ranges.
      //
      static void
      mark_dirty (vector<range>& rs, uint32_t first, uint32_t last)
      {
        if (first >= last)
          return;

        auto i (rs.end ());
        while (i != rs.begin () && prev (i)->first > last)
          --i;

        auto j (i);
        while (j != rs.begin () && prev (j)->last >= first)
        {
          --j;
          first = min (first, j->first);
          last = max (last, j->last);
        }

        i = rs.erase (j, i);
        rs.insert (i, range {first, last});
      }

      // Copy the dirty bytes in [first, last) to the Vulkan buffer and
      // remove them from the dirty ranges. A range that extends past
      // [first, last) keeps the part outside, so a range that extends past
      // both ends is split in two.
      //
      static void
      copy_dirty (entry& e, uint32_t first, uint32_t last)
      {
        vector<range>& rs (e.dirty);

        size_t i (ranges::upper_bound (rs, first, {}, &range::last) - rs.begin ());

        while (i != rs.size ())
        {
          range r (rs[i]);

          if (r.first >= last)
            break;

          uint32_t f (max (r.first, first));
          uint32_t l (min (r.last, last));

          memcpy (e.buffer->lock (f, l - f, d3d9::lock_nooverwrite),
                  e.contents.data () + f,
                  l - f);
          e.buffer->unlock ();

          if (r.first < f && l < r.last)
          {
            rs[i].last = f;
            rs.insert (rs.begin () + i + 1, range {l, r.last});
            i += 2;
          }
          else if (r.first < f)
          {
            rs[i].last = f;
            ++i;
          }
          else if (l < r.last)
          {
            rs[i].first = l;
            ++i;
          }
          else
            rs.erase (rs.begin () + i);
        }
      }

      struct lock_range
      {
        uint32_t offset = 0;
        uint32_t size = 0;
        uint32_t flags = 0;
        bool active = false;
      };

      struct write
      {
        uint32_t offset;
        uint32_t size;
        uint32_t flags;
      };

      struct entry
      {
        vk_buffer::kind kind;
        uint32_t size;
        lock_range lock;
        vector<unsigned char> contents;
        vector<write> writes;
        vk_ref<vk_buffer> buffer;

        vector<write> applying;
        vector<range> dirty;

        atomic<bool> pending {false}; // Writes not yet applied.
      };

    private:
      mutex mutex_;
      unordered_map<IUnknown*, unique_ptr<entry>> map_;
      atomic<uint32_t> generation_ {0};
    };

    dynamic_buffer_store dynamic_buffers;

    // Textures that IW4 changed through a lock since our last upload. The
    // renderer uploads such a texture again on its next lookup (see
    // renderer::texture()). Note that, unlike a buffer, we can read a
    // texture back, so we only need to know that it has changed.
    //
    // Note also that our upload locks the Direct3D 9 texture as well. The
    // reading flag makes the unlock hook ignore these locks.
    //
    class texture_update_store
    {
    public:
      void
      unlocked (IUnknown* t)
      {
        if (reading)
          return;

        lock_guard l (mutex_);
        updated_.insert (t);
      }

      bool
      take (IUnknown* t)
      {
        lock_guard l (mutex_);
        return updated_.erase (t) != 0;
      }

      template <predicate<IUnknown*> F>
      void
      sweep (F&& have)
      {
        lock_guard l (mutex_);
        erase_if (updated_, [&have] (IUnknown* t) {return !have (t);});
      }

      static inline thread_local bool reading = false;

    private:
      mutex mutex_;
      unordered_set<IUnknown*> updated_;
    };

    texture_update_store texture_updates;

    // Report the failure and terminate. There is no fallback since the
    // Direct3D 9 device no longer renders. Note that we also show a message
    // box because the console may not be visible.
    //
    [[noreturn]] void
    fail (const char* what, const exception& e)
    {
      const string m (format ("Vulkan: unable to {}: {}", what, e.what ()));

      Logger::Print ("{}\n", m);
      OutputDebugStringA ((m + '\n').c_str ());
      MessageBoxA (nullptr, m.c_str (), "IW4x", MB_OK | MB_ICONERROR);

      terminate ();
    }

    // Vulkan renderer. We create it when IW4 creates its device and destroy
    // it when IW4 shuts down and destroys its window (see
    // r_shutdown_hook()).
    //
    // Note that a frame starts with the first rendering call (see
    // begin_frame()). IW4 sets the render target and the viewport before
    // that, so until then we only record them.
    //
    class renderer
    {
    public:
      renderer (HWND, uint32_t width, uint32_t height);

      ~renderer ();

      renderer (const renderer&) = delete;
      renderer& operator= (const renderer&) = delete;

      void
      draw_tess (const Game::GfxCmdBufContext&);

      void
      draw_indexed (d3d9::primitive_type,
                    int32_t base,
                    uint32_t min_index,
                    uint32_t vertices,
                    uint32_t start,
                    uint32_t primitives);

      void
      update_texture (IDirect3DBaseTexture9* src, IDirect3DBaseTexture9* dst);

      void
      gamma_ramp (const D3DGAMMARAMP&);

      void
      custom_texture (uint32_t sampler, IDirect3DBaseTexture9* t) noexcept
      {
        if (sampler < custom_.size ())
          custom_[sampler] = t;
      }

      void
      context (const Game::GfxCmdBufContext& c) noexcept {pass_context_ = c;}

      void
      target (Game::GfxRenderTargetId);

      void
      copy (IDirect3DSurface9* src, const RECT* src_rect,
            IDirect3DSurface9* dst, const RECT* dst_rect,
            D3DTEXTUREFILTERTYPE);

      void
      viewport (const D3DVIEWPORT9&);

      void
      viewport (const Game::GfxCmdBufState&, const Game::GfxViewport&);

      void
      clear (uint32_t flags, const float* color, float depth, uint8_t stencil);

      void
      release_objects ();

      void
      present ();

    private:
      void
      begin_frame ();

      void
      apply_gamma (vk_surface& frame, vk_surface& back);

      bool
      apply_pass (const Game::GfxCmdBufContext&, const Game::MaterialPass&);

      void
      apply_sampler (uint32_t sampler,
                     vk_texture*,
                     uint8_t sampler_state,
                     bool srgb);

      void
      bind_target ();

      void
      apply_scissor (const Game::GfxCmdBufState&);

      void
      apply_viewport ();

      vk_surface*
      surface (IDirect3DSurface9*);

      vk_buffer*
      buffer (IUnknown*,
              uint32_t offset = 0,
              uint32_t stride = 1,
              uint32_t first = 0,
              uint32_t count = 0);

      vk_vertex_declaration*
      declaration (IDirect3DVertexDeclaration9*);

      template <d3d9_shader S>
      vk_shader*
      shader (vk_shader_stage, S*);

      vk_texture*
      texture (const Game::GfxImage*);

      vk_texture*
      texture (IDirect3DBaseTexture9*);

      vk_texture*
      make_texture (IDirect3DBaseTexture9*);

      bool
      upload (IDirect3DBaseTexture9*, vk_texture&);

    private:
      vk_window window_;
      unique_ptr<vk_device> device_;
      unique_ptr<vk_context> context_;
      unique_ptr<vk_swapchain> swapchain_;

      object_cache<vk_vertex_declaration> declarations_;
      object_cache<vk_shader> shaders_;
      object_cache<vk_texture> textures_;
      object_cache<vk_surface> surfaces_;

      vk_render_states states_;
      vk_texture_filtering filtering_;

      vector<tess_vertex> vertices_;

      struct buffer_lookup
      {
        static_buffer_store::contents* static_buffer;
        dynamic_buffer_store::entry* dynamic_buffer;
      };

      unordered_map<IUnknown*, buffer_lookup> buffers_;
      uint32_t static_generation_ = ~0u;
      uint32_t dynamic_generation_ = ~0u;

      Game::GfxRenderTargetId target_ = Game::R_RENDERTARGET_FRAME_BUFFER;
      d3d9::viewport viewport_ {};
      Game::GfxCmdBufContext pass_context_ {};
      array<IDirect3DBaseTexture9*, 4> custom_ {}; // By sampler.

      array<uint16_t, 3 * 256> gamma_ {}; // Red, green, then blue.
      bool gamma_identity_ = true;
      bool gamma_changed_ = false;

      vk_ref<vk_texture> gamma_texture_;
      vk_ref<vk_shader> gamma_vertex_shader_;
      vk_ref<vk_shader> gamma_pixel_shader_;
      vk_ref<vk_vertex_declaration> gamma_declaration_;
      vk_ref<vk_texture> gamma_frame_;
      vk_ref<vk_surface> gamma_frame_surface_;

      bool drawing_ = false;
    };

    renderer::
    renderer (HWND w, uint32_t width, uint32_t height)
      : window_ (w),
        states_ (false)
    {
      vk_device::options d;
      d.log = [] (string_view m)
      {
        Logger::Print ("{}\n", m);
      };

      device_ = make_unique<vk_device> (window_, move (d));
      context_ = make_unique<vk_context> (*device_, false);

      vk_swapchain::options s;
      s.width = width;
      s.height = height;

      if (const Game::dvar_t* v = Game::Dvar_FindVar ("r_vsync"))
        s.vsync = v->current.enabled;
      swapchain_ = make_unique<vk_swapchain> (*context_, s);

      viewport_ = {0, 0, width, height, 0.0f, 1.0f};

      Logger::Print ("Vulkan: rendering on {}\n", device_->name ());
    }

    renderer::
    ~renderer ()
    {
      device_->wait_idle ();

      textures_.clear ();
      shaders_.clear ();
      declarations_.clear ();
      surfaces_.clear ();
      static_buffers.release_buffers ();
      dynamic_buffers.release_buffers ();

      swapchain_.reset ();
      context_.reset ();
    }

    void renderer::
    release_objects ()
    {
      // IW4 releases its Direct3D 9 objects but keeps the window, presumably
      // to create them again (see r_shutdown_hook()). Note that we don't
      // need to clear the buffer stores since sweep() removes the released
      // buffers.
      //
      device_->wait_idle ();

      textures_.clear ();
      shaders_.clear ();
      declarations_.clear ();
      surfaces_.clear ();
      custom_ = {};
    }

    void renderer::
    begin_frame ()
    {
      drawing_ = true;

      bind_target ();
      apply_viewport ();
    }

    void renderer::
    target (Game::GfxRenderTargetId t)
    {
      target_ = t;

      // Reset the viewport to cover the new target. IW4 seems to rely on
      // this since it only sets the viewport when it needs less than the
      // whole target.
      //
      if (static_cast<uint32_t> (t) < render_target_count)
      {
        const render_target& r (
          reinterpret_cast<const render_target*> (render_targets)[t]);

        viewport_ = {0, 0, r.width, r.height, 0.0f, 1.0f};
      }

      if (drawing_)
        bind_target ();
    }

    void renderer::
    bind_target ()
    {
      if (static_cast<uint32_t> (target_) >= render_target_count)
        return;

      const render_target& r (
        reinterpret_cast<const render_target*> (render_targets)[target_]);

      vk_surface* c (surface (r.surface));

      context_->set_render_target (0, c);
      context_->set_depth_stencil (surface (r.depth_stencil));
    }

    vk_surface* renderer::
    surface (IDirect3DSurface9* s)
    {
      if (s == nullptr)
        return nullptr;

      // Only mirror render targets and depth stencils. The NULL FOURCC
      // format is a known driver hack for a render target without memory,
      // which IW4 presumably uses for depth-only passes. We treat it as no
      // color target.
      //
      return surfaces_.find (s, [this, s] () -> vk_surface*
      {
        constexpr D3DFORMAT null_format (
          static_cast<D3DFORMAT> ('N' | 'U' << 8 | 'L' << 16 | 'L' << 24));

        D3DSURFACE_DESC d;
        if (FAILED (s->GetDesc (&d)) ||
            (d.Usage & (D3DUSAGE_RENDERTARGET | D3DUSAGE_DEPTHSTENCIL)) == 0 ||
            d.Format == null_format)
          return nullptr;

        try
        {
          return vk_surface::create (
            *device_,
            d.Width,
            d.Height,
            static_cast<d3d9::format> (d.Format),
            max (static_cast<uint32_t> (d.MultiSampleType), 1u),
            d.Usage & (D3DUSAGE_RENDERTARGET | D3DUSAGE_DEPTHSTENCIL),
            d3d9::pool_default);
        }
        catch (const exception& e)
        {
          fail ("make render target", e);
        }
      });
    }

    void renderer::
    copy (IDirect3DSurface9* src, const RECT* src_rect,
          IDirect3DSurface9* dst, const RECT* dst_rect,
          D3DTEXTUREFILTERTYPE f)
    {
      if (!drawing_)
        begin_frame ();

      vk_surface* s (surface (src));
      vk_surface* d (surface (dst));

      if (s == nullptr || d == nullptr || s == d)
        return;

      auto rect = [] (const RECT* r) -> optional<d3d9::rect>
      {
        if (r == nullptr)
          return nullopt;

        return d3d9::rect {r->left, r->top, r->right, r->bottom};
      };

      optional<d3d9::rect> sr (rect (src_rect));
      optional<d3d9::rect> dr (rect (dst_rect));

      context_->stretch_rect (*s, sr ? &*sr : nullptr,
                              *d, dr ? &*dr : nullptr,
                              static_cast<d3d9::texture_filter_type> (f));
    }

    void renderer::
    present ()
    {

      if (!drawing_)
        begin_frame ();

      // Present IW4's frame buffer render target, applying the gamma ramp if
      // it is not identity. If there is no frame buffer yet, then present
      // black.
      //
      vk_surface& b (swapchain_->back_buffer ());

      if (vk_surface* f = surface (reinterpret_cast<const render_target*> (render_targets)[
                                     Game::R_RENDERTARGET_FRAME_BUFFER].surface))
      {
        if (gamma_identity_)
          context_->stretch_rect (*f, nullptr, b, nullptr, d3d9::texf_linear);
        else
          apply_gamma (*f, b);
      }
      else
      {
        context_->set_render_target (0, &b);
        context_->set_depth_stencil (nullptr);
        context_->set_render_state (d3d9::rs_scissortestenable, false);
        context_->clear ({}, d3d9::clear_target, d3d9::color (0xFF000000), 1.0f, 0);
      }

      swapchain_->present ();
      drawing_ = false;

      // Remove the objects that IW4 released during the frame. We do this
      // once per frame since doing it on every draw would be wasteful.
      //
      declarations_.sweep ();
      shaders_.sweep ();
      textures_.sweep ();
      surfaces_.sweep ();
      static_buffers.sweep ();
      dynamic_buffers.sweep ();
      texture_updates.sweep ([this] (IUnknown* t) {return textures_.contains (t);});
    }

    void renderer::
    apply_viewport ()
    {
      if (viewport_.width != 0 && viewport_.height != 0)
        context_->set_viewport (viewport_);
    }

    void renderer::
    viewport (const Game::GfxCmdBufState& st, const Game::GfxViewport& v)
    {
      viewport_ = {static_cast<uint32_t> (v.x),
                   static_cast<uint32_t> (v.y),
                   static_cast<uint32_t> (v.width),
                   static_cast<uint32_t> (v.height),
                   st.depthRangeNear,
                   st.depthRangeFar};

      if (drawing_)
        apply_viewport ();
    }

    void renderer::
    viewport (const D3DVIEWPORT9& v)
    {
      viewport_ = {v.X, v.Y, v.Width, v.Height, v.MinZ, v.MaxZ};

      if (drawing_)
        apply_viewport ();
    }

    void renderer::
    update_texture (IDirect3DBaseTexture9* src, IDirect3DBaseTexture9* dst)
    {
      if (src == nullptr || dst == nullptr)
        return;

      if (vk_texture* t = texture (dst))
        upload (src, *t);
    }

    void renderer::
    gamma_ramp (const D3DGAMMARAMP& r)
    {
      // If this is the identity ramp, then present() skips the gamma pass.
      // Note that the identity ramp maps 8-bit i to 16-bit i * 257 (0xff to
      // 0xffff).
      //
      gamma_identity_ = true;

      for (uint32_t i (0); i != 256; ++i)
      {
        gamma_[i] = r.red[i];
        gamma_[256 + i] = r.green[i];
        gamma_[512 + i] = r.blue[i];

        const uint16_t x (static_cast<uint16_t> (i * 257));

        if (r.red[i] != x || r.green[i] != x || r.blue[i] != x)
          gamma_identity_ = false;
      }

      gamma_changed_ = true;
    }

    void renderer::
    apply_gamma (vk_surface& frame, vk_surface& back)
    {
      // Direct3D 9 applies the gamma ramp when the display scans out the
      // frame. Vulkan has no equivalent, so we apply it in a full-screen pass
      // that looks up each channel in a 256x1 texture. Note that the shader
      // scales the color so that 0 and 1 map to the first and last texel
      // centers.
      //
      // The frame buffer may differ in size from the back buffer. So we first
      // copy it, with linear filtering, into a texture with the back buffer's
      // size, and the pass then point-samples that copy so that each pixel
      // reads one texel.
      //
      // @@ Could the pass sample the frame buffer with linear filtering and
      //    skip the copy? Probably, though the frame buffer would need a
      //    transition to a shader read layout and back. We haven't tried it.
      //
      const uint32_t w (back.width ());
      const uint32_t h (back.height ());

      if (gamma_texture_.get () == nullptr)
      {
        vk_texture_desc td;
        td.width = 256;
        td.format = d3d9::fmt_a16b16g16r16;
        td.pool = d3d9::pool_managed;
        gamma_texture_ = vk_ref<vk_texture> (new vk_texture (*device_, td));

        static const char vs[] =
          "void main (float4 p : POSITION, float2 t : TEXCOORD0,\n"
          "           out float4 op : POSITION, out float2 ot : TEXCOORD0)\n"
          "{\n"
          "  op = p;\n"
          "  ot = t;\n"
          "}\n";

        static const char ps[] =
          "sampler2D frame : register (s0);\n"
          "sampler2D ramp : register (s1);\n"
          "float4 main (float2 t : TEXCOORD0) : COLOR\n"
          "{\n"
          "  float3 c = tex2D (frame, t).rgb * (255.0 / 256.0) + 0.5 / 256.0;\n"
          "  return float4 (tex2D (ramp, float2 (c.r, 0.5)).r,\n"
          "                 tex2D (ramp, float2 (c.g, 0.5)).g,\n"
          "                 tex2D (ramp, float2 (c.b, 0.5)).b,\n"
          "                 1.0);\n"
          "}\n";

        gamma_vertex_shader_ = vk_ref<vk_shader> (
          new vk_shader (*device_,
                         vk_shader_stage::vertex,
                         vk_compile_hlsl (vs, "vs_3_0")));

        gamma_pixel_shader_ = vk_ref<vk_shader> (
          new vk_shader (*device_,
                         vk_shader_stage::pixel,
                         vk_compile_hlsl (ps, "ps_3_0")));

        static const d3d9::vertex_element es[]
        {
          {0, 0,  d3d9::decltype_float4, d3d9::declmethod_default,
           d3d9::declusage_position, 0},
          {0, 16, d3d9::decltype_float2, d3d9::declmethod_default,
           d3d9::declusage_texcoord, 0},
          d3d9::decl_end
        };

        gamma_declaration_ = vk_ref<vk_vertex_declaration> (
          new vk_vertex_declaration (*device_, es));
      }

      if (gamma_changed_)
      {
        d3d9::locked_rect l (gamma_texture_->lock_rect (0, 0, nullptr, 0));
        uint16_t* p (static_cast<uint16_t*> (l.bits));

        for (uint32_t i (0); i != 256; ++i)
        {
          p[i * 4 + 0] = gamma_[i];
          p[i * 4 + 1] = gamma_[256 + i];
          p[i * 4 + 2] = gamma_[512 + i];
          p[i * 4 + 3] = 0xffff;
        }

        gamma_texture_->unlock_rect (0, 0);
        gamma_changed_ = false;
      }

      if (gamma_frame_.get () == nullptr       ||
          gamma_frame_surface_->width () != w  ||
          gamma_frame_surface_->height () != h)
      {
        gamma_frame_surface_ = vk_ref<vk_surface> ();

        vk_texture_desc td;
        td.width = w;
        td.height = h;
        td.format = back.texture ().desc ().format;
        td.usage = d3d9::usage_rendertarget;
        gamma_frame_ = vk_ref<vk_texture> (new vk_texture (*device_, td));
        gamma_frame_surface_ = vk_ref<vk_surface> (gamma_frame_->surface (0, 0));
      }

      context_->stretch_rect (frame,
                              nullptr,
                              *gamma_frame_surface_,
                              nullptr,
                              d3d9::texf_linear);

      // Disable the states that IW4's last pass may have enabled, such as
      // depth testing and blending.
      //
      context_->set_render_target (0, &back);
      context_->set_depth_stencil (nullptr);
      context_->set_viewport ({0, 0, w, h, 0.0f, 1.0f});

      const pair<d3d9::render_state_type, uint32_t> rs[]
      {
        {d3d9::rs_zenable, d3d9::zb_false},
        {d3d9::rs_zwriteenable, false},
        {d3d9::rs_stencilenable, false},
        {d3d9::rs_alphatestenable, false},
        {d3d9::rs_alphablendenable, false},
        {d3d9::rs_scissortestenable, false},
        {d3d9::rs_srgbwriteenable, false},
        {d3d9::rs_cullmode, d3d9::cull_none},
        {d3d9::rs_fillmode, d3d9::fill_solid},
        {d3d9::rs_depthbias, 0},
        {d3d9::rs_slopescaledepthbias, 0},
        {d3d9::rs_colorwriteenable, 0xf}
      };

      for (const auto& [s, v]: rs)
        context_->set_render_state (s, v);

      auto sampler = [this] (uint32_t s, vk_texture* t, uint32_t filter)
      {
        context_->set_sampler_state (s, d3d9::samp_minfilter, filter);
        context_->set_sampler_state (s, d3d9::samp_magfilter, filter);
        context_->set_sampler_state (s, d3d9::samp_mipfilter, d3d9::texf_none);
        context_->set_sampler_state (s, d3d9::samp_addressu, d3d9::taddress_clamp);
        context_->set_sampler_state (s, d3d9::samp_addressv, d3d9::taddress_clamp);
        context_->set_sampler_state (s, d3d9::samp_srgbtexture, false);
        context_->set_texture (s, t);
      };

      sampler (0, gamma_frame_.get (), d3d9::texf_point);
      sampler (1, gamma_texture_.get (), d3d9::texf_linear);

      context_->set_vertex_shader (gamma_vertex_shader_.get ());
      context_->set_pixel_shader (gamma_pixel_shader_.get ());
      context_->set_vertex_declaration (gamma_declaration_.get ());

      // The context emulates Direct3D 9 pixel centers (see vk_context's
      // viewport), so shift the quad by half a pixel as usual in Direct3D 9.
      // Half a pixel is 1/w in clip space since clip space is 2 wide.
      //
      const float dx (1.0f / static_cast<float> (w));
      const float dy (1.0f / static_cast<float> (h));

      const float vs[4][6]
      {
        {-1.0f - dx,  1.0f + dy, 0.0f, 1.0f, 0.0f, 0.0f},
        { 1.0f - dx,  1.0f + dy, 0.0f, 1.0f, 1.0f, 0.0f},
        {-1.0f - dx, -1.0f + dy, 0.0f, 1.0f, 0.0f, 1.0f},
        { 1.0f - dx, -1.0f + dy, 0.0f, 1.0f, 1.0f, 1.0f}
      };

      context_->draw_user (d3d9::pt_trianglestrip, 2, vs, sizeof (vs[0]));
    }

    void renderer::
    clear (uint32_t flags, const float* color, float depth, uint8_t stencil)
    {
      if (!drawing_)
        begin_frame ();

      // Disable the scissor test, which the last draw may have left enabled.
      // We assume that IW4 wants to clear the whole viewport.
      //
      apply_viewport ();
      context_->set_render_state (d3d9::rs_scissortestenable, false);

      array<float, 4> c {0.0f, 0.0f, 0.0f, 0.0f};
      if (color != nullptr)
        for (size_t i (0); i != c.size (); ++i)
          c[i] = clamp (color[i], 0.0f, 1.0f);

      context_->clear ({}, flags, c, depth, stencil);
    }

    void renderer::
    draw_tess (const Game::GfxCmdBufContext& c)
    {
      const Game::GfxCmdBufState& st (*c.state);

      if (st.material == nullptr || st.technique == nullptr)
        return;

      const uint32_t vn (*reinterpret_cast<const uint32_t*> (tess_vertex_count));
      const uint32_t in (*reinterpret_cast<const uint32_t*> (tess_index_count));

      if (vn == 0 || in < 3)
        return;

      if (!drawing_)
        begin_frame ();

      // Expand the indexed tess geometry into a triangle list since
      // draw_user() doesn't take indices. Drop a trailing incomplete
      // triangle and use a zero vertex for an out of range index. The tess
      // buffers are small, so the copy should be cheap.
      //
      {
        const tess_vertex* vs (reinterpret_cast<const tess_vertex*> (tess_vertices));
        const uint16_t* is (reinterpret_cast<const uint16_t*> (tess_indices));

        const uint32_t n (in - in % 3);

        vertices_.resize (n);
        for (uint32_t i (0); i != n; ++i)
          vertices_[i] = is[i] < vn ? vs[is[i]] : tess_vertex {};
      }

      apply_scissor (st);

      const Game::MaterialTechnique& t (*st.technique);

      for (uint16_t i (0); i != t.passCount; ++i)
      {
        if (!apply_pass (c, t.passArray[i]))
          continue;

        context_->draw_user (d3d9::pt_trianglelist,
                             static_cast<uint32_t> (vertices_.size () / 3),
                             vertices_.data (),
                             sizeof (tess_vertex));
      }
    }

    void renderer::
    draw_indexed (d3d9::primitive_type type,
                  int32_t base,
                  uint32_t min_index,
                  uint32_t vertices,
                  uint32_t start,
                  uint32_t primitives)
    {
      // The DrawIndexedPrimitive() call site only passes the Direct3D 9
      // arguments. We get the material and the pass from the last
      // R_SetupPass() call's context (see r_setup_pass_hook()).
      //
      const Game::GfxCmdBufContext& c (pass_context_);

      if (c.state == nullptr || c.source == nullptr)
        return;

      const Game::GfxCmdBufState& st (*c.state);

      if (st.material == nullptr || st.pass == nullptr)
        return;

      if (!drawing_)
        begin_frame ();

      const Game::GfxCmdBufPrimState& prim (st.prim);

      const Game::MaterialPass& p (*st.pass);

      if (p.vertexDecl == nullptr)
        return;

      vk_vertex_declaration* d (
        declaration (p.vertexDecl->routing.decl[prim.vertDeclType]));

      if (d == nullptr)
        return;

      // Bind the vertex streams. IW4 uses two and seems to leave the second
      // one set when the declaration no longer reads it. Binding it would
      // make us update a dynamic buffer that the draw doesn't read, so
      // unbind such a stream.
      //
      for (uint32_t i (0); i != 2; ++i)
      {
        const auto& s (prim.streams[i]);

        const bool used (ranges::any_of (
          d->elements (),
          [i] (const d3d9::vertex_element& e) {return e.stream == i;}));

        if (!used)
        {
          context_->set_stream_source (i, nullptr, 0, 0);
          continue;
        }

        vk_buffer* b (buffer (s.vb,
                              s.offset,
                              s.stride,
                              static_cast<uint32_t> (base) + min_index,
                              vertices));
        if (b == nullptr)
          return;

        context_->set_stream_source (i, b, s.offset, s.stride);
      }

      vk_buffer* ib (buffer (prim.indexBuffer,
                             0,
                             0,
                             start,
                             vk_vertex_count (type, primitives)));
      if (ib == nullptr)
        return;

      context_->set_indices (ib);

      apply_scissor (st);

      if (!apply_pass (c, p))
        return;

      context_->draw_indexed (type, base, start, primitives);
    }

    void renderer::
    apply_scissor (const Game::GfxCmdBufState& st)
    {
      const bool scissor (st.scissorW != 0 && st.scissorH != 0);

      context_->set_render_state (d3d9::rs_scissortestenable, scissor);

      if (scissor)
        context_->set_scissor ({static_cast<int32_t> (st.scissorX),
                                static_cast<int32_t> (st.scissorY),
                                static_cast<int32_t> (st.scissorX + st.scissorW),
                                static_cast<int32_t> (st.scissorY + st.scissorH)});
    }

    vk_buffer* renderer::
    buffer (IUnknown* b,
            uint32_t offset,
            uint32_t stride,
            uint32_t first,
            uint32_t count)
    {
      if (b == nullptr)
        return nullptr;

      // Looking a buffer up in the stores locks their mutex. So we cache the
      // lookups and clear the cache when a store's generation changes.
      //
      if (uint32_t s (static_buffers.generation ()),
                   d (dynamic_buffers.generation ());
          s != static_generation_ || d != dynamic_generation_)
      {
        buffers_.clear ();
        static_generation_ = s;
        dynamic_generation_ = d;
      }

      auto i (buffers_.find (b));
      if (i == buffers_.end ())
      {
        buffer_lookup l {static_buffers.find (b), nullptr};

        if (l.static_buffer == nullptr)
          l.dynamic_buffer = dynamic_buffers.find (b);

        i = buffers_.emplace (b, l).first;
      }

      // For a dynamic buffer, update the range that this draw reads. An index
      // buffer has zero stride, in which case we use the index size.
      //
      if (dynamic_buffer_store::entry* e = i->second.dynamic_buffer)
      {
        try
        {
          if (stride == 0)
            stride = e->kind == vk_buffer::kind::index32 ? 4 : 2;

          uint64_t f (offset + uint64_t (first) * stride);
          uint64_t l (count != 0 ? f + uint64_t (count) * stride : e->size);

          return dynamic_buffers.update (
            *e,
            *device_,
            static_cast<uint32_t> (min<uint64_t> (f, e->size)),
            static_cast<uint32_t> (min<uint64_t> (l, e->size)));
        }
        catch (const exception& e)
        {
          fail ("make buffer", e);
        }
      }

      static_buffer_store::contents* c (i->second.static_buffer);
      if (c == nullptr)
        return nullptr;

      // For a static buffer, create the Vulkan buffer on first use.
      //
      if (c->buffer.get () == nullptr && !c->data.empty ())
      {
        try
        {
          vk_ref<vk_buffer> r (
            new vk_buffer (*device_,
                           static_cast<uint32_t> (c->data.size ()),
                           d3d9::usage_writeonly,
                           c->kind));

          memcpy (r->lock (0, 0, 0), c->data.data (), c->data.size ());
          r->unlock ();

          c->buffer = move (r);
        }
        catch (const exception& e)
        {
          fail ("make buffer", e);
        }
      }

      return c->buffer.get ();
    }

    bool renderer::
    apply_pass (const Game::GfxCmdBufContext& c, const Game::MaterialPass& p)
    {
      Game::GfxCmdBufSourceState& src (*c.source);
      const Game::GfxCmdBufState& st (*c.state);
      const Game::Material& m (*st.material);

      vk_vertex_declaration* d (
        p.vertexDecl != nullptr
        ? declaration (p.vertexDecl->routing.decl[st.prim.vertDeclType])
        : nullptr);

      vk_shader* vs (p.vertexShader != nullptr
                     ? shader (vk_shader_stage::vertex, p.vertexShader->prog.vs)
                     : nullptr);

      vk_shader* ps (p.pixelShader != nullptr
                     ? shader (vk_shader_stage::pixel, p.pixelShader->prog.ps)
                     : nullptr);

      if (d == nullptr || vs == nullptr || ps == nullptr)
        return false;

      context_->set_vertex_declaration (d);
      context_->set_vertex_shader (vs);
      context_->set_pixel_shader (ps);

      // Apply the pass's state bits. The material's stateBitsEntry for the
      // technique type is the first pass's index in the state bits table
      // and the other passes follow it.
      //
      const uint32_t sb (static_cast<uint8_t> (m.stateBitsEntry[st.techType]) +
                         static_cast<uint32_t> (&p - st.technique->passArray));

      if (m.stateBitsTable != nullptr && sb < m.stateBitsCount)
      {
        const Game::GfxStateBits& b (m.stateBitsTable[sb]);

        vk_apply_state_bits (states_,
                             b.loadBits[0],
                             b.loadBits[1],
                             polygon_offsets ());

        for (d3d9::render_state_type s: vk_state_bits_states ())
          context_->set_render_state (s, states_[s]);
      }

      auto constant = [this] (vk_shader_stage s,
                              uint32_t dest,
                              const float* v,
                              uint32_t rows)
      {
        context_->set_constants (s, vk_constant_set::floats, dest, v, rows);
      };

      auto material_constant = [&m] (uint32_t name_hash) -> const float*
      {
        span<const Game::MaterialConstantDef> cs (m.constantTable, m.constantCount);

        auto i (ranges::find (cs, name_hash, &Game::MaterialConstantDef::nameHash));
        return i != cs.end () ? i->literal : nullptr;
      };

      // Set the shader arguments. We look up material constants by name hash
      // and use zero for a missing one. The code constants below
      // CONST_SRC_CODE_COUNT_FLOAT4 are vectors in the source state. The ones
      // above are matrices, which IW4 computes on request
      // (R_GetCodeMatrix()).
      //
      static const float zero[4] {};

      const auto get_code_matrix (reinterpret_cast<r_get_code_matrix_t*> (r_get_code_matrix));

      const uint32_t n (static_cast<uint8_t> (p.perPrimArgCount) +
                        static_cast<uint8_t> (p.perObjArgCount)  +
                        static_cast<uint8_t> (p.stableArgCount));

      for (const Game::MaterialShaderArgument& a: span (p.args, n))
      {
        switch (a.type)
        {
          case Game::MTL_ARG_MATERIAL_VERTEX_CONST:
          case Game::MTL_ARG_MATERIAL_PIXEL_CONST:
          {
            const float* v (material_constant (a.u.nameHash));

            constant (a.type == Game::MTL_ARG_MATERIAL_VERTEX_CONST
                      ? vk_shader_stage::vertex
                      : vk_shader_stage::pixel,
                      a.dest,
                      v != nullptr ? v : zero,
                      1);
            break;
          }
          case Game::MTL_ARG_LITERAL_VERTEX_CONST:
          case Game::MTL_ARG_LITERAL_PIXEL_CONST:
          {
            constant (a.type == Game::MTL_ARG_LITERAL_VERTEX_CONST
                      ? vk_shader_stage::vertex
                      : vk_shader_stage::pixel,
                      a.dest,
                      a.u.literalConst,
                      1);
            break;
          }
          case Game::MTL_ARG_CODE_VERTEX_CONST:
          case Game::MTL_ARG_CODE_PIXEL_CONST:
          {
            const vk_shader_stage s (a.type == Game::MTL_ARG_CODE_VERTEX_CONST
                                     ? vk_shader_stage::vertex
                                     : vk_shader_stage::pixel);

            const Game::MaterialArgumentCodeConst& cc (a.u.codeConst);
            const uint32_t rows (static_cast<uint8_t> (cc.rowCount));

            if (cc.index < Game::CONST_SRC_CODE_COUNT_FLOAT4)
            {
              for (uint32_t r (0); r != rows; ++r)
              {
                const uint32_t i (cc.index + r);

                constant (s,
                          a.dest + r,
                          i < Game::CONST_SRC_CODE_COUNT_FLOAT4
                          ? src.input.consts[i]
                          : zero,
                          1);
              }
            }
            else
              constant (s,
                        a.dest,
                        get_code_matrix (&src,
                                         cc.index,
                                         static_cast<uint8_t> (cc.firstRow)),
                        rows);
            break;
          }
          case Game::MTL_ARG_MATERIAL_PIXEL_SAMPLER:
          {
            span<const Game::MaterialTextureDef> ts (m.textureTable, m.textureCount);

            auto i (ranges::find (ts, a.u.nameHash, &Game::MaterialTextureDef::nameHash));

            const Game::GfxImage* image (nullptr);
            uint8_t state (0);

            if (i != ts.end ())
            {
              state = static_cast<uint8_t> (i->samplerState);

              // A water map stores its image in the water definition.
              //
              if (i->semantic == Game::TS_WATER_MAP)
                image = i->u.water != nullptr ? i->u.water->image : nullptr;
              else
                image = i->u.image;
            }

            apply_sampler (a.dest,
                           texture (image),
                           state,
                           image != nullptr && image->useSrgbReads);
            break;
          }
          case Game::MTL_ARG_CODE_PIXEL_SAMPLER:
          {
            const uint32_t s (a.u.codeSampler);

            if (s < Game::TEXTURE_SRC_CODE_COUNT)
            {
              const Game::GfxImage* image (src.input.codeImages[s]);

              apply_sampler (
                a.dest,
                texture (image),
                static_cast<uint8_t> (src.input.codeImageSamplerStates[s]),
                image != nullptr && image->useSrgbReads);
            }
            else
              apply_sampler (a.dest, nullptr, 0, false);
            break;
          }
          default:
            break;
        }
      }

      // Bind the custom samplers. IW4 sets their textures with SetTexture()
      // outside the material (see custom_texture()).
      //
      // @@ We hardcode the sampler states that IW4 seems to use for these
      //    samplers. Can we get them from IW4?
      //
      static const uint8_t custom_states[3] {0x72, 0x62, 0x62};
      const uint32_t srgb_read (0x4000000); // samplerState bit.

      for (uint32_t i (0); i != 3; ++i)
      {
        if ((static_cast<uint8_t> (p.customSamplerFlags) & (1u << i)) != 0)
          apply_sampler (i + 1,
                         texture (custom_[i + 1]),
                         custom_states[i],
                         (st.samplerState[i + 1] & srgb_read) != 0);
      }

      return true;
    }

    void renderer::
    apply_sampler (uint32_t sampler,
                   vk_texture* tex,
                   uint8_t sampler_state,
                   bool srgb)
    {
      static const d3d9::sampler_state_type types[]
      {
        d3d9::samp_minfilter,
        d3d9::samp_magfilter,
        d3d9::samp_mipfilter,
        d3d9::samp_maxanisotropy,
        d3d9::samp_addressu,
        d3d9::samp_addressv,
        d3d9::samp_addressw
      };

      vk_sampler_states s;
      vk_apply_sampler_state (s, sampler_state, filtering_);

      for (d3d9::sampler_state_type t: types)
        context_->set_sampler_state (sampler, t, s[t]);

      context_->set_sampler_state (sampler, d3d9::samp_srgbtexture, srgb);

      context_->set_texture (sampler, tex);
    }

    vk_vertex_declaration* renderer::
    declaration (IDirect3DVertexDeclaration9* d)
    {
      if (d == nullptr)
        return nullptr;

      return declarations_.find (d, [this, d] () -> vk_vertex_declaration*
      {
        UINT n (0);
        if (FAILED (d->GetDeclaration (nullptr, &n)) || n == 0)
          return nullptr;

        vector<D3DVERTEXELEMENT9> es (n);
        if (FAILED (d->GetDeclaration (es.data (), &n)))
          return nullptr;

        try
        {
          return new vk_vertex_declaration (
            *device_,
            reinterpret_cast<const d3d9::vertex_element*> (es.data ()));
        }
        catch (const exception& e)
        {
          fail ("make vertex declaration", e);
        }
      });
    }

    template <d3d9_shader S>
    vector<uint32_t>
    shader_code (S* s)
    {
      UINT n (0);
      if (FAILED (s->GetFunction (nullptr, &n)) || n < sizeof (uint32_t))
        return {};

      vector<uint32_t> r (n / sizeof (uint32_t));
      if (FAILED (s->GetFunction (r.data (), &n)))
        return {};

      return r;
    }

    template <d3d9_shader S>
    vk_shader* renderer::
    shader (vk_shader_stage stage, S* s)
    {
      if (s == nullptr)
        return nullptr;

      return shaders_.find (s, [this, stage, s] () -> vk_shader*
      {
        vector<uint32_t> code (shader_code (s));

        try
        {
          return !code.empty () ? new vk_shader (*device_, stage, code) : nullptr;
        }
        catch (const exception& e)
        {
          fail ("translate shader", e);
        }
      });
    }

    vk_texture* renderer::
    texture (const Game::GfxImage* i)
    {
      return texture (i != nullptr ? i->texture.basemap : nullptr);
    }

    vk_texture* renderer::
    texture (IDirect3DBaseTexture9* d)
    {

      // If IW4 samples a render target texture, then use our surface's
      // texture, which has what IW4 rendered. A texture created from the
      // Direct3D 9 one would be empty since that device doesn't render.
      //
      if (d != nullptr && d->GetType () == D3DRTYPE_TEXTURE)
      {
        auto* x (static_cast<IDirect3DTexture9*> (d));

        D3DSURFACE_DESC sd;
        if (SUCCEEDED (x->GetLevelDesc (0, &sd)) &&
            (sd.Usage & (D3DUSAGE_RENDERTARGET | D3DUSAGE_DEPTHSTENCIL)) != 0)
        {
          IDirect3DSurface9* s (nullptr);
          if (FAILED (x->GetSurfaceLevel (0, &s)))
            return nullptr;

          vk_surface* r (surface (s));
          s->Release ();

          return r != nullptr ? &r->texture () : nullptr;
        }
      }

      vk_texture* r (d != nullptr
                     ? textures_.find (d, [this, d] () {return make_texture (d);})
                     : nullptr);

      // Upload the texture again if IW4 changed it.
      //
      if (r != nullptr && texture_updates.take (d))
        upload (d, *r);

      return r;
    }

    vk_texture* renderer::
    make_texture (IDirect3DBaseTexture9* d)
    {
      vk_texture_desc td;
      td.levels = d->GetLevelCount ();

      // We can only fill the texture now if Direct3D 9 lets us lock it,
      // which is not the case for a default pool texture that is not
      // dynamic. Such a texture stays empty until IW4 updates it (see
      // update_texture()).
      //
      // Note that we don't mirror system memory textures. Direct3D 9 can't
      // sample them, so they are only update sources, which update_texture()
      // reads directly.
      //
      bool readable (true);
      td.pool = d3d9::pool_managed;

      const D3DRESOURCETYPE type (d->GetType ());

      switch (type)
      {
        case D3DRTYPE_TEXTURE:
        {
          D3DSURFACE_DESC s;
          if (FAILED (static_cast<IDirect3DTexture9*> (d)->GetLevelDesc (0, &s)) ||
              s.Pool == D3DPOOL_SYSTEMMEM)
            return nullptr;

          readable = s.Pool != D3DPOOL_DEFAULT ||
                     (s.Usage & D3DUSAGE_DYNAMIC) != 0;

          td.type = vk_texture_desc::kind::plane;
          td.width = s.Width;
          td.height = s.Height;
          td.format = static_cast<d3d9::format> (s.Format);
          break;
        }
        case D3DRTYPE_CUBETEXTURE:
        {
          D3DSURFACE_DESC s;
          // @@ For now we only mirror managed cube textures. Should we handle
          //    the other pools the same as for plain textures?
          //
          if (FAILED (static_cast<IDirect3DCubeTexture9*> (d)->GetLevelDesc (0, &s)) ||
              s.Pool != D3DPOOL_MANAGED)
            return nullptr;

          td.type = vk_texture_desc::kind::cube;
          td.width = s.Width;
          td.height = s.Height;
          td.format = static_cast<d3d9::format> (s.Format);
          break;
        }
        case D3DRTYPE_VOLUMETEXTURE:
        {
          D3DVOLUME_DESC s;
          if (FAILED (static_cast<IDirect3DVolumeTexture9*> (d)->GetLevelDesc (0, &s)) ||
              s.Pool == D3DPOOL_SYSTEMMEM)
            return nullptr;

          readable = s.Pool != D3DPOOL_DEFAULT ||
                     (s.Usage & D3DUSAGE_DYNAMIC) != 0;

          td.type = vk_texture_desc::kind::volume;
          td.width = s.Width;
          td.height = s.Height;
          td.depth = s.Depth;
          td.format = static_cast<d3d9::format> (s.Format);
          break;
        }
        default:
          return nullptr;
      }

      optional<vk_format_info> f (vk_format (td.format));
      if (!f)
        return nullptr;

      try
      {
        vk_ref<vk_texture> t (new vk_texture (*device_, td));

        if (readable && !upload (d, *t))
          return nullptr;

        return t.release ();
      }
      catch (const exception& e)
      {
        fail ("make texture", e);
      }
    }

    bool renderer::
    upload (IDirect3DBaseTexture9* d, vk_texture& t)
    {
      struct reading_guard
      {
        reading_guard () {texture_update_store::reading = true;}
        ~reading_guard () {texture_update_store::reading = false;}
      } g;

      // Copy each face's levels row by row since the Direct3D 9 pitch can be
      // larger than ours.
      //
      const D3DRESOURCETYPE type (d->GetType ());
      const vk_texture_desc& td (t.desc ());

      optional<vk_format_info> f (vk_format (td.format));
      if (!f)
        return false;

      const uint32_t faces (type == D3DRTYPE_CUBETEXTURE ? 6 : 1);

      for (uint32_t face (0); face != faces; ++face)
      {
        for (uint32_t level (0); level != td.levels; ++level)
        {
          const uint32_t pitch (f->pitch (t.width (level)));
          const uint32_t rows (f->rows (t.height (level)));

          if (type == D3DRTYPE_VOLUMETEXTURE)
          {
            auto* v (static_cast<IDirect3DVolumeTexture9*> (d));

            D3DLOCKED_BOX s;
            if (FAILED (v->LockBox (level, &s, nullptr, D3DLOCK_READONLY)))
              return false;

            d3d9::locked_box l (t.lock_box (level, nullptr, 0));

            for (uint32_t z (0); z != t.depth (level); ++z)
              for (uint32_t r (0); r != rows; ++r)
                memcpy (static_cast<unsigned char*> (l.bits) +
                        static_cast<size_t> (z) * l.slice_pitch +
                        static_cast<size_t> (r) * l.row_pitch,
                        static_cast<const unsigned char*> (s.pBits) +
                        static_cast<size_t> (z) * s.SlicePitch +
                        static_cast<size_t> (r) * s.RowPitch,
                        pitch);

            t.unlock_box (level);
            v->UnlockBox (level);
          }
          else
          {
            D3DLOCKED_RECT s;
            HRESULT hr (type == D3DRTYPE_CUBETEXTURE
                        ? static_cast<IDirect3DCubeTexture9*> (d)->LockRect (
                            static_cast<D3DCUBEMAP_FACES> (face),
                            level, &s, nullptr, D3DLOCK_READONLY)
                        : static_cast<IDirect3DTexture9*> (d)->LockRect (
                            level, &s, nullptr, D3DLOCK_READONLY));
            if (FAILED (hr))
              return false;

            d3d9::locked_rect l (t.lock_rect (face, level, nullptr, 0));

            for (uint32_t r (0); r != rows; ++r)
              memcpy (static_cast<unsigned char*> (l.bits) +
                      static_cast<size_t> (r) * l.pitch,
                      static_cast<const unsigned char*> (s.pBits) +
                      static_cast<size_t> (r) * s.Pitch,
                      pitch);

            t.unlock_rect (face, level);

            if (type == D3DRTYPE_CUBETEXTURE)
              static_cast<IDirect3DCubeTexture9*> (d)->UnlockRect (
                static_cast<D3DCUBEMAP_FACES> (face), level);
            else
              static_cast<IDirect3DTexture9*> (d)->UnlockRect (level);
          }
        }
      }

      return true;
    }

    unique_ptr<renderer> vr;

    // Buffer lock hooks. For a dynamic buffer the lock returns a pointer
    // into our copy (see dynamic_buffer_store).
    //
    HRESULT STDMETHODCALLTYPE
    vertex_buffer_lock_hook (IDirect3DVertexBuffer9* b,
                             UINT offset,
                             UINT size,
                             void** data,
                             DWORD flags)
    {
      HRESULT r (vertex_buffer_lock_original (b, offset, size, data, flags));

      if (SUCCEEDED (r))
        *data = dynamic_buffers.locked (b, offset, size, flags, *data);

      return r;
    }

    HRESULT STDMETHODCALLTYPE
    vertex_buffer_unlock_hook (IUnknown* b)
    {
      static_buffers.finished (b);
      dynamic_buffers.unlocking (b);
      return vertex_buffer_unlock_original (b);
    }

    HRESULT STDMETHODCALLTYPE
    index_buffer_lock_hook (IDirect3DIndexBuffer9* b,
                            UINT offset,
                            UINT size,
                            void** data,
                            DWORD flags)
    {
      HRESULT r (index_buffer_lock_original (b, offset, size, data, flags));

      if (SUCCEEDED (r))
        *data = dynamic_buffers.locked (b, offset, size, flags, *data);

      return r;
    }

    HRESULT STDMETHODCALLTYPE
    index_buffer_unlock_hook (IUnknown* b)
    {
      static_buffers.finished (b);
      dynamic_buffers.unlocking (b);
      return index_buffer_unlock_original (b);
    }

    // Hook the buffer locks. Lock() and Unlock() are buffer methods and IW4
    // calls them in many places. However, a Direct3D 9 implementation
    // normally uses one vtable for all vertex buffers and one for all index
    // buffers. So we create a temporary buffer of each kind and patch its
    // vtable.
    //
    bool
    patch_buffer_locks (IDirect3DDevice9* d)
    {
      constexpr size_t lock_slot (11);
      constexpr size_t unlock_slot (12);

      IDirect3DVertexBuffer9* vb (nullptr);
      IDirect3DIndexBuffer9* ib (nullptr);

      if (FAILED (d->CreateVertexBuffer (16,
                                         D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
                                         0,
                                         D3DPOOL_DEFAULT,
                                         &vb,
                                         nullptr)))
        return false;

      if (FAILED (d->CreateIndexBuffer (16,
                                        D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
                                        D3DFMT_INDEX16,
                                        D3DPOOL_DEFAULT,
                                        &ib,
                                        nullptr)))
      {
        vb->Release ();
        return false;
      }

      void** vt (*reinterpret_cast<void***> (vb));
      void** it (*reinterpret_cast<void***> (ib));

      vertex_buffer_lock_original =
        reinterpret_cast<vertex_buffer_lock_t*> (vt[lock_slot]);
      vertex_buffer_unlock_original =
        reinterpret_cast<buffer_unlock_t*> (vt[unlock_slot]);
      index_buffer_lock_original =
        reinterpret_cast<index_buffer_lock_t*> (it[lock_slot]);
      index_buffer_unlock_original =
        reinterpret_cast<buffer_unlock_t*> (it[unlock_slot]);

      Utils::Hook::Set<void*> (&vt[lock_slot],
                               reinterpret_cast<void*> (&vertex_buffer_lock_hook));
      Utils::Hook::Set<void*> (&vt[unlock_slot],
                               reinterpret_cast<void*> (&vertex_buffer_unlock_hook));
      Utils::Hook::Set<void*> (&it[lock_slot],
                               reinterpret_cast<void*> (&index_buffer_lock_hook));
      Utils::Hook::Set<void*> (&it[unlock_slot],
                               reinterpret_cast<void*> (&index_buffer_unlock_hook));

      ib->Release ();
      vb->Release ();
      return true;
    }

    HRESULT STDMETHODCALLTYPE
    device_nop ()
    {
      return D3D_OK;
    }

    // Make the device's state and draw methods do nothing. The device
    // still creates and locks IW4's resources.
    //
    void
    patch_device (IDirect3DDevice9* d)
    {
      constexpr size_t slots[]
      {
        17,  // Present
        21,  // SetGammaRamp
        34,  // StretchRect
        35,  // ColorFill
        37,  // SetRenderTarget
        39,  // SetDepthStencilSurface
        41,  // BeginScene
        42,  // EndScene
        43,  // Clear
        44,  // SetTransform
        47,  // SetViewport
        57,  // SetRenderState
        65,  // SetTexture
        67,  // SetTextureStageState
        69,  // SetSamplerState
        75,  // SetScissorRect
        81,  // DrawPrimitive
        82,  // DrawIndexedPrimitive
        83,  // DrawPrimitiveUP
        84,  // DrawIndexedPrimitiveUP
        87,  // SetVertexDeclaration
        89,  // SetFVF
        92,  // SetVertexShader
        94,  // SetVertexShaderConstantF
        96,  // SetVertexShaderConstantI
        98,  // SetVertexShaderConstantB
        100, // SetStreamSource
        102, // SetStreamSourceFreq
        104, // SetIndices
        107, // SetPixelShader
        109, // SetPixelShaderConstantF
        111, // SetPixelShaderConstantI
        113  // SetPixelShaderConstantB
      };

      void** vt (*reinterpret_cast<void***> (d));

      for (size_t s: slots)
        Utils::Hook::Set<void*> (&vt[s], reinterpret_cast<void*> (&device_nop));
    }

    // Texture unlock hooks (see texture_update_store).
    //
    HRESULT STDMETHODCALLTYPE
    texture_unlock_hook (IDirect3DTexture9* t, UINT level)
    {
      texture_updates.unlocked (t);
      return texture_unlock_original (t, level);
    }

    HRESULT STDMETHODCALLTYPE
    cube_unlock_hook (IDirect3DCubeTexture9* t, D3DCUBEMAP_FACES face, UINT level)
    {
      texture_updates.unlocked (t);
      return cube_unlock_original (t, face, level);
    }

    HRESULT STDMETHODCALLTYPE
    volume_unlock_hook (IDirect3DVolumeTexture9* t, UINT level)
    {
      texture_updates.unlocked (t);
      return volume_unlock_original (t, level);
    }

    // As patch_buffer_locks() but for the texture unlocks, with a temporary
    // texture for each type.
    //
    bool
    patch_texture_unlocks (IDirect3DDevice9* d)
    {
      constexpr size_t unlock_slot (20);

      IDirect3DTexture9* p (nullptr);
      IDirect3DCubeTexture9* c (nullptr);
      IDirect3DVolumeTexture9* v (nullptr);

      bool r (SUCCEEDED (d->CreateTexture (1, 1, 1, 0, D3DFMT_A8R8G8B8,
                                           D3DPOOL_MANAGED, &p, nullptr)) &&
              SUCCEEDED (d->CreateCubeTexture (1, 1, 0, D3DFMT_A8R8G8B8,
                                               D3DPOOL_MANAGED, &c, nullptr)) &&
              SUCCEEDED (d->CreateVolumeTexture (1, 1, 1, 1, 0, D3DFMT_A8R8G8B8,
                                                 D3DPOOL_MANAGED, &v, nullptr)));

      if (r)
      {
        void** pt (*reinterpret_cast<void***> (p));
        void** ct (*reinterpret_cast<void***> (c));
        void** vt (*reinterpret_cast<void***> (v));

        texture_unlock_original =
          reinterpret_cast<texture_unlock_t*> (pt[unlock_slot]);
        cube_unlock_original =
          reinterpret_cast<cube_unlock_t*> (ct[unlock_slot]);
        volume_unlock_original =
          reinterpret_cast<volume_unlock_t*> (vt[unlock_slot]);

        Utils::Hook::Set<void*> (&pt[unlock_slot],
                                 reinterpret_cast<void*> (&texture_unlock_hook));
        Utils::Hook::Set<void*> (&ct[unlock_slot],
                                 reinterpret_cast<void*> (&cube_unlock_hook));
        Utils::Hook::Set<void*> (&vt[unlock_slot],
                                 reinterpret_cast<void*> (&volume_unlock_hook));
      }

      if (v != nullptr)
        v->Release ();

      if (c != nullptr)
        c->Release ();

      if (p != nullptr)
        p->Release ();

      return r;
    }

    // IW4's window parameters while R_CreateDevice() runs. The
    // CreateDevice() call site needs the display size but doesn't have it.
    //
    Game::GfxWindowParms* creating (nullptr);

    HRESULT
    create_device_call_hook (IDirect3D9* d3d,
                             UINT adapter,
                             D3DDEVTYPE type,
                             HWND focus,
                             DWORD flags,
                             D3DPRESENT_PARAMETERS* pp,
                             IDirect3DDevice9** d)
    {
      HRESULT r (d3d->CreateDevice (adapter, type, focus, flags, pp, d));

      if (FAILED (r) || creating == nullptr)
        return r;

      try
      {
        vr = make_unique<renderer> (
          focus,
          static_cast<uint32_t> (creating->displayWidth),
          static_cast<uint32_t> (creating->displayHeight));
      }
      catch (const exception& e)
      {
        fail ("create device", e);
      }

      // Patch the vtables only once. They outlive the device, so patching
      // them again after a device restart would make our hooks call
      // themselves.
      //
      static bool patched (false);

      if (!patched)
      {
        if (!patch_buffer_locks (*d))
          Logger::Print ("Vulkan: unable to hook buffer locks\n");

        if (!patch_texture_unlocks (*d))
          Logger::Print ("Vulkan: unable to hook texture unlocks\n");

        patch_device (*d);

        patched = true;
      }

      return r;
    }

    bool
    r_create_device_hook (Game::GfxWindowParms* wp)
    {
      creating = wp;
      const bool r (r_create_device_original (wp));
      creating = nullptr;

      if (!r || vr == nullptr)
        return r;

      // IW4 presents with call [rax+18h], which should be its swap chain's
      // IDirect3DSwapChain9::Present(). This is not a device method, so
      // patch_device() doesn't cover it. Replace the 3-byte call with
      // xor eax, eax followed by a nop so that IW4's error check sees
      // D3D_OK. We present in rb_swap_buffers_hook().
      //
      static bool patched (false);

      if (!patched)
      {
        Utils::Hook::Set<uint8_t> (rb_swap_present + 0, 0x31);
        Utils::Hook::Set<uint8_t> (rb_swap_present + 1, 0xC0);
        Utils::Hook::Set<uint8_t> (rb_swap_present + 2, 0x90);
        patched = true;
      }

      return true;
    }

    void
    r_shutdown_hook (int destroy_window)
    {
      // If IW4 keeps the window, then this is presumably a restart and IW4
      // will create its objects again. So we keep the renderer and only
      // release our objects.
      //
      if (vr != nullptr)
      {
        if (destroy_window != 0)
          vr.reset ();
        else
          vr->release_objects ();
      }

      if (destroy_window != 0)
      {
        static_buffers.clear ();
        dynamic_buffers.clear ();
      }

      r_shutdown_original (destroy_window);
    }

    void
    rb_swap_buffers_hook (int flags)
    {
      rb_swap_buffers_original (flags);

      if ((flags & swap_present) != 0 && vr != nullptr)
      {
        try
        {
          vr->present ();
        }
        catch (const exception& e)
        {
          fail ("present", e);
        }
      }
    }

    void
    r_draw_tess_technique_hook (const Game::GfxCmdBufContext* c,
                                const Game::GfxDrawPrimArgs* args)
    {
      r_draw_tess_technique_original (c, args);

      if (vr != nullptr)
      {
        try
        {
          vr->draw_tess (*c);
        }
        catch (const exception& e)
        {
          fail ("draw", e);
        }
      }
    }

    // Save the pass context for the DrawIndexedPrimitive() calls that follow
    // (see renderer::draw_indexed()).
    //
    void
    r_setup_pass_hook (const Game::GfxCmdBufContext* c, unsigned int pass)
    {
      r_setup_pass_original (c, pass);

      if (vr != nullptr)
        vr->context (*c);
    }

    HRESULT
    draw_call_hook (IDirect3DDevice9* d,
                    D3DPRIMITIVETYPE type,
                    INT base,
                    UINT min_index,
                    UINT vertices,
                    UINT start,
                    UINT primitives)
    {
      if (vr == nullptr)
        return d->DrawIndexedPrimitive (type, base, min_index, vertices, start, primitives);

      try
      {
        vr->draw_indexed (static_cast<d3d9::primitive_type> (type),
                          base,
                          min_index,
                          vertices,
                          start,
                          primitives);
      }
      catch (const exception& e)
      {
        fail ("draw", e);
      }

      return D3D_OK;
    }

    HRESULT
    custom_texture_call_hook (IDirect3DDevice9* d,
                              DWORD sampler,
                              IDirect3DBaseTexture9* t)
    {
      HRESULT r (d->SetTexture (sampler, t));

      if (vr != nullptr)
        vr->custom_texture (sampler, t);

      return r;
    }

    HRESULT
    viewport_call_hook (IDirect3DDevice9* d, const D3DVIEWPORT9* v)
    {
      HRESULT r (d->SetViewport (v));

      if (vr != nullptr && v != nullptr)
        vr->viewport (*v);

      return r;
    }

    HRESULT
    copy_call_hook (IDirect3DDevice9* d,
                    IDirect3DSurface9* src,
                    const RECT* src_rect,
                    IDirect3DSurface9* dst,
                    const RECT* dst_rect,
                    D3DTEXTUREFILTERTYPE f)
    {
      HRESULT r (d->StretchRect (src, src_rect, dst, dst_rect, f));

      if (vr != nullptr)
      {
        try
        {
          vr->copy (src, src_rect, dst, dst_rect, f);
        }
        catch (const exception& e)
        {
          fail ("copy", e);
        }
      }

      return r;
    }

    void
    gamma_ramp_call_hook (IDirect3DDevice9* d,
                          UINT swapchain,
                          DWORD flags,
                          const D3DGAMMARAMP* r)
    {
      d->SetGammaRamp (swapchain, flags, r);

      if (vr != nullptr && r != nullptr)
        vr->gamma_ramp (*r);
    }

    HRESULT
    update_texture_call_hook (IDirect3DDevice9* d,
                              IDirect3DBaseTexture9* src,
                              IDirect3DBaseTexture9* dst)
    {
      HRESULT r (d->UpdateTexture (src, dst));

      if (vr != nullptr)
      {
        try
        {
          vr->update_texture (src, dst);
        }
        catch (const exception& e)
        {
          fail ("texture update", e);
        }
      }

      return r;
    }

    // IW4's static buffer allocators return the new buffer locked (see
    // static_buffer_store).
    //
    void*
    r_alloc_static_vertex_buffer_hook (IDirect3DVertexBuffer9** b, int size)
    {
      void* r (r_alloc_static_vertex_buffer_original (b, size));

      if (r != nullptr && *b != nullptr && size > 0)
        static_buffers.allocated (*b, r, static_cast<uint32_t> (size), vk_buffer::kind::vertex);

      return r;
    }

    void*
    r_alloc_static_index_buffer_hook (IDirect3DIndexBuffer9** b, int size)
    {
      void* r (r_alloc_static_index_buffer_original (b, size));

      if (r != nullptr && *b != nullptr && size > 0)
        static_buffers.allocated (*b, r, static_cast<uint32_t> (size), vk_buffer::kind::index16);

      return r;
    }

    // Replace IW4's vtable call at a with a call to f. Note that on x64 the
    // interface pointer is the first argument, so f takes the same
    // arguments as the method.
    //
    // The vtable call is call [reg + disp32], which is 6 bytes long, and 7
    // with the REX prefix for r8-r15. We write a 5-byte direct call and nop
    // out the remaining bytes.
    //
    void
    replace_call (uintptr_t a, void* f)
    {
      const uint8_t* c (reinterpret_cast<const uint8_t*> (a));
      const size_t n (c[0] == 0x41 ? 7 : 6);

      assert (c[n - 6] == 0xFF);

      Utils::Hook (a, f, HOOK_CALL).install ()->quick ();
      Utils::Hook::Nop (a + 5, n - 5);
    }

    void
    r_set_render_target_hook (const Game::GfxCmdBufContext* c,
                              Game::GfxRenderTargetId id)
    {
      // Only pass on actual changes since a change resets the viewport (see
      // renderer::target()). IW4 seems to set the same target repeatedly.
      //
      const bool changed (c->state->renderTargetId != id);

      r_set_render_target_original (c, id);

      if (vr != nullptr && changed)
        vr->target (id);
    }

    void
    r_set_viewport_hook (Game::GfxCmdBufState* st, const Game::GfxViewport* v)
    {
      r_set_viewport_original (st, v);

      if (vr != nullptr)
        vr->viewport (*st, *v);
    }

    void
    r_clear_screen_hook (IDirect3DDevice9* d,
                         uint8_t which,
                         const float* color,
                         float depth,
                         uint8_t stencil)
    {
      r_clear_screen_original (d, which, color, depth, stencil);

      if (vr != nullptr)
      {
        try
        {
          vr->clear (which, color, depth, stencil);
        }
        catch (const exception& e)
        {
          fail ("clear", e);
        }
      }
    }

    const Game::dvar_t* r_vulkan (nullptr);
  }

  vulkan::
  vulkan ()
  {
    if (Dedicated::IsEnabled ())
      return;

    // Verify that our Game structure offsets match IW4's.
    //
    AssertOffset (Game::GfxWindowParms,       hwnd,           0x00);
    AssertOffset (Game::GfxWindowParms,       displayWidth,   0x20);
    AssertOffset (Game::GfxWindowParms,       displayHeight,  0x24);
    AssertOffset (Game::GfxCmdBufState,       prim,           0xD0);
    AssertOffset (Game::GfxCmdBufState,       technique,      0x130);
    AssertOffset (Game::GfxCmdBufState,       pass,           0x138);
    AssertOffset (Game::GfxCmdBufState,       depthRangeNear, 0x148);
    AssertOffset (Game::GfxCmdBufState,       scissorX,       0xB78);
    AssertOffset (Game::GfxCmdBufState,       viewport,       0xB88);
    AssertOffset (Game::GfxCmdBufState,       renderTargetId, 0xB98);
    AssertOffset (Game::GfxCmdBufSourceState, input,          0xE00);
    AssertOffset (Game::GfxCmdBufSourceState, eyeOffset,      0x1510);
    AssertOffset (Game::GfxCmdBufSourceState, constVersions,  0x156C);
    AssertOffset (Game::GfxCmdBufSourceState, matrixVersions, 0x1674);
    AssertOffset (Game::GfxCmdBufSourceState, viewMode,       0x16E0);
    AssertOffset (Game::GfxCmdBufSourceState, sceneViewport,  0x1718);
    AssertOffset (Game::GfxCmdBufSourceState, viewportIsDirty, 0x1738);
    AssertOffset (Game::MaterialPass,         args,           0x20);

    Events::OnDvarInit ([] ()
    {
      r_vulkan = Game::Dvar_RegisterBool (
        "r_vulkan",
        false,
        Game::DVAR_ARCHIVE | Game::DVAR_LATCH,
        "Render with Vulkan (applies on restart)");
    });

    // Install the hooks if r_vulkan is enabled. The dvar is latched, so we
    // first apply a +set from the command line (Com_StartupVariable()) and
    // then check the latched value.
    //
    Events::OnClientInit ([] ()
    {
      Game::Com_StartupVariable ("r_vulkan");

      if (r_vulkan == nullptr || !r_vulkan->latched.enabled)
        return;

      r_create_device_original       = Utils::Hook::Detour (r_create_device,       5, &r_create_device_hook);
      r_shutdown_original            = Utils::Hook::Detour (r_shutdown,            5, &r_shutdown_hook);
      rb_swap_buffers_original       = Utils::Hook::Detour (rb_swap_buffers,       7, &rb_swap_buffers_hook);
      r_draw_tess_technique_original = Utils::Hook::Detour (r_draw_tess_technique, 5, &r_draw_tess_technique_hook);
      r_set_render_target_original   = Utils::Hook::Detour (r_set_render_target,   6, &r_set_render_target_hook);
      r_set_viewport_original        = Utils::Hook::Detour (r_set_viewport,        7, &r_set_viewport_hook);
      r_clear_screen_original        = Utils::Hook::Detour (r_clear_screen,        5, &r_clear_screen_hook);
      r_setup_pass_original          = Utils::Hook::Detour (r_setup_pass,          5, &r_setup_pass_hook);

      r_alloc_static_vertex_buffer_original = Utils::Hook::Detour (r_alloc_static_vertex_buffer, 5, &r_alloc_static_vertex_buffer_hook);
      r_alloc_static_index_buffer_original  = Utils::Hook::Detour (r_alloc_static_index_buffer,  6, &r_alloc_static_index_buffer_hook);

      for (uintptr_t a: draw_calls)
        replace_call (a, reinterpret_cast<void*> (&draw_call_hook));

      for (uintptr_t a: copy_calls)
        replace_call (a, reinterpret_cast<void*> (&copy_call_hook));

      replace_call (update_texture_call,
                    reinterpret_cast<void*> (&update_texture_call_hook));

      replace_call (gamma_ramp_call,
                    reinterpret_cast<void*> (&gamma_ramp_call_hook));

      for (uintptr_t a: viewport_calls)
        replace_call (a, reinterpret_cast<void*> (&viewport_call_hook));

      replace_call (create_device_call,
                    reinterpret_cast<void*> (&create_device_call_hook));

      for (uintptr_t a: custom_texture_calls)
        replace_call (a, reinterpret_cast<void*> (&custom_texture_call_hook));
    });
  }
}
