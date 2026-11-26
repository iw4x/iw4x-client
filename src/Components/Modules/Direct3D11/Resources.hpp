#pragma once

#include "Formats.hpp"
#include "Object.hpp"

#include <wrl/client.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <vector>

namespace Components
{
  namespace d3d11
  {
    class device;

    using Microsoft::WRL::ComPtr;

    // Return the device as IDirect3DDevice9, with a reference added. This
    // is what every object's GetDevice() returns.
    //
    HRESULT
    get_device (device&, IDirect3DDevice9**);

    // The private data that D3D9 lets the game attach to a resource (see
    // SetPrivateData()). Nothing in IW4 seems to use it but it's cheap to
    // support and D3DX might.
    //
    class private_data
    {
    public:
      HRESULT
      set (REFGUID, const void*, DWORD size, DWORD flags);

      HRESULT
      get (REFGUID, void*, DWORD* size) const;

      HRESULT
      free (REFGUID);

    private:
      struct guid_less
      {
        bool
        operator() (const GUID& x, const GUID& y) const noexcept
        {
          return memcmp (&x, &y, sizeof (GUID)) < 0;
        }
      };

      struct entry
      {
        std::vector<std::byte> data;
        ComPtr<IUnknown>       object; // If set with D3DSPD_IUNKNOWN.
      };

      std::map<GUID, entry, guid_less> entries_;
    };

    // The IDirect3DResource9 part of buffers, textures, and surfaces.
    //
    template <com_interface I>
    class resource: public unknown<I>
    {
    public:
      resource (device& d, D3DRESOURCETYPE t)
        : device_ (d), type_ (t)
      {
      }

      STDMETHOD (GetDevice) (IDirect3DDevice9** r) override
      {
        return get_device (device_, r);
      }

      STDMETHOD (SetPrivateData) (REFGUID g, const void* d, DWORD n, DWORD f) override
      {
        return data_.set (g, d, n, f);
      }

      STDMETHOD (GetPrivateData) (REFGUID g, void* d, DWORD* n) override
      {
        return data_.get (g, d, n);
      }

      STDMETHOD (FreePrivateData) (REFGUID g) override
      {
        return data_.free (g);
      }

      // D3D9 uses the priority to decide which managed resources to evict
      // first. We never evict, so we only remember it.
      //
      STDMETHOD_ (DWORD, SetPriority) (DWORD p) override
      {
        const DWORD r (priority_);
        priority_ = p;
        return r;
      }

      STDMETHOD_ (DWORD, GetPriority) () override
      {
        return priority_;
      }

      STDMETHOD_ (void, PreLoad) () override
      {
      }

      STDMETHOD_ (D3DRESOURCETYPE, GetType) () override
      {
        return type_;
      }

    protected:
      bool
      implements (REFIID riid) const noexcept override
      {
        return riid == __uuidof (IDirect3DResource9) ||
               unknown<I>::implements (riid);
      }

      device& device_;

    private:
      D3DRESOURCETYPE type_;
      DWORD           priority_ = 0;
      private_data    data_;
    };

    // Vertex and index buffers.
    //
    // D3D9 lets the game lock any part of a buffer at any time while D3D11
    // only maps a dynamic buffer whole, either discarding it or promising
    // not to overwrite what the GPU may still read. So we keep a copy of
    // every buffer in system memory, which is what the game locks, and
    // send the locked range to the GPU on unlock:
    //
    // - For dynamic buffers we map with D3D11_MAP_WRITE_NO_OVERWRITE if
    //   the game locked with D3DLOCK_NOOVERWRITE, which is the usual way
    //   of appending to a buffer that the GPU is reading. Otherwise we map
    //   with D3D11_MAP_WRITE_DISCARD and send the whole copy, which keeps
    //   the rest of the buffer intact whatever the game locked.
    //
    // - For static buffers we update the range with UpdateSubresource().
    //
    class buffer
    {
    public:
      buffer (device&, UINT size, DWORD usage, D3DPOOL, UINT bind_flags);

