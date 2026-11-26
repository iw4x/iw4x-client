#include "Resources.hpp"

#include "Device.hpp"

#include <algorithm> // max(), min(), ranges::fill()
#include <cstring>   // memcpy()
#include <mutex>

using namespace std;

namespace Components
{
  namespace d3d11
  {
    // private_data
    //
    HRESULT private_data::
    set (REFGUID g, const void* d, DWORD n, DWORD f)
    {
      if (d == nullptr)
        return D3DERR_INVALIDCALL;

      entry e;

      if ((f & D3DSPD_IUNKNOWN) != 0)
      {
        if (n != sizeof (IUnknown*))
          return D3DERR_INVALIDCALL;

        e.object = *static_cast<IUnknown* const*> (d);
      }
      else
      {
        const std::byte* p (static_cast<const std::byte*> (d));
        e.data.assign (p, p + n);
      }

      entries_[g] = move (e);
      return D3D_OK;
    }

    HRESULT private_data::
    get (REFGUID g, void* d, DWORD* n) const
    {
      if (n == nullptr)
        return D3DERR_INVALIDCALL;

      auto i (entries_.find (g));
      if (i == entries_.end ())
        return D3DERR_NOTFOUND;

      const entry& e (i->second);
      const DWORD size (e.object ? sizeof (IUnknown*) : static_cast<DWORD> (e.data.size ()));

      if (d == nullptr)
      {
        *n = size;
        return D3D_OK;
      }

      if (*n < size)
      {
        *n = size;
        return D3DERR_MOREDATA;
      }

      if (e.object)
      {
        IUnknown* o (e.object.Get ());
        o->AddRef ();
        memcpy (d, &o, sizeof (o));
      }
      else
        memcpy (d, e.data.data (), size);

      *n = size;
      return D3D_OK;
    }

    HRESULT private_data::
    free (REFGUID g)
    {
      return entries_.erase (g) != 0 ? D3D_OK : D3DERR_NOTFOUND;
    }

    // buffer
    //
    buffer::
    buffer (device& d, UINT size, DWORD usage, D3DPOOL pool, UINT bind_flags)
      : device_ (d),
        size_ (size),
        usage_ (usage),
        pool_ (pool),
        bind_flags_ (bind_flags)
    {
    }

    HRESULT buffer::
    create ()
    {
      if (size_ == 0)
        return D3DERR_INVALIDCALL;

      D3D11_BUFFER_DESC d {};
      d.ByteWidth = size_;
      d.BindFlags = bind_flags_;

      if ((usage_ & D3DUSAGE_DYNAMIC) != 0)
      {
        d.Usage = D3D11_USAGE_DYNAMIC;
        d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      }
      else
        d.Usage = D3D11_USAGE_DEFAULT;

      if (FAILED (device_.d3d ()->CreateBuffer (&d, nullptr, &buffer_)))
        return D3DERR_OUTOFVIDEOMEMORY;

      data_.resize (size_);
      return D3D_OK;
    }

    HRESULT buffer::
    lock (UINT offset, UINT size, void** data, DWORD flags)
    {
      if (data == nullptr || offset > size_)
        return D3DERR_INVALIDCALL;

      // Zero size means the rest of the buffer from the offset.
      //
      const UINT end (size == 0 ? size_ : min (size_, offset + size));

      if (locks_++ == 0)
      {
        lock_begin_ = offset;
        lock_end_   = end;
        lock_flags_ = flags;
      }
      else
      {
        // If any of the nested locks writes, then we have to upload, and
        // we can only rely on not overwriting if all of them promise so.
        //
        lock_begin_ = min (lock_begin_, offset);
        lock_end_   = max (lock_end_, end);
        lock_flags_ = (lock_flags_ & flags & (D3DLOCK_READONLY | D3DLOCK_NOOVERWRITE)) |
                      ((lock_flags_ | flags) & D3DLOCK_DISCARD);
      }

      *data = data_.data () + offset;
      return D3D_OK;
    }

