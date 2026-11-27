#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <unordered_map>

#include <Vulkan/vk.hpp>
#include <Vulkan/vk_state.hpp>
#include <Vulkan/vk_shader.hpp>
#include <Vulkan/vk_vertex_declaration.hpp>

namespace Vulkan
{
  class vk_device;

  // Color blend attachment state as stored in the pipeline key. Note that
  // the key hash treats it as a single 64-bit word, so it should stay at 8
  // bytes without padding.
  //
  struct vk_blend_key
  {
    std::uint8_t enable = 0;
    std::uint8_t src_color = 0;
    std::uint8_t dst_color = 0;
    std::uint8_t color_op = 0;
    std::uint8_t src_alpha = 0;
    std::uint8_t dst_alpha = 0;
    std::uint8_t alpha_op = 0;
    std::uint8_t mask = 0;

    vk_blend_key () = default;

    explicit
    vk_blend_key (const VkPipelineColorBlendAttachmentState&) noexcept;

    VkPipelineColorBlendAttachmentState
    state () const noexcept;

    friend bool
    operator== (const vk_blend_key&, const vk_blend_key&) = default;
  };

  const std::uint32_t vk_render_targets (4); // As in Direct3D 9.

  // Direct3D 9 lets IW4 change the shaders and the render states
  // independently and at any time. Vulkan, on the other hand, bakes most of
  // this state into a pipeline, which is expensive to create. So we create
  // a pipeline when IW4 first draws with a new combination and reuse it
  // for every later draw.
  //
  // The key is what such a combination consists of, that is, everything
  // that a pipeline depends on minus the state that we can set dynamically
  // (see vk_pipeline.cpp for that). The shaders and the declaration are
  // identified by their ids since a pointer can be reused by a new object
  // (see vk_object::id()).
  //
  // For the attachments, an undefined format means there is no attachment
  // and the bound color targets come first. If stencil is true, then the
  // depth format has a stencil aspect as well.
  //
  struct vk_pipeline_key
  {
    std::uint64_t vertex_shader = 0;
    std::uint64_t pixel_shader = 0;
    std::uint64_t declaration = 0;

    vk_alpha_test alpha;

    std::uint32_t shadow_samplers = 0;

    std::array<vk_blend_key, vk_render_targets> blend {};

    std::array<VkFormat, vk_render_targets> colors {};
    VkFormat depth = VK_FORMAT_UNDEFINED;
    bool stencil = false;
    std::uint32_t samples = 1;

    vk_topology_class topology = vk_topology_class::triangle;

    friend bool
    operator== (const vk_pipeline_key&, const vk_pipeline_key&) = default;
  };

  struct vk_pipeline_key_hash
  {
    std::size_t
    operator() (const vk_pipeline_key&) const noexcept;
  };

  // Pipeline cache keyed by vk_pipeline_key (see above for background).
  //
  // All the pipelines share the same layout: a single push descriptor set
  // with the constants and the samplers of both stages at fixed bindings.
  // So the caller can push the descriptors without regard for the current
  // pipeline.
  //
  // A pipeline lives until trim() finds that it has not been used since a
  // given frame. The cache is expected to be used by a single thread.
  //
  class vk_pipeline_cache
  {
  public:
    explicit
    vk_pipeline_cache (vk_device&);

    ~vk_pipeline_cache ();

    vk_pipeline_cache (const vk_pipeline_cache&) = delete;
    vk_pipeline_cache& operator= (const vk_pipeline_cache&) = delete;

    VkPipelineLayout
    layout () const noexcept {return layout_;}

    struct pipeline
    {
      VkPipeline handle = VK_NULL_HANDLE;
      std::uint32_t bindings = 0; // Vertex buffer bindings that it reads.
      std::uint64_t used = 0;     // Most recent frame that used it.
    };

    // Return the key's pipeline, creating it on first use, and note that it
    // was used in the frame. The shaders and the declaration are only
    // needed for the creation and must be the ones that the key identifies.
    // Throw vk_error on failure.
    //
    const pipeline&
    get (const vk_pipeline_key&,
         vk_shader& vertex_shader,
         vk_shader& pixel_shader,
         const vk_vertex_declaration&,
         std::uint64_t frame);

    // Destroy the pipelines that were not used in the frame and after it.
    // Since the GPU may still be using them, the destruction is deferred
    // (see vk_device::retire()).
    //
    void
    trim (std::uint64_t frame);

    std::size_t
    size () const noexcept {return pipelines_.size ();}

  private:
    pipeline
    create (const vk_pipeline_key&,
            vk_shader&,
            vk_shader&,
            const vk_vertex_declaration&);

  private:
    vk_device& device_;
    VkDescriptorSetLayout set_layout_ = VK_NULL_HANDLE;
    VkPipelineLayout layout_ = VK_NULL_HANDLE;
    VkPipelineCache cache_ = VK_NULL_HANDLE;

    std::unordered_map<vk_pipeline_key, pipeline, vk_pipeline_key_hash>
    pipelines_;
  };
}
