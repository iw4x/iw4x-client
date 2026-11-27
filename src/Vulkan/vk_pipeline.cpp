#include <Vulkan/vk_pipeline.hpp>

#include <bit>      // bit_cast()
#include <vector>
#include <iterator> // size()

#include <Vulkan/vk_error.hpp>
#include <Vulkan/vk_device.hpp>
#include <Vulkan/vk_vertex_input.hpp>

using namespace std;

namespace Vulkan
{
  vk_blend_key::
  vk_blend_key (const VkPipelineColorBlendAttachmentState& s) noexcept
    : enable (static_cast<uint8_t> (s.blendEnable)),
      src_color (static_cast<uint8_t> (s.srcColorBlendFactor)),
      dst_color (static_cast<uint8_t> (s.dstColorBlendFactor)),
      color_op (static_cast<uint8_t> (s.colorBlendOp)),
      src_alpha (static_cast<uint8_t> (s.srcAlphaBlendFactor)),
      dst_alpha (static_cast<uint8_t> (s.dstAlphaBlendFactor)),
      alpha_op (static_cast<uint8_t> (s.alphaBlendOp)),
      mask (static_cast<uint8_t> (s.colorWriteMask))
  {
  }

  VkPipelineColorBlendAttachmentState vk_blend_key::
  state () const noexcept
  {
    VkPipelineColorBlendAttachmentState r {};
    r.blendEnable = enable;
    r.srcColorBlendFactor = static_cast<VkBlendFactor> (src_color);
    r.dstColorBlendFactor = static_cast<VkBlendFactor> (dst_color);
    r.colorBlendOp = static_cast<VkBlendOp> (color_op);
    r.srcAlphaBlendFactor = static_cast<VkBlendFactor> (src_alpha);
    r.dstAlphaBlendFactor = static_cast<VkBlendFactor> (dst_alpha);
    r.alphaBlendOp = static_cast<VkBlendOp> (alpha_op);
    r.colorWriteMask = mask;
    return r;
  }

  size_t vk_pipeline_key_hash::
  operator() (const vk_pipeline_key& k) const noexcept
  {
    // FNV-1a hash over the key's fields. We hash them one by one since the
    // key may have padding. Note that a blend key is 8 bytes without
    // padding, so it goes in as a single word.
    //
    uint64_t h (0xcbf29ce484222325);

    auto add = [&h] (uint64_t v)
    {
      for (uint32_t i (0); i != 8; ++i, v >>= 8)
      {
        h ^= v & 0xff;
        h *= 0x100000001b3;
      }
    };

    add (k.vertex_shader);
    add (k.pixel_shader);
    add (k.declaration);
    add (static_cast<uint64_t> (k.alpha.func));
    add (bit_cast<uint32_t> (k.alpha.ref));
    add (k.shadow_samplers);

    for (const vk_blend_key& b: k.blend)
      add (bit_cast<uint64_t> (b));

    for (VkFormat f: k.colors)
      add (static_cast<uint64_t> (f));

    add (static_cast<uint64_t> (k.depth));
    add (k.stencil ? 1 : 0);
    add (k.samples);
    add (static_cast<uint64_t> (k.topology));

    return static_cast<size_t> (h);
  }