    HRESULT buffer::
    unlock ()
    {
      if (locks_ == 0)
        return D3DERR_INVALIDCALL;

      if (--locks_ != 0 || (lock_flags_ & D3DLOCK_READONLY) != 0 ||
          lock_begin_ == lock_end_)
        return D3D_OK;

      lock_guard l (device_.mutex ());
      ID3D11DeviceContext1* c (device_.context ());

      if ((usage_ & D3DUSAGE_DYNAMIC) != 0)
      {
        // With D3DLOCK_DISCARD the rest of the buffer is undefined in D3D9
        // as well, so we only send what was locked. Without either flag
        // the game expects the rest of the buffer to survive, so we send
        // all of it.
        //
        const bool overwrite ((lock_flags_ & D3DLOCK_NOOVERWRITE) != 0 &&
                              (lock_flags_ & D3DLOCK_DISCARD)     == 0);
        const bool discard   ((lock_flags_ & D3DLOCK_DISCARD) != 0);

        D3D11_MAPPED_SUBRESOURCE m;
        if (FAILED (c->Map (buffer_.Get (),
                            0,
                            overwrite ? D3D11_MAP_WRITE_NO_OVERWRITE : D3D11_MAP_WRITE_DISCARD,
                            0,
                            &m)))
          return D3DERR_INVALIDCALL;

        if (overwrite || discard)
          memcpy (static_cast<std::byte*> (m.pData) + lock_begin_,
                  data_.data () + lock_begin_,
                  lock_end_ - lock_begin_);
        else
          memcpy (m.pData, data_.data (), size_);

        c->Unmap (buffer_.Get (), 0);
      }
      else
      {
        const D3D11_BOX b {lock_begin_, 0, 0, lock_end_, 1, 1};
        c->UpdateSubresource (buffer_.Get (), 0, &b, data_.data () + lock_begin_, 0, 0);
      }

      return D3D_OK;
    }

    // vertex_buffer
    //
    vertex_buffer::
    vertex_buffer (device& d, UINT size, DWORD usage, DWORD fvf, D3DPOOL pool)
      : resource (d, D3DRTYPE_VERTEXBUFFER),
        storage (d, size, usage, pool, D3D11_BIND_VERTEX_BUFFER),
        fvf_ (fvf)
    {
    }

    HRESULT vertex_buffer::
    Lock (UINT offset, UINT size, void** data, DWORD flags)
    {
      return storage.lock (offset, size, data, flags);
    }

    HRESULT vertex_buffer::
    Unlock ()
    {
      return storage.unlock ();
    }

    HRESULT vertex_buffer::
    GetDesc (D3DVERTEXBUFFER_DESC* d)
    {
      if (d == nullptr)
        return D3DERR_INVALIDCALL;

      d->Format = D3DFMT_VERTEXDATA;
      d->Type   = D3DRTYPE_VERTEXBUFFER;
      d->Usage  = storage.usage ();
      d->Pool   = storage.pool ();
      d->Size   = storage.size ();
      d->FVF    = fvf_;
      return D3D_OK;
    }

    // index_buffer
    //
    index_buffer::
    index_buffer (device& d, UINT size, DWORD usage, D3DFORMAT f, D3DPOOL pool)
      : resource (d, D3DRTYPE_INDEXBUFFER),
        storage (d, size, usage, pool, D3D11_BIND_INDEX_BUFFER),
        format_ (f)
    {
    }

    HRESULT index_buffer::
    Lock (UINT offset, UINT size, void** data, DWORD flags)
    {
      return storage.lock (offset, size, data, flags);
    }

    HRESULT index_buffer::
    Unlock ()
    {
      return storage.unlock ();
    }

    HRESULT index_buffer::
    GetDesc (D3DINDEXBUFFER_DESC* d)
    {
      if (d == nullptr)
        return D3DERR_INVALIDCALL;

      d->Format = format_;
      d->Type   = D3DRTYPE_INDEXBUFFER;
      d->Usage  = storage.usage ();
      d->Pool   = storage.pool ();
      d->Size   = storage.size ();
      return D3D_OK;
    }

    // image
    //
    image::
    image (device& d, const image_desc& desc, const format_info* f)
      : device_ (d), desc_ (desc), format_ (f)
    {
    }

