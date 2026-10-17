#pragma once

#include "Types.hpp"

namespace Sentry
{
  // Close a Windows handle when it goes out of scope.
  //
  class auto_handle
  {
  public:
    explicit
    auto_handle (HANDLE h = nullptr) noexcept: h_ (h) {}

    auto_handle (auto_handle&& x) noexcept: h_ (x.release ()) {}

    auto_handle&
    operator= (auto_handle&& x) noexcept
    {
      reset (x.release ());
      return *this;
    }

    auto_handle (const auto_handle&) = delete;
    auto_handle& operator= (const auto_handle&) = delete;

    ~auto_handle () noexcept {reset ();}

    HANDLE
    get () const noexcept {return h_;}

    HANDLE
    release () noexcept
    {
      HANDLE r (h_);
      h_ = nullptr;
      return r;
    }

    void
    reset (HANDLE h = nullptr) noexcept
    {
      if (h_ != nullptr)
        CloseHandle (h_);

      h_ = h;
    }

  private:
    HANDLE h_;
  };

  // Remove a file when it goes out of scope, unless cancelled. Note that the
  // path stays readable after cancel().
  //
  // Movable-only type. Move-assignment cancels the rhs object.
  //
  struct auto_rmfile
  {
    std::filesystem::path path;
    bool active;

    explicit
    auto_rmfile (std::filesystem::path p) noexcept
      : path (std::move (p)), active (true) {}

    auto_rmfile (auto_rmfile&& x) noexcept
      : path (std::move (x.path)), active (x.active)
    {
      x.active = false;
    }

    auto_rmfile&
    operator= (auto_rmfile&& x) noexcept
    {
      if (this != &x)
      {
        path = std::move (x.path);
        active = x.active;
        x.active = false;
      }

      return *this;
    }

    auto_rmfile (const auto_rmfile&) = delete;
    auto_rmfile& operator= (const auto_rmfile&) = delete;

    ~auto_rmfile ()
    {
      if (active)
      {
        // Don't check for an error as not much we can do here.
        //
        std::error_code ec;
        std::filesystem::remove (path, ec);
      }
    }

    void
    cancel () noexcept {active = false;}
  };
}
