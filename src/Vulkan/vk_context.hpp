#pragma once

#include <span>
#include <array>
#include <vector>
#include <cstdint>
#include <optional>

#include <Vulkan/vk.hpp>
#include <Vulkan/vk_d3d9.hpp>
#include <Vulkan/vk_ring.hpp>
#include <Vulkan/vk_state.hpp>
#include <Vulkan/vk_query.hpp>
#include <Vulkan/vk_buffer.hpp>
#include <Vulkan/vk_device.hpp>
#include <Vulkan/vk_shader.hpp>
#include <Vulkan/vk_texture.hpp>
#include <Vulkan/vk_pipeline.hpp>
#include <Vulkan/vk_sampler_cache.hpp>
#include <Vulkan/vk_vertex_declaration.hpp>

namespace Vulkan
{
  // Direct3D 9 device on top of vk_device.
  //
  // The interface mostly mirrors IDirect3DDevice9 and, unless stated
  // otherwise, a function behaves like its Direct3D 9 counterpart. The
  // context holds a reference to every object that is bound to it (see
  // vk_object).
  //
  // Since a draw normally follows a number of state changes, most of which
  // affect the pipeline, the setters only record the state and mark what
  // changed (see dirty_flags). The Vulkan state is then brought up to date
  // once per draw (see prepare()).
  //
  // Commands are recorded into one of slot_count command buffers that we
  // rotate on every submit(). This way we can record the next frame while
  // the GPU is still executing the previous ones.
  //
  // Note that the context is not thread-safe. All the calls are expected to
  // come from the render thread.
  //
  class vk_context
  {
  public:
    // If depth_buffer is true, then the initial render state has the depth
    // test enabled, which is what Direct3D 9 does when the device is created
    // with an automatic depth stencil surface.
    //
    // Throw vk_error on failure.
    //
    vk_context (vk_device&, bool depth_buffer);

    ~vk_context ();

    vk_context (const vk_context&) = delete;
    vk_context& operator= (const vk_context&) = delete;

    vk_device&
    device () const noexcept {return device_;}

  public:
    // Render and sampler state.
    //
    // The sampler numbers are as in Direct3D 9 and so include the vertex
    // texture samplers (d3d9::vertex_texture_sampler0 and up). Note that we
    // silently ignore unknown states and samplers.
    //
    void
    set_render_state (d3d9::render_state_type, std::uint32_t);

    std::uint32_t
    render_state (d3d9::render_state_type s) const noexcept {return render_[s];}

    void
    set_sampler_state (std::uint32_t sampler,
                       d3d9::sampler_state_type,
                       std::uint32_t);

    std::uint32_t
    sampler_state (std::uint32_t sampler,
                   d3d9::sampler_state_type) const noexcept;

    void
    set_texture (std::uint32_t sampler, vk_texture*);

    vk_texture*
    texture (std::uint32_t sampler) const noexcept;

  public:
    // Shaders and vertex input.
    //
    void
    set_vertex_shader (vk_shader*);

    vk_shader*
    vertex_shader () const noexcept {return vertex_shader_;}

    void
    set_pixel_shader (vk_shader*);

    vk_shader*
    pixel_shader () const noexcept {return pixel_shader_;}

    // Set the registers in the [start, start + count) range. As in Direct3D
    // 9, float and integer constants take 16 bytes per register while
    // boolean constants take a 32-bit BOOL each. Anything beyond the set's
    // size is ignored.
    //
    void
    set_constants (vk_shader_stage,
                   vk_constant_set,
                   std::uint32_t start,
                   const void* data,
                   std::uint32_t count);

    void
    set_vertex_declaration (vk_vertex_declaration*);

    vk_vertex_declaration*
    vertex_declaration () const noexcept {return declaration_;}

    void
    set_stream_source (std::uint32_t stream,
                       vk_buffer*,
                       std::uint32_t offset,
                       std::uint32_t stride);

    // Use vertex data in memory for the stream. We copy the data to the ring
    // since the caller is free to change it once we return. An empty span
    // unsets the stream.
    //
    void
    set_stream_memory (std::uint32_t stream,
                       std::span<const unsigned char>,
                       std::uint32_t stride);