    HRESULT image::
    create ()
    {
      const UINT subresources (desc_.levels * layers ());

      views_.resize (subresources);
      locks_.resize (subresources);

      // The NULL format has no storage at all, only a size.
      //
      if (null ())
        return D3D_OK;

      // The system memory and scratch pools only exist on the CPU. The
      // game can't draw with them, only copy them (with UpdateTexture()
      // and friends) and lock them.
      //
      const bool cpu_only (desc_.pool == D3DPOOL_SYSTEMMEM ||
                           desc_.pool == D3DPOOL_SCRATCH);

      if (desc_.pool == D3DPOOL_MANAGED      ||
          cpu_only                           ||
          (desc_.usage & D3DUSAGE_DYNAMIC) != 0 ||
          desc_.lockable)
      {
        copies_.resize (subresources);
        for (UINT s (0); s != subresources; ++s)
          copies_[s].resize (static_cast<size_t> (slice_pitch (s)) * depth (level_of (s)));
      }

      if (cpu_only)
        return D3D_OK;

      const bool target ((desc_.usage & D3DUSAGE_RENDERTARGET) != 0);
      const bool depth  ((desc_.usage & D3DUSAGE_DEPTHSTENCIL) != 0);
      const bool ms     (desc_.samples >= D3DMULTISAMPLE_2_SAMPLES);

      UINT bind (0);
      UINT misc (0);

      // We can't sample a multisampled surface the way D3D9 would (it
      // can't either), so there is no point in a shader resource view.
      //
      if (!ms)
        bind |= D3D11_BIND_SHADER_RESOURCE;

      if (target)
        bind |= D3D11_BIND_RENDER_TARGET;

      if (depth)
        bind |= D3D11_BIND_DEPTH_STENCIL;

      // A plain surface in the default pool isn't something D3D9 renders
      // to but ColorFill() and StretchRect() write to it, which for us
      // means a render target view. So let's make it one if the format
      // allows.
      //
      if (desc_.type == image_type::surface && !target && !depth)
      {
        UINT s (0);
        if (SUCCEEDED (device_.d3d ()->CheckFormatSupport (format_->view, &s)) &&
            (s & D3D11_FORMAT_SUPPORT_RENDER_TARGET) != 0)
          bind |= D3D11_BIND_RENDER_TARGET;
      }

      if ((desc_.usage & D3DUSAGE_AUTOGENMIPMAP) != 0 && desc_.levels > 1)
      {
        bind |= D3D11_BIND_RENDER_TARGET;
        misc |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
      }

      HRESULT hr;

      if (desc_.type == image_type::volume)
      {
        D3D11_TEXTURE3D_DESC d {};
        d.Width     = desc_.width;
        d.Height    = desc_.height;
        d.Depth     = desc_.depth;
        d.MipLevels = desc_.levels;
        d.Format    = format_->resource;
        d.Usage     = D3D11_USAGE_DEFAULT;
        d.BindFlags = bind & ~D3D11_BIND_DEPTH_STENCIL;
        d.MiscFlags = misc;

        ComPtr<ID3D11Texture3D> t;
        hr = device_.d3d ()->CreateTexture3D (&d, nullptr, &t);
        resource_ = t;
      }
      else
      {
        D3D11_TEXTURE2D_DESC d {};
        d.Width              = desc_.width;
        d.Height             = desc_.height;
        d.MipLevels          = desc_.levels;
        d.ArraySize          = layers ();
        d.Format             = format_->resource;
        d.SampleDesc.Count   = ms ? static_cast<UINT> (desc_.samples) : 1;
        d.SampleDesc.Quality = ms ? desc_.quality : 0;
        d.Usage              = D3D11_USAGE_DEFAULT;
        d.BindFlags          = bind;
        d.MiscFlags          = misc | (desc_.type == image_type::cube ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0);

        ComPtr<ID3D11Texture2D> t;
        hr = device_.d3d ()->CreateTexture2D (&d, nullptr, &t);
        resource_ = t;
      }

      if (FAILED (hr))
      {
        Logger::Warning (Game::CON_CHANNEL_GFX,
                         "Couldn't create a {}x{} texture of format {:#x} ({:#x})\n",
                         desc_.width, desc_.height,
                         static_cast<uint32_t> (desc_.format),
                         static_cast<uint32_t> (hr));
        return D3DERR_OUTOFVIDEOMEMORY;
      }

      return D3D_OK;
    }

    UINT image::
    row_pitch (UINT s) const noexcept
    {
      return d3d11::row_pitch (*format_, width (level_of (s)));
    }

    UINT image::
    slice_pitch (UINT s) const noexcept
    {
      const UINT l (level_of (s));
      return row_pitch (s) * mip_blocks (*format_, desc_.height, l);
    }

    ID3D11ShaderResourceView* image::
    shader_view (bool srgb)
    {
      if (resource_ == nullptr || desc_.samples >= D3DMULTISAMPLE_2_SAMPLES)
        return nullptr;

      ComPtr<ID3D11ShaderResourceView>& v (shader_[srgb ? 1 : 0]);

      if (v == nullptr)
      {
        const UINT first (min (lod_, desc_.levels - 1));

        D3D11_SHADER_RESOURCE_VIEW_DESC d {};
        d.Format = srgb ? format_->view_srgb : format_->view;

        switch (desc_.type)
        {
          case image_type::cube:
          {
            d.ViewDimension               = D3D11_SRV_DIMENSION_TEXTURECUBE;
            d.TextureCube.MostDetailedMip = first;
            d.TextureCube.MipLevels       = desc_.levels - first;
            break;
          }
          case image_type::volume:
          {
            d.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE3D;
            d.Texture3D.MostDetailedMip = first;
            d.Texture3D.MipLevels       = desc_.levels - first;
            break;
          }
          default:
          {
            d.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
            d.Texture2D.MostDetailedMip = first;
            d.Texture2D.MipLevels       = desc_.levels - first;
            break;
          }
        }

        if (FAILED (device_.d3d ()->CreateShaderResourceView (resource_.Get (), &d, &v)))
          return nullptr;
      }

      return v.Get ();
    }

