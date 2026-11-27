#include <Vulkan/vk_device_select.hpp>

using namespace std;

namespace Vulkan
{
  optional<string>
  vk_missing_requirement (const vk_device_candidate& c)
  {
    // This is VK_MAKE_API_VERSION (0, 1, 3, 0) spelled out since we don't
    // include the Vulkan headers here.
    //
    const uint32_t vulkan_1_3 ((1u << 22) | (3u << 12));

    struct requirement
    {
      bool met;
      const char* name;
    };

    // Note that we only report the first unmet requirement, which is why
    // the more basic ones (the API version, for example) come first. The
    // name completes the "<device> lacks ..." diagnostics, so it should read
    // as an object (see vk_device.cpp).
    //
    const requirement rs[]
    {
      {c.api_version >= vulkan_1_3, "Vulkan 1.3"},
      {c.dynamic_rendering,         "dynamic rendering"},
      {c.synchronization2,          "synchronization2"},
      {c.timeline_semaphore,        "timeline semaphores"},
      {c.host_query_reset,          "host query reset"},
      {c.texture_compression_bc,    "BC texture compression"},
      {c.sampler_anisotropy,        "anisotropic filtering"},
      {c.independent_blend,         "independent blending"},
      {c.swapchain,                 "VK_KHR_swapchain"},
      {c.push_descriptor,           "VK_KHR_push_descriptor"},
      {c.present_queue,             "a graphics queue that can present"}
    };

    for (const requirement& r: rs)
    {
      if (!r.met)
        return r.name;
    }

    return nullopt;
  }

  optional<size_t>
  vk_select_device (const vector<vk_device_candidate>& cs)
  {
    using kind = vk_device_candidate::kind;

    // Prefer discrete GPUs, then integrated ones. Everything else (virtual
    // GPUs, software implementations like lavapipe) ranks lowest but is
    // still usable if nothing better is available.
    //
    auto rank = [] (kind k)
    {
      switch (k)
      {
        case kind::discrete:   return 2;
        case kind::integrated: return 1;
        default:               return 0;
      }
    };

    // Pick the highest ranked candidate that meets all the requirements. On
    // a tie we keep the earlier candidate, so the enumeration order decides.
    //
    optional<size_t> r;

    for (size_t i (0); i != cs.size (); ++i)
    {
      if (vk_missing_requirement (cs[i]))
        continue;

      if (!r || rank (cs[i].type) > rank (cs[*r].type))
        r = i;
    }

    return r;
  }
}