      HRESULT
      create ();

      HRESULT
      lock (UINT offset, UINT size, void** data, DWORD flags);

      HRESULT
      unlock ();

      ID3D11Buffer*
      get () const noexcept
      {
        return buffer_.Get ();
      }

      UINT    size  () const noexcept {return size_;}
      DWORD   usage () const noexcept {return usage_;}
      D3DPOOL pool  () const noexcept {return pool_;}

    private:
      device&               device_;
      UINT                  size_;
      DWORD                 usage_;
      D3DPOOL               pool_;
      UINT                  bind_flags_;
      ComPtr<ID3D11Buffer>  buffer_;
      std::vector<std::byte> data_;

      // The range and flags of the current lock, if any. D3D9 allows
      // nested locks and so do we, merging their ranges.
      //
      UINT  locks_       = 0;
      UINT  lock_begin_  = 0;
      UINT  lock_end_    = 0;
      DWORD lock_flags_  = 0;
    };

    class vertex_buffer: public resource<IDirect3DVertexBuffer9>
    {
    public:
      vertex_buffer (device&, UINT size, DWORD usage, DWORD fvf, D3DPOOL);

      STDMETHOD (Lock) (UINT offset, UINT size, void** data, DWORD flags) override;
      STDMETHOD (Unlock) () override;
      STDMETHOD (GetDesc) (D3DVERTEXBUFFER_DESC*) override;

      buffer storage;

    private:
      DWORD fvf_;
    };

    class index_buffer: public resource<IDirect3DIndexBuffer9>
    {
    public:
      index_buffer (device&, UINT size, DWORD usage, D3DFORMAT, D3DPOOL);

      STDMETHOD (Lock) (UINT offset, UINT size, void** data, DWORD flags) override;
      STDMETHOD (Unlock) () override;
      STDMETHOD (GetDesc) (D3DINDEXBUFFER_DESC*) override;

      DXGI_FORMAT
      format () const noexcept
      {
        return format_ == D3DFMT_INDEX32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
      }

      buffer storage;

    private:
      D3DFORMAT format_;
    };

    // The storage of a texture or a standalone surface: the D3D11 resource,
    // its views, and the system memory copy that the game locks.
    //
    // As with buffers, D3D9 lets the game lock textures in ways D3D11
    // doesn't (and in the D3D9 layout of the format, which isn't always
    // the D3D11 one). So textures that the game can lock (the managed and
    // system memory pools, plus dynamic textures) get a copy in system
    // memory in the D3D9 layout and we upload the locked box on unlock.
    // This is the same copy that D3D9 itself keeps for managed textures.
    // It also means that reading a managed texture back (with
    // D3DLOCK_READONLY, as Streaming does) never touches the GPU.
    //
    // Render targets and depth stencils have no copy. Locking one, which
    // D3D9 only allows for lockable render targets, reads it back from
    // the GPU instead.
    //
    // A subresource is a mipmap level of one of the layers: the faces for
    // a cube texture and a single layer otherwise. The volume texture's
    // depth is part of its levels.
    //
    enum class image_type
    {
      surface, // Render target, depth stencil, or offscreen plain surface.
      texture,
      cube,
      volume
    };

    struct image_desc
    {
      image_type          type;
      UINT                width;
      UINT                height;
      UINT                depth;  // 1 unless a volume texture.
      UINT                levels; // Actual count, never 0.
      DWORD               usage;
      D3DPOOL             pool;
      D3DFORMAT           format;
      D3DMULTISAMPLE_TYPE samples;
      DWORD               quality;
      bool                lockable;
    };

    class image
    {
    public:
      image (device&, const image_desc&, const format_info*);

      // Create the D3D11 resource and the system memory copy.
      //
      HRESULT
      create ();

      const image_desc&   desc   () const noexcept {return desc_;}
      const format_info*  format () const noexcept {return format_;}