    ID3D11RenderTargetView* image::
    target_view (UINT s, bool srgb)
    {
      if (resource_ == nullptr || s >= views_.size ())
        return nullptr;

      ComPtr<ID3D11RenderTargetView>& v (views_[s].target[srgb ? 1 : 0]);

      if (v == nullptr)
      {
        D3D11_RENDER_TARGET_VIEW_DESC d {};
        d.Format = srgb ? format_->view_srgb : format_->view;

        if (desc_.samples >= D3DMULTISAMPLE_2_SAMPLES)
          d.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
        else if (desc_.type == image_type::cube)
        {
          d.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
          d.Texture2DArray.MipSlice        = level_of (s);
          d.Texture2DArray.FirstArraySlice = s / desc_.levels;
          d.Texture2DArray.ArraySize       = 1;
        }
        else
        {
          d.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
          d.Texture2D.MipSlice = level_of (s);
        }

        if (FAILED (device_.d3d ()->CreateRenderTargetView (resource_.Get (), &d, &v)))
          return nullptr;
      }

      return v.Get ();
    }

    ID3D11DepthStencilView* image::
    depth_view (UINT s)
    {
      if (resource_ == nullptr || s >= views_.size () || !depth_format (*format_))
        return nullptr;

      ComPtr<ID3D11DepthStencilView>& v (views_[s].depth);

      if (v == nullptr)
      {
        D3D11_DEPTH_STENCIL_VIEW_DESC d {};
        d.Format = format_->depth;

        if (desc_.samples >= D3DMULTISAMPLE_2_SAMPLES)
          d.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        else
        {
          d.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
          d.Texture2D.MipSlice = level_of (s);
        }

        if (FAILED (device_.d3d ()->CreateDepthStencilView (resource_.Get (), &d, &v)))
          return nullptr;
      }

      return v.Get ();
    }

    HRESULT image::
    lock (UINT s, D3DLOCKED_BOX& r, const D3DBOX* b, DWORD flags)
    {
      if (s >= locks_.size () || copies_.empty ())
        return D3DERR_INVALIDCALL;

      const UINT l (level_of (s));

      D3DBOX box {0, 0, width (l), height (l), 0, depth (l)};
      if (b != nullptr)
      {
        if (b->Left >= b->Right || b->Top >= b->Bottom || b->Front >= b->Back ||
            b->Right > box.Right || b->Bottom > box.Bottom || b->Back > box.Back)
          return D3DERR_INVALIDCALL;

        box = *b;
      }

      lock_state& ls (locks_[s]);

      // Reading a lockable render target means reading what the GPU drew.
      //
      if (ls.count == 0 && desc_.lockable && resource_ != nullptr &&
          (flags & D3DLOCK_DISCARD) == 0)
      {
        if (HRESULT hr = read_back (s); FAILED (hr))
          return hr;
      }

      if (ls.count++ == 0)
      {
        ls.box   = box;
        ls.flags = flags;
      }
      else
      {
        ls.box.Left   = min (ls.box.Left,   box.Left);
        ls.box.Top    = min (ls.box.Top,    box.Top);
        ls.box.Front  = min (ls.box.Front,  box.Front);
        ls.box.Right  = max (ls.box.Right,  box.Right);
        ls.box.Bottom = max (ls.box.Bottom, box.Bottom);
        ls.box.Back   = max (ls.box.Back,   box.Back);
        ls.flags     &= flags & D3DLOCK_READONLY;
      }

      const UINT row   (row_pitch (s));
      const UINT slice (slice_pitch (s));
      const UINT bk    (format_->block);

      r.RowPitch   = static_cast<INT> (row);
      r.SlicePitch = static_cast<INT> (slice);
      r.pBits      = copies_[s].data () +
                     static_cast<size_t> (box.Front) * slice +
                     static_cast<size_t> (box.Top / bk) * row +
                     static_cast<size_t> (box.Left / bk) * format_->size;
      return D3D_OK;
    }

    HRESULT image::
    unlock (UINT s)
    {
      if (s >= locks_.size () || locks_[s].count == 0)
        return D3DERR_INVALIDCALL;

      lock_state& ls (locks_[s]);
      if (--ls.count != 0)
        return D3D_OK;

      if ((ls.flags & D3DLOCK_READONLY) == 0)
        upload (s, &ls.box);

      return D3D_OK;
    }

    void image::
    upload (UINT s, const D3DBOX* b)
    {
      if (!copies_.empty ())
        send (s, copies_[s].data (), b);
    }

