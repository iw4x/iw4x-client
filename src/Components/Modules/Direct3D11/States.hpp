#pragma once

#include "Object.hpp"
#include "Resources.hpp"

#include <array>
#include <concepts>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

namespace Components
{
  namespace d3d11
  {
    // The D3D9 render states, indexed by D3DRENDERSTATETYPE, and the
    // sampler states of one sampler, indexed by D3DSAMPLERSTATETYPE.
    //
    using render_states  = std::array<DWORD, 256>;
    using sampler_states = std::array<DWORD, 14>;

    // Fill the states with their D3D9 defaults. The depth test default
    // depends on whether the device has an automatic depth stencil.
    //
    void
    default_render_states (render_states&, bool depth);

    void
    default_sampler_states (sampler_states&);

    // Return true if the sampler filters linearly when magnifying and
    // minifying, which is what decides whether D3D9 hardware filters a
    // depth comparison (see the CompareDepth() function that dexilifier
    // emits).
    //
    bool
    linear_sampler (const sampler_states&) noexcept;

    // A cache of D3D11 state objects keyed by their description.
    //
    // D3D9 sets states one at a time while D3D11 groups them into
    // immutable objects. Building the D3D11 description from the D3D9
    // states is cheap, so at draw time we build it and look up the object
    // here, creating it the first time. The descriptions are plain
    // structures, so we key on their bytes (we always zero-initialize them,
    // padding included, so that equal states have equal bytes).
    //
    template <typename D>
    concept state_desc = std::is_trivially_copyable_v<D>;

    template <state_desc D, com_interface S>
    class state_cache
    {
    public:
      template <std::invocable<const D&, S**> F>
      S*
      find (const D& d, F&& create)
      {
        const std::string_view k (reinterpret_cast<const char*> (&d), sizeof (D));

        if (auto i (states_.find (k)); i != states_.end ())
          return i->second.Get ();

        ComPtr<S> s;
        if (FAILED (create (d, s.GetAddressOf ())))
          return nullptr;

        return states_.emplace (std::string (k), std::move (s)).first->second.Get ();
      }

    private:
      struct hash
      {
        using is_transparent = void;

        size_t
        operator() (std::string_view s) const noexcept
        {
          return std::hash<std::string_view> () (s);
        }
      };

      std::unordered_map<std::string, ComPtr<S>, hash, std::equal_to<>> states_;
    };

    // Build the D3D11 descriptions from the D3D9 states.
    //
    D3D11_BLEND_DESC
    blend_desc (const render_states&) noexcept;

    D3D11_DEPTH_STENCIL_DESC
    depth_stencil_desc (const render_states&) noexcept;

    // The depth bias is in units of the depth buffer's precision in D3D11
    // while D3D9 has it as a fraction of the depth range, so we need the
    // depth buffer's bits to convert.
    //
    D3D11_RASTERIZER_DESC
    rasterizer_desc (const render_states&, UINT depth_bits) noexcept;

    D3D11_SAMPLER_DESC
    sampler_desc (const sampler_states&) noexcept;
  }
}