      UINT
      layers () const noexcept
      {
        return desc_.type == image_type::cube ? 6 : 1;
      }

      UINT
      subresource (UINT level, UINT layer) const noexcept
      {
        return level + layer * desc_.levels;
      }

      UINT width  (UINT level) const noexcept {return mip_extent (desc_.width, level);}
      UINT height (UINT level) const noexcept {return mip_extent (desc_.height, level);}
      UINT depth  (UINT level) const noexcept {return mip_extent (desc_.depth, level);}

      // Return true if this is the NULL render target format, which has no
      // resource and only fixes the size of the render target.
      //
      bool
      null () const noexcept
      {
        return desc_.format == null_format;
      }

      ID3D11Resource*
      resource () const noexcept
      {
        return resource_.Get ();
      }

      // Return the shader resource view, the sRGB one if requested and the
      // format has one. The view starts at the most detailed mipmap level
      // that SetLOD() selected.
      //
      ID3D11ShaderResourceView*
      shader_view (bool srgb);

      ID3D11RenderTargetView*
      target_view (UINT subresource, bool srgb);

      ID3D11DepthStencilView*
      depth_view (UINT subresource);

      // Lock and unlock a subresource. The box is in texels and null means
      // the whole subresource.
      //
      HRESULT
      lock (UINT subresource, D3DLOCKED_BOX&, const D3DBOX*, DWORD flags);

      HRESULT
      unlock (UINT subresource);

      // Copy the system memory copy of the subresource to the GPU, the box
      // only if specified.
      //
      void
      upload (UINT subresource, const D3DBOX*);

      // Send the data of the subresource, in the D3D9 layout, to the GPU. The
      // box is in texels and null means the whole subresource.
      //
      void
      send (UINT subresource, const std::byte*, const D3DBOX*);

      // Copy the whole subresource from another image of the same format and
      // size, either from its system memory copy or on the GPU.
      //
      HRESULT
      copy_from (UINT subresource, image& from, UINT from_subresource);

      // Copy the subresource from the GPU into the system memory copy.
      //
      HRESULT
      read_back (UINT subresource);

      // Fill the subresource with the color, for ColorFill().
      //
      HRESULT
      fill (UINT subresource, const RECT*, D3DCOLOR);

      void
      set_lod (UINT) noexcept;

      UINT
      lod () const noexcept
      {
        return lod_;
      }

      void
      generate_mips ();

      bool
      has_copy () const noexcept
      {
        return !copies_.empty ();
      }

    private:
      UINT
      level_of (UINT subresource) const noexcept
      {
        return subresource % desc_.levels;
      }

      // Return the row and slice pitch of the subresource in the D3D9 layout.
      //
      UINT row_pitch   (UINT subresource) const noexcept;
      UINT slice_pitch (UINT subresource) const noexcept;

      struct view_set
      {
        ComPtr<ID3D11RenderTargetView> target[2];
        ComPtr<ID3D11DepthStencilView> depth;
      };

      struct lock_state
      {
        UINT    count = 0;
        D3DBOX  box   {};
        DWORD   flags = 0;
      };

      device&                         device_;
      image_desc                      desc_;
      const format_info*              format_;
      ComPtr<ID3D11Resource>          resource_;
      ComPtr<ID3D11ShaderResourceView> shader_[2];
      std::vector<view_set>           views_;
      std::vector<std::vector<std::byte>> copies_;
      std::vector<lock_state>         locks_;
      UINT                            lod_ = 0;
    };

    // The textures. All three keep their storage in an image and hand out
    // their levels as surfaces (volumes for the volume texture).
    //
    // The device only ever deals with textures through texture_object,
    // whatever their interface, which also lets it hold private references
    // to them (see unknown).
    //
    class surface;
    class volume;

    class texture_object
    {
    public:
      virtual void private_add_ref () noexcept = 0;
      virtual void private_release () noexcept = 0;

      image&
      storage () noexcept
      {
        return *image_;
      }

    protected:
      explicit
      texture_object (std::unique_ptr<image> i)
        : image_ (std::move (i))
      {
      }