    void image::
    send (UINT s, const std::byte* base, const D3DBOX* b)
    {
      if (resource_ == nullptr)
        return;

      const UINT l (level_of (s));
      const UINT bk (format_->block);

      // Block compressed formats can only be updated in whole blocks, which
      // gets tricky for the levels smaller than a block. Partial locks of
      // these seem rare enough that we just send the whole level.
      //
      D3DBOX box {0, 0, width (l), height (l), 0, depth (l)};
      bool whole (b == nullptr || bk != 1);
      if (!whole)
      {
        box = *b;
        whole = box.Left == 0 && box.Top == 0 && box.Front == 0 &&
                box.Right == width (l) && box.Bottom == height (l) &&
                box.Back == depth (l);
      }

      if (whole)
        box = D3DBOX {0, 0, width (l), height (l), 0, depth (l)};

      const UINT row   (row_pitch (s));
      const UINT slice (slice_pitch (s));

      const std::byte* p (base +
                     static_cast<size_t> (box.Front) * slice +
                     static_cast<size_t> (box.Top / bk) * row +
                     static_cast<size_t> (box.Left / bk) * format_->size);

      const UINT blocks_wide ((box.Right - box.Left + bk - 1) / bk);
      const UINT blocks_high ((box.Bottom - box.Top + bk - 1) / bk);
      const UINT slices      (box.Back - box.Front);

      UINT dst_row   (row);
      UINT dst_slice (slice);

      // Expand the formats that D3D11 doesn't have into the ones we
      // created the texture with.
      //
      vector<std::byte> converted;
      if (format_->convert != conversion::none)
      {
        dst_row   = blocks_wide * format_->card_size;
        dst_slice = dst_row * blocks_high;

        converted.resize (static_cast<size_t> (dst_slice) * slices);

        for (UINT z (0); z != slices; ++z)
          convert (*format_,
                   p + static_cast<size_t> (z) * slice, row,
                   converted.data () + static_cast<size_t> (z) * dst_slice, dst_row,
                   blocks_wide, blocks_high);

        p = converted.data ();
      }

      const D3D11_BOX d {box.Left, box.Top, box.Front, box.Right, box.Bottom, box.Back};

      lock_guard g (device_.mutex ());
      device_.context ()->UpdateSubresource (resource_.Get (),
                                             s,
                                             whole ? nullptr : &d,
                                             p,
                                             dst_row,
                                             dst_slice);
    }

    HRESULT image::
    copy_from (UINT s, image& from, UINT fs)
    {
      if (s >= views_.size () || fs >= from.views_.size () ||
          format_ != from.format_)
        return D3DERR_INVALIDCALL;

      const UINT l (level_of (s));
      const UINT fl (from.level_of (fs));

      if (width (l) != from.width (fl) || height (l) != from.height (fl) ||
          depth (l) != from.depth (fl))
        return D3DERR_INVALIDCALL;

      // Prefer the system memory copy if the source has one, which is the
      // usual case of a system memory texture updating a default one.
      //
      if (!from.copies_.empty ())
      {
        if (!copies_.empty ())
        {
          copies_[s] = from.copies_[fs];
          upload (s, nullptr);
          return D3D_OK;
        }

        if (resource_ == nullptr)
          return D3DERR_INVALIDCALL;

        // No copy of our own, so upload straight from the source's.
        //
        send (s, from.copies_[fs].data (), nullptr);
        return D3D_OK;
      }

      if (resource_ == nullptr || from.resource_ == nullptr)
        return D3DERR_INVALIDCALL;

      lock_guard g (device_.mutex ());
      device_.context ()->CopySubresourceRegion (resource_.Get (), s, 0, 0, 0,
                                                 from.resource_.Get (), fs,
                                                 nullptr);
      return D3D_OK;
    }

    HRESULT image::
    read_back (UINT s)
    {
      if (resource_ == nullptr || copies_.empty () ||
          desc_.samples >= D3DMULTISAMPLE_2_SAMPLES ||
          format_->convert != conversion::none ||
          desc_.type == image_type::volume)
        return D3DERR_INVALIDCALL;

      const UINT l (level_of (s));

      D3D11_TEXTURE2D_DESC d {};
      d.Width            = width (l);
      d.Height           = height (l);
      d.MipLevels        = 1;
      d.ArraySize        = 1;
      d.Format           = format_->resource;
      d.SampleDesc.Count = 1;
      d.Usage            = D3D11_USAGE_STAGING;
      d.CPUAccessFlags   = D3D11_CPU_ACCESS_READ;

      ComPtr<ID3D11Texture2D> staging;
      if (FAILED (device_.d3d ()->CreateTexture2D (&d, nullptr, &staging)))
        return D3DERR_OUTOFVIDEOMEMORY;

      lock_guard g (device_.mutex ());
      ID3D11DeviceContext1* c (device_.context ());

      c->CopySubresourceRegion (staging.Get (), 0, 0, 0, 0, resource_.Get (), s, nullptr);

      D3D11_MAPPED_SUBRESOURCE m;
      if (FAILED (c->Map (staging.Get (), 0, D3D11_MAP_READ, 0, &m)))
        return D3DERR_INVALIDCALL;

      const UINT row (row_pitch (s));
      const UINT rows (mip_blocks (*format_, desc_.height, l));

      for (UINT y (0); y != rows; ++y)
        memcpy (copies_[s].data () + static_cast<size_t> (y) * row,
                static_cast<const std::byte*> (m.pData) + static_cast<size_t> (y) * m.RowPitch,
                row);

      c->Unmap (staging.Get (), 0);
      return D3D_OK;
    }

