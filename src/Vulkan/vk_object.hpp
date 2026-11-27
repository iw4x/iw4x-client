#pragma once

#include <atomic>
#include <concepts> // derived_from
#include <cstdint>

namespace Vulkan
{
  class vk_device;

  // Base for the objects that IW4 sees as Direct3D 9 resources (the
  // textures and the shaders, for example).
  //
  // Like their COM counterparts, these objects are reference-counted with
  // the count starting at 1 for the creator. Releasing the last reference
  // doesn't delete the object right away since the GPU may still be using
  // it. The deletion is deferred until the GPU is done (see
  // vk_device::retire()).
  //
  // Each object also has an id that, unlike its address, is never reused.
  // See vk_object.cpp for why this is useful.
  //
  class vk_object
  {
  public:
    // Increment/decrement the reference count and return the new value.
    //
    std::uint32_t
    add_ref () noexcept;

    std::uint32_t
    release () noexcept;

    vk_device&
    device () const noexcept {return device_;}

    std::uint64_t
    id () const noexcept {return id_;}

  protected:
    explicit
    vk_object (vk_device&) noexcept;

    virtual
    ~vk_object ();

    vk_object (const vk_object&) = delete;
    vk_object& operator= (const vk_object&) = delete;

  private:
    vk_device& device_;
    std::uint64_t id_;
    std::atomic<std::uint32_t> refs_ {1};
  };

  template <typename T>
  concept vk_object_type = std::derived_from<T, vk_object>;

  // Owning reference to an object, similar to auto_fd. Note that the
  // constructor doesn't add a reference.
  //
  template <vk_object_type T>
  class vk_ref
  {
  public:
    vk_ref () noexcept = default;

    explicit
    vk_ref (T* p) noexcept: p_ (p) {}

    vk_ref (vk_ref&& x) noexcept: p_ (x.release ()) {}

    vk_ref&
    operator= (vk_ref&& x) noexcept
    {
      if (this != &x)
        reset (x.release ());
      return *this;
    }

    vk_ref (const vk_ref&) = delete;
    vk_ref& operator= (const vk_ref&) = delete;

    ~vk_ref () noexcept {reset ();}

    T*
    get () const noexcept {return p_;}

    T*
    operator-> () const noexcept {return p_;}

    T&
    operator* () const noexcept {return *p_;}

    explicit
    operator bool () const noexcept {return p_ != nullptr;}

    T*
    release () noexcept
    {
      T* r (p_);
      p_ = nullptr;
      return r;
    }

    void
    reset (T* p = nullptr) noexcept
    {
      if (p_ != nullptr)
        p_->release ();
      p_ = p;
    }

  private:
    T* p_ = nullptr;
  };

  // Release the reference and reset the pointer, if not already NULL.
  //
  template <vk_object_type T>
  inline void
  vk_release (T*& p) noexcept
  {
    if (p != nullptr)
    {
      p->release ();
      p = nullptr;
    }
  }
}