      ~texture_object () = default;

      std::unique_ptr<image> image_;
    };

    // Return our texture object behind the D3D9 interface.
    //
    texture_object*
    texture_of (IDirect3DBaseTexture9*) noexcept;

    template <com_interface I>
    class base_texture: public resource<I>, public texture_object
    {
    public:
      base_texture (device& d, D3DRESOURCETYPE t, std::unique_ptr<image> i)
        : resource<I> (d, t), texture_object (std::move (i))
      {
      }

      void
      private_add_ref () noexcept override
      {
        resource<I>::private_add_ref ();
      }

      void
      private_release () noexcept override
      {
        resource<I>::private_release ();
      }

      STDMETHOD_ (DWORD, SetLOD) (DWORD l) override
      {
        // D3D9 only honors this for managed textures.
        //
        if (image_->desc ().pool != D3DPOOL_MANAGED)
          return 0;

        const DWORD r (image_->lod ());
        image_->set_lod (l);
        return r;
      }

      STDMETHOD_ (DWORD, GetLOD) () override
      {
        return image_->lod ();
      }

      STDMETHOD_ (DWORD, GetLevelCount) () override
      {
        return image_->desc ().levels;
      }

      STDMETHOD (SetAutoGenFilterType) (D3DTEXTUREFILTERTYPE f) override
      {
        filter_ = f;
        return D3D_OK;
      }

      STDMETHOD_ (D3DTEXTUREFILTERTYPE, GetAutoGenFilterType) () override
      {
        return filter_;
      }

      STDMETHOD_ (void, GenerateMipSubLevels) () override
      {
        image_->generate_mips ();
      }

    protected:
      bool
      implements (REFIID riid) const noexcept override
      {
        return riid == __uuidof (IDirect3DBaseTexture9) ||
               resource<I>::implements (riid);
      }

    private:
      D3DTEXTUREFILTERTYPE filter_ = D3DTEXF_LINEAR;
    };

    class texture: public base_texture<IDirect3DTexture9>
    {
    public:
      texture (device&, std::unique_ptr<image>);
      ~texture () override;

      STDMETHOD (GetLevelDesc) (UINT, D3DSURFACE_DESC*) override;
      STDMETHOD (GetSurfaceLevel) (UINT, IDirect3DSurface9**) override;
      STDMETHOD (LockRect) (UINT, D3DLOCKED_RECT*, const RECT*, DWORD) override;
      STDMETHOD (UnlockRect) (UINT) override;
      STDMETHOD (AddDirtyRect) (const RECT*) override;

    private:
      std::vector<std::unique_ptr<surface>> levels_;
    };

    class cube_texture: public base_texture<IDirect3DCubeTexture9>
    {
    public:
      cube_texture (device&, std::unique_ptr<image>);
      ~cube_texture () override;

      STDMETHOD (GetLevelDesc) (UINT, D3DSURFACE_DESC*) override;
      STDMETHOD (GetCubeMapSurface) (D3DCUBEMAP_FACES, UINT, IDirect3DSurface9**) override;
      STDMETHOD (LockRect) (D3DCUBEMAP_FACES, UINT, D3DLOCKED_RECT*, const RECT*, DWORD) override;
      STDMETHOD (UnlockRect) (D3DCUBEMAP_FACES, UINT) override;
      STDMETHOD (AddDirtyRect) (D3DCUBEMAP_FACES, const RECT*) override;

    private:
      std::vector<std::unique_ptr<surface>> surfaces_; // Face-major.
    };

    class volume_texture: public base_texture<IDirect3DVolumeTexture9>
    {
    public:
      volume_texture (device&, std::unique_ptr<image>);
      ~volume_texture () override;

