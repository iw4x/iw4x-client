#pragma once

#include <atomic>
#include <concepts>
#include <cstdint>

namespace Components
{
  namespace d3d11
  {
    template <typename T>
    concept com_interface = std::derived_from<T, IUnknown>;

    // The COM side of every object that we hand out in place of a D3D9 one.
    //
    // D3D9 counts references in two ways. The count that AddRef() and
    // Release() return is the one the game sees and manages. But the device
    // also keeps a bound resource alive (a texture set with SetTexture(),
    // say) without changing that count, so the game may release a texture
    // that is still bound and D3D9 only destroys it once it's unbound. We
    // mirror this with a separate private count that the device takes for
    // its bindings (see private_ref) and destroy the object only once both
    // drop to zero.
    //
    // Note also that every object gets an id that is never reused, unlike
    // its address. We key the caches that combine objects (input layouts,
    // say) on these ids so that a new object at the address of an old one
    // can't pick up its stale entries.
    //
    template <com_interface I>
    class unknown: public I
    {
    public:
      unknown () = default;

      unknown (const unknown&) = delete;
      unknown& operator= (const unknown&) = delete;

      virtual
      ~unknown () = default;

      STDMETHOD (QueryInterface) (REFIID riid, void** object) override
      {
        if (object == nullptr)
          return E_POINTER;

        if (riid == __uuidof (IUnknown) || implements (riid))
        {
          AddRef ();
          *object = static_cast<I*> (this);
          return S_OK;
        }

        *object = nullptr;
        return E_NOINTERFACE;
      }

      STDMETHOD_ (ULONG, AddRef) () override
      {
        return ++public_;
      }

      STDMETHOD_ (ULONG, Release) () override
      {
        const ULONG r (--public_);
        if (r == 0 && private_.load () == 0)
          delete this;
        return r;
      }

      // Take and drop a private reference. The device does this for
      // everything it binds.
      //
      void
      private_add_ref () noexcept
      {
        ++private_;
      }

      void
      private_release () noexcept
      {
        if (--private_ == 0 && public_.load () == 0)
          delete this;
      }

      uint64_t
      id () const noexcept
      {
        return id_;
      }

    protected:
      // Return true if the object implements the interface. By default
      // that's I itself but an object that implements a derived interface
      // (IDirect3DTexture9 is also an IDirect3DBaseTexture9 and an
      // IDirect3DResource9, say) overrides this to add the bases.
      //
      virtual bool
      implements (REFIID riid) const noexcept
      {
        return riid == __uuidof (I);
      }

    private:
      std::atomic<ULONG> public_ {1};
      std::atomic<ULONG> private_ {0};

      uint64_t id_ {next_id ()};

      static uint64_t
      next_id () noexcept
      {
        static std::atomic<uint64_t> n {1};
        return n++;
      }
    };

    // A private reference to one of our objects, for the device's
    // bindings. The type is our object type rather than the interface so
    // that the device can get at the implementation without casting.
    //
    template <typename T>
    concept private_object = requires (T& x)
    {
      x.private_add_ref ();
      x.private_release ();
    };

    template <private_object T>
    class private_ref
    {
    public:
      private_ref () = default;

      explicit
      private_ref (T* p) noexcept
        : p_ (p)
      {
        if (p_ != nullptr)
          p_->private_add_ref ();
      }

      private_ref (const private_ref& x) noexcept
        : private_ref (x.p_)
      {
      }

      private_ref (private_ref&& x) noexcept
        : p_ (x.p_)
      {
        x.p_ = nullptr;
      }

      private_ref&
      operator= (private_ref x) noexcept
      {
        std::swap (p_, x.p_);
        return *this;
      }

      ~private_ref ()
      {
        if (p_ != nullptr)
          p_->private_release ();
      }

      T*
      get () const noexcept
      {
        return p_;
      }

      T*
      operator-> () const noexcept
      {
        return p_;
      }

      explicit
      operator bool () const noexcept
      {
        return p_ != nullptr;
      }

    private:
      T* p_ = nullptr;
    };
  }
}