    HRESULT image::
    fill (UINT s, const RECT* r, D3DCOLOR c)
    {
      if (ID3D11RenderTargetView* v = target_view (s, false))
      {
        const float color[4] {((c >> 16) & 0xFF) / 255.0f,
                              ((c >> 8)  & 0xFF) / 255.0f,
                              ( c        & 0xFF) / 255.0f,
                              ((c >> 24) & 0xFF) / 255.0f};

        lock_guard g (device_.mutex ());
        device_.context ()->ClearView (v, color, r, r != nullptr ? 1 : 0);
        return D3D_OK;
      }

      // Otherwise fill the system memory copy, which we can only do for
      // the 32-bit RGB formats (which is what ColorFill() is about anyway).
      //
      if (copies_.empty () || format_->block != 1 || format_->size != 4 ||
          format_->convert != conversion::none)
        return D3DERR_INVALIDCALL;

      const UINT l (level_of (s));
      const RECT whole {0, 0, static_cast<LONG> (width (l)), static_cast<LONG> (height (l))};
      const RECT& f (r != nullptr ? *r : whole);
      const UINT row (row_pitch (s));

      // A8B8G8R8 stores red first, unlike the others.
      //
      D3DCOLOR v (c);
      if (desc_.format == D3DFMT_A8B8G8R8)
        v = (c & 0xFF00FF00) | ((c >> 16) & 0xFF) | ((c & 0xFF) << 16);

      for (LONG y (f.top); y < f.bottom; ++y)
        for (LONG x (f.left); x < f.right; ++x)
          memcpy (copies_[s].data () + static_cast<size_t> (y) * row + static_cast<size_t> (x) * 4,
                  &v,
                  4);

      upload (s, nullptr);
      return D3D_OK;
    }

    void image::
    set_lod (UINT l) noexcept
    {
      if (l != lod_)
      {
        lod_ = l;
        shader_[0].Reset ();
        shader_[1].Reset ();
      }
    }

    void image::
    generate_mips ()
    {
      if ((desc_.usage & D3DUSAGE_AUTOGENMIPMAP) == 0)
        return;

      if (ID3D11ShaderResourceView* v = shader_view (false))
      {
        lock_guard g (device_.mutex ());
        device_.context ()->GenerateMips (v);
      }
    }

    // Textures.
    //
    texture_object*
    texture_of (IDirect3DBaseTexture9* t) noexcept
    {
      if (t == nullptr)
        return nullptr;

      switch (t->GetType ())
      {
        case D3DRTYPE_TEXTURE:
          return static_cast<texture*> (static_cast<IDirect3DTexture9*> (t));
        case D3DRTYPE_CUBETEXTURE:
          return static_cast<cube_texture*> (static_cast<IDirect3DCubeTexture9*> (t));
        case D3DRTYPE_VOLUMETEXTURE:
          return static_cast<volume_texture*> (static_cast<IDirect3DVolumeTexture9*> (t));
        default:
          return nullptr;
      }
    }

    namespace
    {
      void
      surface_desc (image& i, UINT level, D3DSURFACE_DESC& d)
      {
        const image_desc& id (i.desc ());

        d.Format             = id.format;
        d.Type               = D3DRTYPE_SURFACE;
        d.Usage              = id.usage;
        d.Pool               = id.pool;
        d.MultiSampleType    = id.samples;
        d.MultiSampleQuality = id.quality;
        d.Width              = i.width (level);
        d.Height             = i.height (level);
      }

      D3DBOX
      rect_box (const RECT& r)
      {
        return D3DBOX {static_cast<UINT> (r.left),
                       static_cast<UINT> (r.top),
                       static_cast<UINT> (r.right),
                       static_cast<UINT> (r.bottom),
                       0,
                       1};
      }

      HRESULT
      lock_rect (image& i, UINT s, D3DLOCKED_RECT* r, const RECT* rect, DWORD flags)
      {
        if (r == nullptr)
          return D3DERR_INVALIDCALL;

        D3DBOX b;
        if (rect != nullptr)
          b = rect_box (*rect);

        D3DLOCKED_BOX lb;
        HRESULT hr (i.lock (s, lb, rect != nullptr ? &b : nullptr, flags));
        if (FAILED (hr))
        {
          r->pBits = nullptr;
          r->Pitch = 0;
          return hr;
        }

        r->pBits = lb.pBits;
        r->Pitch = lb.RowPitch;
        return D3D_OK;
      }
    }

    texture::
    texture (device& d, unique_ptr<image> i)
      : base_texture (d, D3DRTYPE_TEXTURE, move (i))
    {
      const UINT n (image_->desc ().levels);
      levels_.reserve (n);

      for (UINT l (0); l != n; ++l)
        levels_.push_back (make_unique<surface> (d,
                                                 *this,
                                                 static_cast<IDirect3DTexture9*> (this),
                                                 l,
                                                 0));
    }

    texture::
    ~texture () = default;