      STDMETHOD (GetLevelDesc) (UINT, D3DVOLUME_DESC*) override;
      STDMETHOD (GetVolumeLevel) (UINT, IDirect3DVolume9**) override;
      STDMETHOD (LockBox) (UINT, D3DLOCKED_BOX*, const D3DBOX*, DWORD) override;
      STDMETHOD (UnlockBox) (UINT) override;
      STDMETHOD (AddDirtyBox) (const D3DBOX*) override;

    private:
      std::vector<std::unique_ptr<volume>> levels_;
    };

    // A surface, either a level of a texture or standalone (a render
    // target, a depth stencil, the back buffer).
    //
    // A texture's surfaces share its reference count, as in D3D9: they are
    // destroyed with the texture and adding a reference to one adds it to
    // the texture. That's what the container is for. A standalone surface
    // owns its image and counts its own references.
    //
    class surface: public resource<IDirect3DSurface9>
    {
    public:
      // A texture's surface.
      //
      surface (device&, texture_object& container, IUnknown* com, UINT level, UINT layer);

      // A standalone surface.
      //
      surface (device&, std::unique_ptr<image>);

      STDMETHOD_ (ULONG, AddRef) () override;
      STDMETHOD_ (ULONG, Release) () override;

      STDMETHOD (GetContainer) (REFIID, void**) override;
      STDMETHOD (GetDesc) (D3DSURFACE_DESC*) override;
      STDMETHOD (LockRect) (D3DLOCKED_RECT*, const RECT*, DWORD) override;
      STDMETHOD (UnlockRect) () override;
      STDMETHOD (GetDC) (HDC*) override;
      STDMETHOD (ReleaseDC) (HDC) override;

      void private_add_ref () noexcept;
      void private_release () noexcept;

      image&
      storage () noexcept
      {
        return *image_;
      }

      UINT
      subresource () const noexcept
      {
        return image_->subresource (level_, layer_);
      }

      UINT width  () const noexcept {return image_->width (level_);}
      UINT height () const noexcept {return image_->height (level_);}

      UINT
      level () const noexcept
      {
        return level_;
      }

      ID3D11RenderTargetView*
      target_view (bool srgb)
      {
        return image_->target_view (subresource (), srgb);
      }

      ID3D11DepthStencilView*
      depth_view ()
      {
        return image_->depth_view (subresource ());
      }

    protected:
      bool
      implements (REFIID riid) const noexcept override;

    private:
      image*                 image_;
      std::unique_ptr<image> owned_;
      texture_object*        container_ = nullptr;
      IUnknown*              com_       = nullptr; // Container's interface.
      UINT                   level_     = 0;
      UINT                   layer_     = 0;
    };

    // A level of a volume texture. Note that IDirect3DVolume9 isn't an
    // IDirect3DResource9, so this is the one object that implements the
    // little it shares with one itself.
    //
    class volume: public unknown<IDirect3DVolume9>
    {
    public:
      volume (device&, texture_object& container, IUnknown* com, UINT level);

      STDMETHOD_ (ULONG, AddRef) () override;
      STDMETHOD_ (ULONG, Release) () override;

      STDMETHOD (GetDevice) (IDirect3DDevice9**) override;
      STDMETHOD (SetPrivateData) (REFGUID, const void*, DWORD, DWORD) override;
      STDMETHOD (GetPrivateData) (REFGUID, void*, DWORD*) override;
      STDMETHOD (FreePrivateData) (REFGUID) override;
      STDMETHOD (GetContainer) (REFIID, void**) override;
      STDMETHOD (GetDesc) (D3DVOLUME_DESC*) override;
      STDMETHOD (LockBox) (D3DLOCKED_BOX*, const D3DBOX*, DWORD) override;
      STDMETHOD (UnlockBox) () override;

    private:
      device&         device_;
      texture_object& container_;
      IUnknown*       com_;
      UINT            level_;
      private_data    data_;
    };

    // Return our surface behind the D3D9 interface. Every surface the game
    // has is one of ours, so this is just a cast.
    //
    inline surface*
    surface_of (IDirect3DSurface9* s) noexcept
    {
      return static_cast<surface*> (s);
    }
  }
}