  vk_pipeline_cache::
  vk_pipeline_cache (vk_device& d)
    : device_ (d)
  {
    VkDevice h (d.handle ());

    // All the pipelines share a single layout: one push descriptor set with
    // the constant buffers and the samplers of both stages at fixed bindings
    // (see vk_constant_binding() and vk_sampler_binding()). This way the
    // shaders don't affect the layout and the context can push the
    // descriptors for every draw without tracking descriptor sets.
    //
    try
    {
      {
        vector<VkDescriptorSetLayoutBinding> bs;

        auto add = [&bs] (uint32_t binding,
                          VkDescriptorType type,
                          VkShaderStageFlags stage)
        {
          VkDescriptorSetLayoutBinding b {};
          b.binding = binding;
          b.descriptorType = type;
          b.descriptorCount = 1;
          b.stageFlags = stage;
          bs.push_back (b);
        };

        const vk_constant_set sets[] = {vk_constant_set::floats,
                                        vk_constant_set::ints,
                                        vk_constant_set::bools};

        for (vk_constant_set s: sets)
        {
          add (vk_constant_binding (vk_shader_stage::vertex, s),
               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
               VK_SHADER_STAGE_VERTEX_BIT);

          add (vk_constant_binding (vk_shader_stage::pixel, s),
               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
               VK_SHADER_STAGE_FRAGMENT_BIT);
        }

        for (uint32_t i (0); i != vk_pixel_samplers; ++i)
          add (vk_sampler_binding (vk_shader_stage::pixel, i),
               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
               VK_SHADER_STAGE_FRAGMENT_BIT);

        for (uint32_t i (0); i != vk_vertex_samplers; ++i)
          add (vk_sampler_binding (vk_shader_stage::vertex, i),
               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
               VK_SHADER_STAGE_VERTEX_BIT);

        VkDescriptorSetLayoutCreateInfo ci {};
        ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
        ci.bindingCount = static_cast<uint32_t> (bs.size ());
        ci.pBindings = bs.data ();

        vk_check (vkCreateDescriptorSetLayout (h, &ci, nullptr, &set_layout_),
                  "unable to create descriptor set layout");
      }

      {
        VkPipelineLayoutCreateInfo ci {};
        ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        ci.setLayoutCount = 1;
        ci.pSetLayouts = &set_layout_;

        vk_check (vkCreatePipelineLayout (h, &ci, nullptr, &layout_),
                  "unable to create pipeline layout");
      }

      {
        VkPipelineCacheCreateInfo ci {};
        ci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

        vk_check (vkCreatePipelineCache (h, &ci, nullptr, &cache_),
                  "unable to create pipeline cache");
      }
    }
    catch (...)
    {
      // Destroying a null handle does nothing, so we don't need to know how
      // far we got.
      //
      vkDestroyPipelineLayout (h, layout_, nullptr);
      vkDestroyDescriptorSetLayout (h, set_layout_, nullptr);
      throw;
    }
  }

  vk_pipeline_cache::
  ~vk_pipeline_cache ()
  {
    VkDevice h (device_.handle ());

    for (const auto& p: pipelines_)
      vkDestroyPipeline (h, p.second.handle, nullptr);

    vkDestroyPipelineCache (h, cache_, nullptr);
    vkDestroyPipelineLayout (h, layout_, nullptr);
    vkDestroyDescriptorSetLayout (h, set_layout_, nullptr);
  }

  const vk_pipeline_cache::pipeline& vk_pipeline_cache::
  get (const vk_pipeline_key& k,
       vk_shader& vs,
       vk_shader& ps,
       const vk_vertex_declaration& d,
       uint64_t frame)
  {
    // The key identifies the shaders and the declaration (by id), so we only
    // need the objects themselves to create the pipeline on a miss.
    //
    auto i (pipelines_.find (k));

    if (i == pipelines_.end ())
      i = pipelines_.emplace (k, create (k, vs, ps, d)).first;

    i->second.used = frame;
    return i->second;
  }

  void vk_pipeline_cache::
  trim (uint64_t frame)
  {
    // Note that a pipeline that is no longer used in new frames may still be
    // used by the frames in flight, so we retire its destruction.
    //
    for (auto i (pipelines_.begin ()); i != pipelines_.end (); )
    {
      if (i->second.used < frame)
      {
        VkDevice h (device_.handle ());
        VkPipeline p (i->second.handle);

        device_.retire ([h, p] () {vkDestroyPipeline (h, p, nullptr);});
        i = pipelines_.erase (i);
      }
      else
        ++i;
    }
  }