    void
    set_indices (vk_buffer*);

  public:
    // Render targets and viewport.
    //
    // Note that setting render target 0 makes the viewport and scissor
    // rectangle cover the new surface.
    //
    void
    set_render_target (std::uint32_t index, vk_surface*);

    vk_surface*
    render_target (std::uint32_t i) const noexcept {return targets_[i];}

    void
    set_depth_stencil (vk_surface*);

    vk_surface*
    depth_stencil () const noexcept {return depth_;}

    void
    set_viewport (const d3d9::viewport&);

    const d3d9::viewport&
    viewport () const noexcept {return viewport_;}

    void
    set_scissor (const d3d9::rect&);

    const d3d9::rect&
    scissor () const noexcept {return scissor_;}

  public:
    // Clearing and drawing.
    //
    // Note that a draw is silently skipped if the state is incomplete, for
    // example, if no vertex shader is set (see prepare()).
    //
    void
    clear (std::span<const d3d9::clear_rect>,
           std::uint32_t flags,
           d3d9::color,
           float z,
           std::uint32_t stencil);

    // As above but with the color in floats.
    //
    void
    clear (std::span<const d3d9::clear_rect>,
           std::uint32_t flags,
           const std::array<float, 4>& color,
           float z,
           std::uint32_t stencil);

    void
    draw (d3d9::primitive_type, std::uint32_t start, std::uint32_t primitives);

    void
    draw_indexed (d3d9::primitive_type,
                  std::int32_t base,
                  std::uint32_t start,
                  std::uint32_t primitives);

    // DrawPrimitiveUP(). Note that, as in Direct3D 9, this leaves stream 0
    // unset.
    //
    void
    draw_user (d3d9::primitive_type,
               std::uint32_t primitives,
               const void* vertices,
               std::uint32_t stride);

  public:
    // Copying (StretchRect(), UpdateSurface(), GetRenderTargetData()).
    //
    void
    stretch_rect (vk_surface& src, const d3d9::rect*,
                  vk_surface& dst, const d3d9::rect*,
                  d3d9::texture_filter_type);

    void
    update_surface (vk_surface& src, const d3d9::rect*,
                    vk_surface& dst, const d3d9::point*);

    // Note that this one waits for the GPU since the caller expects the data
    // to be there on return.
    //
    void
    read_surface (vk_surface& src, vk_surface& dst);

  public:
    // Queries.
    //
    void
    issue (vk_query&, std::uint32_t flags);

    // Return true and copy the result to data if the query is done, false
    // otherwise. The flush argument corresponds to D3DGETDATA_FLUSH.
    //
    bool
    query_data (vk_query&, void* data, std::uint32_t size, bool flush);

  public:
    // Command recording.
    //
    // Return the current command buffer for recording commands that are not
    // allowed within rendering, such as copies and layout transitions. This
    // ends the rendering instance, if any.
    //
    VkCommandBuffer
    commands ();

    // Submit the current frame and return its serial (see vk_device::wait()).
    // The swapchain semaphores, if any, are passed in the frame sync (see
    // vk_device::submit_frame()).
    //
    std::uint64_t
    submit (const vk_device::frame_sync& = {});

    // Transition the texture to the layout unless it is already in it.
    //
    void
    transition (vk_texture&, VkImageLayout);

    struct transient_memory
    {
      VkBuffer buffer;
      VkDeviceSize offset;
      void* data;
    };

    // Allocate memory in the ring. It stays valid until the current frame
    // completes.
    //
    transient_memory
    transient (VkDeviceSize size, VkDeviceSize alignment);

  private:
    // Constant registers together with the version, frame, and count of
    // their last copy to the ring (see copy_constants()).
    //
    struct constant_set
    {
      std::vector<std::array<std::uint32_t, 4>> registers;

      std::uint64_t version = 1;
      std::uint64_t copied = 0;
      std::uint64_t copied_frame = 0;
      std::uint32_t copied_count = 0;
      VkBuffer buffer = VK_NULL_HANDLE;
      VkDeviceSize offset = 0;
    };

    // Vertex stream. If the buffer is NULL and the memory is not, then the
    // stream uses set_stream_memory() data in the ring.
    //
    struct stream
    {
      vk_buffer* buffer = nullptr;
      std::uint32_t offset = 0;
      std::uint32_t stride = 0;