    HRESULT texture::
    GetLevelDesc (UINT l, D3DSURFACE_DESC* d)
    {
      if (d == nullptr || l >= levels_.size ())
        return D3DERR_INVALIDCALL;

      surface_desc (*image_, l, *d);
      return D3D_OK;
    }

    HRESULT texture::
    GetSurfaceLevel (UINT l, IDirect3DSurface9** r)
    {
      if (r == nullptr || l >= levels_.size ())
        return D3DERR_INVALIDCALL;

      *r = levels_[l].get ();
      (*r)->AddRef ();
      return D3D_OK;
    }

    HRESULT texture::
    LockRect (UINT l, D3DLOCKED_RECT* r, const RECT* rect, DWORD flags)
    {
      if (l >= levels_.size ())
        return D3DERR_INVALIDCALL;

      return lock_rect (*image_, image_->subresource (l, 0), r, rect, flags);
    }

    HRESULT texture::
    UnlockRect (UINT l)
    {
      if (l >= levels_.size ())
        return D3DERR_INVALIDCALL;

      return image_->unlock (image_->subresource (l, 0));
    }

    HRESULT texture::
    AddDirtyRect (const RECT*)
    {
      // We upload on unlock and copy whole levels in UpdateTexture(), so
      // there is nothing to track.
      //
      return D3D_OK;
    }

    cube_texture::
    cube_texture (device& d, unique_ptr<image> i)
      : base_texture (d, D3DRTYPE_CUBETEXTURE, move (i))
    {
      const UINT n (image_->desc ().levels);
      surfaces_.reserve (n * 6);

      for (UINT f (0); f != 6; ++f)
        for (UINT l (0); l != n; ++l)
          surfaces_.push_back (make_unique<surface> (d,
                                                     *this,
                                                     static_cast<IDirect3DCubeTexture9*> (this),
                                                     l,
                                                     f));
    }

    cube_texture::
    ~cube_texture () = default;

    HRESULT cube_texture::
    GetLevelDesc (UINT l, D3DSURFACE_DESC* d)
    {
      if (d == nullptr || l >= image_->desc ().levels)
        return D3DERR_INVALIDCALL;

      surface_desc (*image_, l, *d);
      return D3D_OK;
    }

    HRESULT cube_texture::
    GetCubeMapSurface (D3DCUBEMAP_FACES f, UINT l, IDirect3DSurface9** r)
    {
      const UINT n (image_->desc ().levels);
      if (r == nullptr || static_cast<UINT> (f) >= 6 || l >= n)
        return D3DERR_INVALIDCALL;

      *r = surfaces_[static_cast<UINT> (f) * n + l].get ();
      (*r)->AddRef ();
      return D3D_OK;
    }

    HRESULT cube_texture::
    LockRect (D3DCUBEMAP_FACES f, UINT l, D3DLOCKED_RECT* r, const RECT* rect, DWORD flags)
    {
      if (static_cast<UINT> (f) >= 6 || l >= image_->desc ().levels)
        return D3DERR_INVALIDCALL;

      return lock_rect (*image_, image_->subresource (l, f), r, rect, flags);
    }

    HRESULT cube_texture::
    UnlockRect (D3DCUBEMAP_FACES f, UINT l)
    {
      if (static_cast<UINT> (f) >= 6 || l >= image_->desc ().levels)
        return D3DERR_INVALIDCALL;

      return image_->unlock (image_->subresource (l, f));
    }

    HRESULT cube_texture::
    AddDirtyRect (D3DCUBEMAP_FACES, const RECT*)
    {
      return D3D_OK;
    }

    volume_texture::
    volume_texture (device& d, unique_ptr<image> i)
      : base_texture (d, D3DRTYPE_VOLUMETEXTURE, move (i))
    {
      const UINT n (image_->desc ().levels);
      levels_.reserve (n);

      for (UINT l (0); l != n; ++l)
        levels_.push_back (make_unique<volume> (d,
                                                *this,
                                                static_cast<IDirect3DVolumeTexture9*> (this),
                                                l));
    }

    volume_texture::
    ~volume_texture () = default;

    HRESULT volume_texture::
    GetLevelDesc (UINT l, D3DVOLUME_DESC* d)
    {
      if (d == nullptr || l >= levels_.size ())
        return D3DERR_INVALIDCALL;

      return levels_[l]->GetDesc (d);
    }

    HRESULT volume_texture::
    GetVolumeLevel (UINT l, IDirect3DVolume9** r)
    {
      if (r == nullptr || l >= levels_.size ())
        return D3DERR_INVALIDCALL;

      *r = levels_[l].get ();
      (*r)->AddRef ();
      return D3D_OK;
    }

    HRESULT volume_texture::
    LockBox (UINT l, D3DLOCKED_BOX* r, const D3DBOX* b, DWORD flags)
    {
      if (r == nullptr || l >= levels_.size ())
        return D3DERR_INVALIDCALL;

      return image_->lock (image_->subresource (l, 0), *r, b, flags);
    }