  vk_pipeline_cache::pipeline vk_pipeline_cache::
  create (const vk_pipeline_key& k,
          vk_shader& vs,
          vk_shader& ps,
          const vk_vertex_declaration& d)
  {
    // The vertex shader module depends on how its outputs link to the pixel
    // shader inputs. The pixel shader module bakes in the alpha test and the
    // shadow samplers, which is why the key includes them.
    //
    VkPipelineShaderStageCreateInfo stages[2] {};
    {
      vk_varying_map vm (vk_link_varyings (vs.signatures ().output,
                                           ps.signatures ().input));

      stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
      stages[0].module = vs.module (vm);
      stages[0].pName = "main";

      stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      stages[1].module = ps.module (k.alpha, k.shadow_samplers);
      stages[1].pName = "main";
    }

    // Match the declaration's elements to the vertex shader inputs. The
    // strides come from SetStreamSource() and are dynamic, so here we only
    // say which bindings are used.
    //
    vk_vertex_input vi (
      vk_match_vertex_input (d.elements (),
                             vs.signatures ().input.elements ()));

    vector<VkVertexInputBindingDescription> bindings;
    for (uint32_t m (vi.bindings); m != 0; m &= m - 1)
    {
      VkVertexInputBindingDescription b {};
      b.binding = static_cast<uint32_t> (countr_zero (m));
      b.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
      bindings.push_back (b);
    }

    VkPipelineVertexInputStateCreateInfo vertex {};
    vertex.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex.vertexBindingDescriptionCount =
      static_cast<uint32_t> (bindings.size ());
    vertex.pVertexBindingDescriptions = bindings.data ();
    vertex.vertexAttributeDescriptionCount =
      static_cast<uint32_t> (vi.attributes.size ());
    vertex.pVertexAttributeDescriptions = vi.attributes.data ();

    VkPipelineInputAssemblyStateCreateInfo assembly {};
    assembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

    // The topology itself is dynamic and only its class has to match the
    // pipeline's, so any topology in the class will do here.
    //
    switch (k.topology)
    {
      case vk_topology_class::point:
        assembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        break;
      case vk_topology_class::line:
        assembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        break;
      case vk_topology_class::triangle:
        assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        break;
    }

    // The remaining state is mostly dynamic (see dynamics below), so we only
    // fill what the pipeline still needs to know.
    //
    VkPipelineViewportStateCreateInfo viewport {};
    viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport.viewportCount = 1;
    viewport.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo raster {};
    raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // @@ This ignores D3DRS_FILLMODE, so wireframe materials render solid.
    //    Probably rare enough, but maybe we should add it to the key?
    //
    raster.polygonMode = VK_POLYGON_MODE_FILL;
    raster.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisample {};
    multisample.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.rasterizationSamples =
      static_cast<VkSampleCountFlagBits> (k.samples);

    VkPipelineDepthStencilStateCreateInfo depth {};
    depth.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    // One blend state per bound render target. The bound targets come first
    // in the key's color formats, followed by undefined ones.
    //
    uint32_t targets (0);
    VkPipelineColorBlendAttachmentState blends[vk_render_targets];

    while (targets != vk_render_targets &&
           k.colors[targets] != VK_FORMAT_UNDEFINED)
    {
      blends[targets] = k.blend[targets].state ();
      ++targets;
    }

    VkPipelineColorBlendStateCreateInfo blend {};
    blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blend.attachmentCount = targets;
    blend.pAttachments = blends;

    // With dynamic rendering the pipeline only needs the attachment formats.
    // A depth format with stencil serves as the stencil format as well.
    //
    VkPipelineRenderingCreateInfo rendering {};
    rendering.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    rendering.colorAttachmentCount = targets;
    rendering.pColorAttachmentFormats = k.colors.data ();
    rendering.depthAttachmentFormat = k.depth;
    rendering.stencilAttachmentFormat =
      k.stencil ? k.depth : VK_FORMAT_UNDEFINED;

    // The state that core Vulkan 1.3 lets us set while recording. The
    // context sets it from the render states before each draw, and
    // everything else has to go into the key.
    //
    static const VkDynamicState dynamics[] =
    {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
      VK_DYNAMIC_STATE_DEPTH_BIAS,
      VK_DYNAMIC_STATE_BLEND_CONSTANTS,
      VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
      VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
      VK_DYNAMIC_STATE_STENCIL_REFERENCE,
      VK_DYNAMIC_STATE_CULL_MODE,
      VK_DYNAMIC_STATE_FRONT_FACE,
      VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
      VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE,
      VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
      VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
      VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,
      VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE,
      VK_DYNAMIC_STATE_STENCIL_OP,
      VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE
    };

    VkPipelineDynamicStateCreateInfo dynamic {};
    dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic.dynamicStateCount = static_cast<uint32_t> (std::size (dynamics));
    dynamic.pDynamicStates = dynamics;

    VkGraphicsPipelineCreateInfo ci {};
    ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    ci.pNext = &rendering;
    ci.stageCount = 2;
    ci.pStages = stages;
    ci.pVertexInputState = &vertex;
    ci.pInputAssemblyState = &assembly;
    ci.pViewportState = &viewport;
    ci.pRasterizationState = &raster;
    ci.pMultisampleState = &multisample;
    ci.pDepthStencilState = &depth;
    ci.pColorBlendState = &blend;
    ci.pDynamicState = &dynamic;
    ci.layout = layout_;

    // Keep the bindings so that the context binds the vertex buffers that the
    // pipeline reads (see vk_context.cpp).
    //
    pipeline r;
    r.bindings = vi.bindings;

    vk_check (vkCreateGraphicsPipelines (device_.handle (), cache_,
                                         1, &ci, nullptr, &r.handle),
              "unable to create graphics pipeline");
    return r;
  }
}