      VkBuffer memory = VK_NULL_HANDLE;
      VkDeviceSize memory_offset = 0;
    };

    // What has to be set before the next draw.
    //
    enum dirty_flags: std::uint32_t
    {
      dirty_pipeline    = 0x01,
      dirty_dynamic     = 0x02,
      dirty_viewport    = 0x04,
      dirty_scissor     = 0x08,
      dirty_streams     = 0x10,
      dirty_indices     = 0x20,
      dirty_all         = 0xff
    };

    struct slot
    {
      VkCommandPool pool = VK_NULL_HANDLE;
      VkCommandBuffer commands = VK_NULL_HANDLE;
      std::uint64_t serial = 0; // Last submission, 0 if none.
    };

    static const std::size_t slot_count = 3;

  private:
    // Return the index in our sampler arrays for the Direct3D 9 sampler
    // number, nullopt if there is no such sampler. The pixel samplers come
    // first, followed by the vertex texture samplers.
    //
    static std::optional<std::size_t>
    sampler_slot (std::uint32_t sampler) noexcept;

    VkCommandBuffer
    begin_commands ();

    void
    begin_rendering ();

    void
    end_rendering ();

    // Bring the Vulkan state up to date for a draw. Return false if the
    // state is incomplete, in which case the draw is skipped.
    //
    bool
    prepare (d3d9::primitive_type);

    void
    collect_textures (std::array<vk_texture*, 20>&);

    void
    bind_pipeline (d3d9::primitive_type);

    void
    set_dynamic_state ();

    void
    push_descriptors (const std::array<vk_texture*, 20>&);

    void
    bind_streams ();

    // Copy the set to the ring unless the last copy is still good and return
    // the descriptor for its first count registers.
    //
    VkDescriptorBufferInfo
    copy_constants (vk_shader_stage, vk_constant_set, std::uint32_t count);

    // Return render target 0's extent and, if there is none, the depth
    // stencil surface's.
    //
    VkExtent2D
    target_extent () const noexcept;

    void
    create_placeholders ();

    void
    create_ring (VkDeviceSize);

    void
    destroy () noexcept;

  private:
    vk_device& device_;

    std::array<slot, slot_count> slots_;
    std::size_t slot_ = 0;
    bool recording_ = false;
    bool rendering_ = false;

    vk_pipeline_cache pipelines_;
    vk_sampler_cache samplers_;

    // Direct3D 9 state.
    //
    vk_render_states render_;
    std::array<vk_sampler_states, 20> sampler_states_;
    std::array<vk_texture*, 20> textures_ {};

    vk_shader* vertex_shader_ = nullptr;
    vk_shader* pixel_shader_ = nullptr;
    vk_vertex_declaration* declaration_ = nullptr;

    std::array<stream, vk_vertex_streams> streams_ {};
    vk_buffer* indices_ = nullptr;

    constant_set constants_[2][3];

    std::array<vk_surface*, vk_render_targets> targets_ {};
    vk_surface* depth_ = nullptr;
    d3d9::viewport viewport_ {};
    d3d9::rect scissor_ {};

    // Vulkan state derived from the above, and what is bound in the current
    // command buffer.
    //
    std::uint32_t dirty_ = dirty_all;
    const vk_pipeline_cache::pipeline* pipeline_ = nullptr;
    vk_topology_class pipeline_class_ = vk_topology_class::triangle;
    std::uint32_t shadow_samplers_ = 0;
    VkPipeline bound_pipeline_ = VK_NULL_HANDLE;
    VkPrimitiveTopology topology_ = VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
    VkBuffer bound_indices_ = VK_NULL_HANDLE;

    std::vector<vk_query*> active_queries_;

    VkBuffer ring_buffer_ = VK_NULL_HANDLE;
    vk_allocation ring_memory_;
    vk_ring ring_;

    // Substitutes for missing vertex elements and textures (see the
    // constructor and collect_textures()).
    //
    VkBuffer default_buffer_ = VK_NULL_HANDLE;
    vk_allocation default_memory_;
    std::array<vk_texture*, 4> placeholders_ {};
  };
}