    HRESULT volume_texture::
    UnlockBox (UINT l)
    {
      if (l >= levels_.size ())
        return D3DERR_INVALIDCALL;

      return image_->unlock (image_->subresource (l, 0));
    }

    HRESULT volume_texture::
    AddDirtyBox (const D3DBOX*)
    {
      return D3D_OK;
    }

    // surface
    //
    surface::
    surface (device& d, texture_object& container, IUnknown* com, UINT level, UINT layer)
      : resource (d, D3DRTYPE_SURFACE),
        image_ (&container.storage ()),
        container_ (&container),
        com_ (com),
        level_ (level),
        layer_ (layer)
    {
    }

    surface::
    surface (device& d, unique_ptr<image> i)
      : resource (d, D3DRTYPE_SURFACE),
        image_ (i.get ()),
        owned_ (move (i))
    {
    }

    ULONG surface::
    AddRef ()
    {
      return com_ != nullptr ? com_->AddRef () : resource::AddRef ();
    }

    ULONG surface::
    Release ()
    {
      return com_ != nullptr ? com_->Release () : resource::Release ();
    }

    void surface::
    private_add_ref () noexcept
    {
      if (container_ != nullptr)
        container_->private_add_ref ();
      else
        resource::private_add_ref ();
    }

    void surface::
    private_release () noexcept
    {
      if (container_ != nullptr)
        container_->private_release ();
      else
        resource::private_release ();
    }

    bool surface::
    implements (REFIID riid) const noexcept
    {
      return resource::implements (riid);
    }

    HRESULT surface::
    GetContainer (REFIID riid, void** r)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      // A standalone surface's container is the device.
      //
      if (com_ != nullptr)
        return com_->QueryInterface (riid, r);

      IDirect3DDevice9* d;
      if (FAILED (get_device (device_, &d)))
        return D3DERR_INVALIDCALL;

      HRESULT hr (d->QueryInterface (riid, r));
      d->Release ();
      return hr;
    }

    HRESULT surface::
    GetDesc (D3DSURFACE_DESC* d)
    {
      if (d == nullptr)
        return D3DERR_INVALIDCALL;

      surface_desc (*image_, level_, *d);
      return D3D_OK;
    }

    HRESULT surface::
    LockRect (D3DLOCKED_RECT* r, const RECT* rect, DWORD flags)
    {
      return lock_rect (*image_, subresource (), r, rect, flags);
    }

    HRESULT surface::
    UnlockRect ()
    {
      return image_->unlock (subresource ());
    }

    HRESULT surface::
    GetDC (HDC*)
    {
      return D3DERR_INVALIDCALL;
    }

    HRESULT surface::
    ReleaseDC (HDC)
    {
      return D3DERR_INVALIDCALL;
    }

    // volume
    //
    volume::
    volume (device& d, texture_object& container, IUnknown* com, UINT level)
      : device_ (d), container_ (container), com_ (com), level_ (level)
    {
    }

    ULONG volume::
    AddRef ()
    {
      return com_->AddRef ();
    }

    ULONG volume::
    Release ()
    {
      return com_->Release ();
    }

    HRESULT volume::
    GetDevice (IDirect3DDevice9** r)
    {
      return get_device (device_, r);
    }

    HRESULT volume::
    SetPrivateData (REFGUID g, const void* d, DWORD n, DWORD f)
    {
      return data_.set (g, d, n, f);
    }

    HRESULT volume::
    GetPrivateData (REFGUID g, void* d, DWORD* n)
    {
      return data_.get (g, d, n);
    }

    HRESULT volume::
    FreePrivateData (REFGUID g)
    {
      return data_.free (g);
    }

    HRESULT volume::
    GetContainer (REFIID riid, void** r)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      return com_->QueryInterface (riid, r);
    }

    HRESULT volume::
    GetDesc (D3DVOLUME_DESC* d)
    {
      if (d == nullptr)
        return D3DERR_INVALIDCALL;

      image& i (container_.storage ());
      const image_desc& id (i.desc ());

      d->Format = id.format;
      d->Type   = D3DRTYPE_VOLUME;
      d->Usage  = id.usage;
      d->Pool   = id.pool;
      d->Width  = i.width (level_);
      d->Height = i.height (level_);
      d->Depth  = i.depth (level_);
      return D3D_OK;
    }

    HRESULT volume::
    LockBox (D3DLOCKED_BOX* r, const D3DBOX* b, DWORD flags)
    {
      if (r == nullptr)
        return D3DERR_INVALIDCALL;

      image& i (container_.storage ());
      return i.lock (i.subresource (level_, 0), *r, b, flags);
    }

    HRESULT volume::
    UnlockBox ()
    {
      image& i (container_.storage ());
      return i.unlock (i.subresource (level_, 0));
    }
  }
}
