#include <Vulkan/vk_device.hpp>

#include <format>
#include <cassert>
#include <cstring>   // strcmp()
#include <utility>   // move(), exchange()
#include <algorithm> // ranges::any_of(), max()

#include <Vulkan/vk_error.hpp>
#include <Vulkan/vk_device_select.hpp>

using namespace std;

namespace Vulkan
{
  namespace
  {
    // Device memory block size for suballocation. Larger allocations (over
    // half a block) get dedicated memory (see allocate()).
    //
    const VkDeviceSize block_size (64 * 1024 * 1024);

    const char validation_layer[] = "VK_LAYER_KHRONOS_validation";

    // Forward a validation message to the device's log. The user data is
    // the device.
    //
    VKAPI_ATTR VkBool32 VKAPI_CALL
    debug_callback (VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                    VkDebugUtilsMessageTypeFlagsEXT,
                    const VkDebugUtilsMessengerCallbackDataEXT* data,
                    void* user)
    {
      using severity_bits = VkDebugUtilsMessageSeverityFlagBitsEXT;

      const severity_bits error (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT);
      const severity_bits warning (
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT);

      const char* s (severity >= error   ? "error"   :
                     severity >= warning ? "warning" :
                                           "info");

      static_cast<const vk_device*> (user)->log (
        format ("vulkan {}: {}", s, data->pMessage));

      return VK_FALSE; // As the specification requires from applications.
    }

    // Return true if the extension list contains the extension.
    //
    bool
    has_extension (const vector<VkExtensionProperties>& es, const char* n)
    {
      return ranges::any_of (es,
                             [n] (const char* e) {return strcmp (e, n) == 0;},
                             &VkExtensionProperties::extensionName);
    }
  }

  vk_device::
  vk_device (const vk_window& w, options o)
    : log_ (move (o.log))
  {
    try
    {
      create_instance (w, o.validation);
      surface_ = w.create_surface (instance_);

      select_physical ();
      create_device ();
    }
    catch (...)
    {
      // The destructor doesn't run for a partially constructed object, so
      // destroy what we have created so far in the reverse order.
      //
      if (device_ != VK_NULL_HANDLE)
        vkDestroyDevice (device_, nullptr);

      if (surface_ != VK_NULL_HANDLE)
        vkDestroySurfaceKHR (instance_, surface_, nullptr);

      if (messenger_ != VK_NULL_HANDLE)
      {
        auto f (reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT> (
                  vkGetInstanceProcAddr (instance_,
                                         "vkDestroyDebugUtilsMessengerEXT")));
        f (instance_, messenger_, nullptr);
      }

      if (instance_ != VK_NULL_HANDLE)
        vkDestroyInstance (instance_, nullptr);

      throw;
    }
  }

  vk_device::
  ~vk_device ()
  {
    // Make sure the GPU is done with everything before we start destroying
    // objects that it may still use.
    //
    vkDeviceWaitIdle (device_);

    // Release the upload batches, including the one that may still be
    // recording. Their command buffers go away with the pool.
    //
    {
      scoped_lock l (upload_mutex_);

      if (recording_open_)
        vkEndCommandBuffer (recording_.commands);

      for (vk_staging& s: recording_.staging)
        destroy_buffer (s.buffer, s.memory);

      for (batch& b: pending_)
      {
        for (vk_staging& s: b.staging)
          destroy_buffer (s.buffer, s.memory);
      }
    }

    // Run the deferred destructions, both those that are not yet assigned
    // a submission and those that wait for one to complete. With the device
    // idle, all of them are safe to run now.
    //
    for (auto& f: retiring_)
      f ();

    for (retired& r: retired_)
      r.function ();

    vkDestroyCommandPool (device_, upload_pool_, nullptr);
    vkDestroySemaphore (device_, timeline_, nullptr);

    for (auto& bs: blocks_)
    {
      for (auto& b: bs)
        vkFreeMemory (device_, b->memory, nullptr);
    }

    vkDestroyDevice (device_, nullptr);
    vkDestroySurfaceKHR (instance_, surface_, nullptr);

    if (messenger_ != VK_NULL_HANDLE)
    {
      auto f (reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT> (
                vkGetInstanceProcAddr (instance_,
                                       "vkDestroyDebugUtilsMessengerEXT")));
      f (instance_, messenger_, nullptr);
    }

    vkDestroyInstance (instance_, nullptr);
  }

  void vk_device::
  log (string_view m) const
  {
    // Note that we may be called from several threads, the validation
    // callback included.
    //
    scoped_lock l (log_mutex_);

    if (log_)
      log_ (m);
  }

  VkDeviceSize vk_device::
  local_memory () const noexcept
  {
    VkDeviceSize r (0);
    for (uint32_t i (0); i != memory_.memoryHeapCount; ++i)
    {
      if (memory_.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        r += memory_.memoryHeaps[i].size;
    }
    return r;
  }

  void vk_device::
  create_instance (const vk_window& w, bool validation)
  {
    // We load the Vulkan loader at runtime (with volk) and don't link to it.
    //
    vk_check (volkInitialize (), "unable to load Vulkan loader");

    bool layer (false);
    bool debug (false);

    // Validation is a debugging aid, so if the layer is not installed, then
    // we log it and carry on. The same goes for the debug utils extension,
    // without which we get no messages.
    //
    if (validation)
    {
      uint32_t n (0);
      vkEnumerateInstanceLayerProperties (&n, nullptr);
      vector<VkLayerProperties> ls (n);
      vkEnumerateInstanceLayerProperties (&n, ls.data ());

      layer = ranges::any_of (
        ls,
        [] (const char* l) {return strcmp (l, validation_layer) == 0;},
        &VkLayerProperties::layerName);

      if (!layer)
        log ("vulkan: validation layer is not installed");

      n = 0;
      vkEnumerateInstanceExtensionProperties (nullptr, &n, nullptr);
      vector<VkExtensionProperties> es (n);
      vkEnumerateInstanceExtensionProperties (nullptr, &n, es.data ());

      debug = has_extension (es, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    VkApplicationInfo ai {};
    ai.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ai.pApplicationName = "IW4x";
    ai.applicationVersion = 1;
    ai.pEngineName = "IW4";
    ai.engineVersion = 1;
    ai.apiVersion = VK_API_VERSION_1_3; // See vk_missing_requirement().

    vector<const char*> es (w.instance_extensions ());
    if (debug)
      es.push_back (VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    const char* ls[] {validation_layer};

    VkInstanceCreateInfo ci {};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &ai;
    ci.enabledExtensionCount = static_cast<uint32_t> (es.size ());
    ci.ppEnabledExtensionNames = es.data ();
    ci.enabledLayerCount = layer ? 1 : 0;
    ci.ppEnabledLayerNames = ls;

    vk_check (vkCreateInstance (&ci, nullptr, &instance_),
              "unable to create Vulkan instance");

    // Load the instance functions only. We load the device functions from
    // the device once we have it (see create_device()), which skips the
    // loader's dispatch.
    //
    volkLoadInstanceOnly (instance_);

    if (debug)
    {
      VkDebugUtilsMessengerCreateInfoEXT mi {};
      mi.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      // Info and verbose messages feel too noisy for the log, so we only ask
      // for warnings and errors.
      //
      mi.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      mi.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      mi.pfnUserCallback = &debug_callback;
      mi.pUserData = this;

      auto f (reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT> (
                vkGetInstanceProcAddr (instance_,
                                       "vkCreateDebugUtilsMessengerEXT")));

      vk_check (f (instance_, &mi, nullptr, &messenger_),
                "unable to create Vulkan debug messenger");
    }
  }

  void vk_device::
  select_physical ()
  {
    uint32_t n (0);
    vk_check (vkEnumeratePhysicalDevices (instance_, &n, nullptr),
              "unable to enumerate Vulkan devices");
    vector<VkPhysicalDevice> ds (n);
    vk_check (vkEnumeratePhysicalDevices (instance_, &n, ds.data ()),
              "unable to enumerate Vulkan devices");

    // Describe every device for the selection (see vk_select_device()),
    // keeping the queue family that we would use with it on the side.
    //
    vector<vk_device_candidate> cs;
    vector<uint32_t> families;

    for (VkPhysicalDevice d: ds)
    {
      VkPhysicalDeviceProperties p;
      vkGetPhysicalDeviceProperties (d, &p);

      VkPhysicalDeviceVulkan13Features f13 {};
      f13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

      VkPhysicalDeviceVulkan12Features f12 {};
      f12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
      f12.pNext = &f13;

      // Chaining the 1.2 and 1.3 feature structures is only valid if the
      // device supports Vulkan 1.3. For an older device the features stay
      // false, but it fails the version requirement anyway.
      //
      VkPhysicalDeviceFeatures2 f {};
      f.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
      f.pNext = p.apiVersion >= VK_API_VERSION_1_3 ? &f12 : nullptr;
      vkGetPhysicalDeviceFeatures2 (d, &f);

      uint32_t en (0);
      vkEnumerateDeviceExtensionProperties (d, nullptr, &en, nullptr);
      vector<VkExtensionProperties> es (en);
      vkEnumerateDeviceExtensionProperties (d, nullptr, &en, es.data ());

      uint32_t qn (0);
      vkGetPhysicalDeviceQueueFamilyProperties (d, &qn, nullptr);
      vector<VkQueueFamilyProperties> qs (qn);
      vkGetPhysicalDeviceQueueFamilyProperties (d, &qn, qs.data ());

      // Find a queue family that can render and present to our surface. We
      // use a single queue for everything (see create_device()).
      //
      uint32_t family (~0u);
      for (uint32_t i (0); i != qn; ++i)
      {
        VkBool32 present (VK_FALSE);
        vkGetPhysicalDeviceSurfaceSupportKHR (d, i, surface_, &present);

        if ((qs[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present)
        {
          family = i;
          break;
        }
      }

      using kind = vk_device_candidate::kind;

      vk_device_candidate c;
      c.name = p.deviceName;
      switch (p.deviceType)
      {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
          c.type = kind::discrete;
          break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
          c.type = kind::integrated;
          break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
          c.type = kind::virtual_gpu;
          break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
          c.type = kind::cpu;
          break;
        default:
          c.type = kind::other;
          break;
      }
      c.api_version = p.apiVersion;
      c.dynamic_rendering = f13.dynamicRendering;
      c.synchronization2 = f13.synchronization2;
      c.timeline_semaphore = f12.timelineSemaphore;
      c.host_query_reset = f12.hostQueryReset;
      c.texture_compression_bc = f.features.textureCompressionBC;
      c.sampler_anisotropy = f.features.samplerAnisotropy;
      c.independent_blend = f.features.independentBlend;
      c.swapchain = has_extension (es, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
      c.push_descriptor =
        has_extension (es, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
      c.present_queue = family != ~0u;

      cs.push_back (move (c));
      families.push_back (family);
    }

    optional<size_t> s (vk_select_device (cs));

    // If no device is usable, then explain why the first one isn't.
    //
    // @@ With several devices the first one is not necessarily the most
    //    interesting. Maybe we should report every device?
    //
    if (!s)
    {
      if (cs.empty ())
        throw vk_error (VK_ERROR_INCOMPATIBLE_DRIVER, "no Vulkan devices");

      throw vk_error (VK_ERROR_INCOMPATIBLE_DRIVER,
                      "no suitable Vulkan device: {} lacks {}",
                      cs[0].name,
                      *vk_missing_requirement (cs[0]));
    }

    physical_ = ds[*s];
    family_ = families[*s];
    name_ = cs[*s].name;

    // Cache the selected device's properties. The memory types also size
    // the per-type allocator state (see allocate()).
    //
    vkGetPhysicalDeviceProperties (physical_, &properties_);
    vkGetPhysicalDeviceMemoryProperties (physical_, &memory_);
    blocks_.resize (memory_.memoryTypeCount);
    full_.resize (memory_.memoryTypeCount);

    log (format ("vulkan: rendering with {} (Vulkan {}.{}.{})",
                 name_,
                 VK_API_VERSION_MAJOR (properties_.apiVersion),
                 VK_API_VERSION_MINOR (properties_.apiVersion),
                 VK_API_VERSION_PATCH (properties_.apiVersion)));
  }

  void vk_device::
  create_device ()
  {
    // Enable the required features (see vk_missing_requirement()) and the
    // optional ones that the device supports. Requesting an unsupported
    // feature fails the device creation, so the optional ones mirror the
    // device.
    //
    VkPhysicalDeviceFeatures supported;
    vkGetPhysicalDeviceFeatures (physical_, &supported);

    VkPhysicalDeviceVulkan13Features f13 {};
    f13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    f13.dynamicRendering = VK_TRUE;
    f13.synchronization2 = VK_TRUE;

    VkPhysicalDeviceVulkan12Features f12 {};
    f12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    f12.pNext = &f13;
    f12.timelineSemaphore = VK_TRUE;
    f12.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 f {};
    f.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    f.pNext = &f12;
    f.features.textureCompressionBC = VK_TRUE;
    f.features.samplerAnisotropy = VK_TRUE;
    f.features.independentBlend = VK_TRUE;
    f.features.fillModeNonSolid = supported.fillModeNonSolid;
    f.features.depthBiasClamp = supported.depthBiasClamp;
    f.features.shaderClipDistance = supported.shaderClipDistance;

    // A single queue for rendering, uploads, and presentation.
    //
    const float priority (1.0f);

    VkDeviceQueueCreateInfo qi {};
    qi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qi.queueFamilyIndex = family_;
    qi.queueCount = 1;
    qi.pQueuePriorities = &priority;

    const char* es[] {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                      VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME};

    VkDeviceCreateInfo ci {};
    ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    ci.pNext = &f;
    ci.queueCreateInfoCount = 1;
    ci.pQueueCreateInfos = &qi;
    ci.enabledExtensionCount = 2;
    ci.ppEnabledExtensionNames = es;

    vk_check (vkCreateDevice (physical_, &ci, nullptr, &device_),
              "unable to create Vulkan device for {}", name_);

    volkLoadDevice (device_);

    vkGetDeviceQueue (device_, family_, 0, &queue_);

    push_descriptor_set = reinterpret_cast<PFN_vkCmdPushDescriptorSetKHR> (
      vkGetDeviceProcAddr (device_, "vkCmdPushDescriptorSetKHR"));

    // The timeline semaphore tracks the submissions' progress. Every
    // submission signals its serial (see submit()).
    //
    VkSemaphoreTypeCreateInfo ti {};
    ti.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    ti.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    ti.initialValue = 0;

    VkSemaphoreCreateInfo si {};
    si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    si.pNext = &ti;

    vk_check (vkCreateSemaphore (device_, &si, nullptr, &timeline_),
              "unable to create timeline semaphore");

    VkCommandPoolCreateInfo pi {};
    pi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // We reset and reuse the upload command buffers (see collect()).
    //
    pi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pi.queueFamilyIndex = family_;

    vk_check (vkCreateCommandPool (device_, &pi, nullptr, &upload_pool_),
              "unable to create upload command pool");
  }

  vk_allocation vk_device::
  allocate (const VkMemoryRequirements& r,
            VkMemoryPropertyFlags required,
            VkMemoryPropertyFlags preferred)
  {
    // We suballocate small resources from blocks of block_size bytes, which
    // we keep per memory type, and give large ones dedicated memory. This
    // keeps the number of device memory allocations low since drivers
    // limit it (maxMemoryAllocationCount), sometimes to just 4096.
    //
    // A resource may also prefer properties that it can do without (for
    // example, device-local memory for a host-visible buffer). In this case
    // we first try the type with all the properties and fall back to a type
    // with the required ones if that type is out of memory.
    //

    // Return the first memory type with the properties that the resource
    // can use, ~0u if there is none.
    //
    auto find = [this, &r] (VkMemoryPropertyFlags f) -> uint32_t
    {
      for (uint32_t i (0); i != memory_.memoryTypeCount; ++i)
      {
        if ((r.memoryTypeBits & (1u << i)) &&
            (memory_.memoryTypes[i].propertyFlags & f) == f)
          return i;
      }
      return ~0u;
    };

    auto allocate_type ([this, &r] (uint32_t type, bool fallback)
                        -> vk_allocation
    {
      bool host (memory_.memoryTypes[type].propertyFlags &
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

      auto allocate_memory ([this, type, host, fallback] (VkDeviceSize size,
                                                          void** mapped)
                            -> VkDeviceMemory
      {
        *mapped = nullptr;

        // With a fallback, running out of memory is not fatal. We return a
        // null handle and mark the type as full, so that we don't keep
        // hitting the driver with allocations that fail. See free() for
        // where the mark is cleared.
        //
        if (fallback && full_[type])
          return VK_NULL_HANDLE;

        VkMemoryAllocateInfo ai {};
        ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ai.allocationSize = size;
        ai.memoryTypeIndex = type;

        VkDeviceMemory m;
        if (VkResult e = vkAllocateMemory (device_, &ai, nullptr, &m); e < 0)
        {
          if (fallback &&
              (e == VK_ERROR_OUT_OF_DEVICE_MEMORY ||
               e == VK_ERROR_OUT_OF_HOST_MEMORY))
          {
            full_[type] = true;
            return VK_NULL_HANDLE;
          }

          throw vk_error (e,
                          "unable to allocate {} bytes of device memory",
                          size);
        }

        // Keep host-visible memory mapped for its whole lifetime. A block
        // is shared by many allocations, so mapping it per allocation is
        // not an option.
        //
        if (host)
        {
          if (VkResult e = vkMapMemory (device_,
                                        m,
                                        0,
                                        VK_WHOLE_SIZE,
                                        0,
                                        mapped);
              e < 0)
          {
            vkFreeMemory (device_, m, nullptr);
            throw vk_error (e, "unable to map device memory");
          }
        }

        return m;
      });

      // Note that a dedicated allocation has no block (~0u), which is how
      // free() tells them apart.
      //
      if (r.size > block_size / 2)
      {
        vk_allocation a;
        a.memory = allocate_memory (r.size, &a.mapped);

        if (a.memory == VK_NULL_HANDLE)
          return vk_allocation ();

        a.size = r.size;
        a.type = type;
        a.block = ~0u;
        return a;
      }

      // Buffers and images may end up next to each other in a block, so
      // we also have to respect the buffer-image granularity.
      //
      VkDeviceSize alignment (max (r.alignment,
                                   properties_.limits.bufferImageGranularity));

      vector<unique_ptr<block>>& bs (blocks_[type]);

      auto take ([&r, alignment, type] (block& b, uint32_t i)
                 -> vk_allocation
      {
        vk_allocation a;
        if (optional<uint64_t> o = b.allocator.allocate (r.size, alignment))
        {
          a.memory = b.memory;
          a.offset = *o;
          a.size = r.size;
          a.mapped = b.mapped != nullptr ? static_cast<char*> (b.mapped) + *o
                                         : nullptr;
          a.type = type;
          a.block = i;
        }
        return a;
      });

      // Try the existing blocks first and only then allocate a new one.
      //
      for (uint32_t i (0); i != bs.size (); ++i)
      {
        if (vk_allocation a = take (*bs[i], i))
          return a;
      }

      void* mapped;
      VkDeviceMemory m (allocate_memory (block_size, &mapped));

      if (m == VK_NULL_HANDLE)
        return vk_allocation ();

      bs.push_back (
        make_unique<block> (block {m,
                                   mapped,
                                   vk_block_allocator (block_size)}));

      // The block is new and the resource takes at most half of it, so this
      // can't fail.
      //
      vk_allocation a (
        take (*bs.back (), static_cast<uint32_t> (bs.size () - 1)));

      assert (a);
      return a;
    });

    // Return the first memory type that has what the resource requires but
    // lacks some of what it prefers, ~0u if there is none.
    //
    auto find_fallback ([this, &r, required, preferred] () -> uint32_t
    {
      for (uint32_t i (0); i != memory_.memoryTypeCount; ++i)
      {
        VkMemoryPropertyFlags f (memory_.memoryTypes[i].propertyFlags);

        if ((r.memoryTypeBits & (1u << i)) &&
            (f & required) == required &&
            (f & preferred) != preferred)
          return i;
      }
      return ~0u;
    });

    uint32_t type (find (required | preferred));
    uint32_t fallback (find_fallback ());

    scoped_lock l (memory_mutex_);

    if (type != ~0u)
    {
      if (vk_allocation a = allocate_type (type, fallback != ~0u))
        return a;
    }

    if (fallback == ~0u)
      throw vk_error (VK_ERROR_OUT_OF_DEVICE_MEMORY,
                      "no memory type for properties {:#x}", required);

    return allocate_type (fallback, false);
  }

  void vk_device::
  free (const vk_allocation& a) noexcept
  {
    if (!a)
      return;

    scoped_lock l (memory_mutex_);

    // Dedicated memory goes back to the driver, so the type may have room
    // again.
    //
    if (a.block == ~0u)
    {
      vkFreeMemory (device_, a.memory, nullptr);
      full_[a.type] = false;
      return;
    }

    blocks_[a.type][a.block]->allocator.free (a.offset, a.size);
  }

  pair<VkBuffer, vk_allocation> vk_device::
  create_buffer (VkDeviceSize size,
                 VkBufferUsageFlags usage,
                 VkMemoryPropertyFlags required,
                 VkMemoryPropertyFlags preferred)
  {
    VkBufferCreateInfo ci {};
    ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    ci.size = size;
    ci.usage = usage;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer b;
    vk_check (vkCreateBuffer (device_, &ci, nullptr, &b),
              "unable to create {} byte buffer", size);

    try
    {
      VkMemoryRequirements r;
      vkGetBufferMemoryRequirements (device_, b, &r);

      vk_allocation a (allocate (r, required, preferred));

      if (VkResult e = vkBindBufferMemory (device_, b, a.memory, a.offset);
          e < 0)
      {
        free (a);
        throw vk_error (e, "unable to bind buffer memory");
      }

      return {b, a};
    }
    catch (...)
    {
      vkDestroyBuffer (device_, b, nullptr); // Don't leak the buffer.
      throw;
    }
  }

  void vk_device::
  destroy_buffer (VkBuffer b, const vk_allocation& a) noexcept
  {
    vkDestroyBuffer (device_, b, nullptr);
    free (a);
  }

  vk_staging vk_device::
  stage (VkDeviceSize size)
  {
    auto [b, a] = create_buffer (size,
                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    return vk_staging {b, a};
  }

  VkCommandBuffer vk_device::
  upload_batch ()
  {
    // Keep recording into the open batch, if any. Otherwise start a new one,
    // reusing a command buffer from a completed batch if we have one (see
    // collect()).
    //
    if (recording_open_)
      return recording_.commands;

    VkCommandBuffer c;
    if (!spare_.empty ())
    {
      c = spare_.back ();
      spare_.pop_back ();
    }
    else
    {
      VkCommandBufferAllocateInfo ai {};
      ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      ai.commandPool = upload_pool_;
      ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      ai.commandBufferCount = 1;

      vk_check (vkAllocateCommandBuffers (device_, &ai, &c),
                "unable to allocate upload command buffer");
    }

    VkCommandBufferBeginInfo bi {};
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vk_check (vkBeginCommandBuffer (c, &bi),
              "unable to begin upload command buffer");

    recording_.commands = c;
    recording_open_ = true;
    return c;
  }

  void vk_device::
  upload (vk_staging&& s, upload_function f)
  {
    scoped_lock l (upload_mutex_);

    // The staging buffer has to stay alive until the batch completes, so we
    // keep it in the batch until then (see collect()).
    //
    f (upload_batch (), s);
    recording_.staging.push_back (move (s));
  }

  void vk_device::
  record (move_only_function<void (VkCommandBuffer)> f)
  {
    scoped_lock l (upload_mutex_);
    f (upload_batch ());
  }

  void vk_device::
  retire (move_only_function<void ()> f)
  {
    // The object may still be used by commands that are recorded but not
    // yet submitted. So we can only assign the function a serial once the
    // next frame is submitted (see submit_frame()).
    //
    scoped_lock l (retire_mutex_);
    retiring_.push_back (move (f));
  }

  uint64_t vk_device::
  submit (const VkCommandBufferSubmitInfo* cs, uint32_t n,
          const frame_sync* fs)
  {
    scoped_lock l (queue_mutex_);

    // Every submission signals the timeline semaphore with its serial. For a
    // frame we also wait for the swapchain image before writing to it and
    // signal the presentation's semaphore.
    //
    uint64_t serial (next_serial_);

    VkSemaphoreSubmitInfo signal[2] {};
    signal[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    signal[0].semaphore = timeline_;
    signal[0].value = serial;
    signal[0].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    VkSemaphoreSubmitInfo wait {};
    wait.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;

    VkSubmitInfo2 si {};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    si.commandBufferInfoCount = n;
    si.pCommandBufferInfos = cs;
    si.signalSemaphoreInfoCount = 1;
    si.pSignalSemaphoreInfos = signal;

    if (fs != nullptr && fs->wait != VK_NULL_HANDLE)
    {
      wait.semaphore = fs->wait;
      wait.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

      si.waitSemaphoreInfoCount = 1;
      si.pWaitSemaphoreInfos = &wait;
    }

    if (fs != nullptr && fs->signal != VK_NULL_HANDLE)
    {
      signal[1].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
      signal[1].semaphore = fs->signal;
      signal[1].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

      si.signalSemaphoreInfoCount = 2;
    }

    vk_check (vkQueueSubmit2 (queue_, 1, &si, VK_NULL_HANDLE),
              "unable to submit to the queue");

    ++next_serial_;
    return serial;
  }

  uint64_t vk_device::
  submit_frame (VkCommandBuffer frame, const frame_sync& fs)
  {
    uint64_t serial;

    {
      scoped_lock l (upload_mutex_);

      // Submit the upload batch, if any, together with the frame and ahead
      // of it.
      //
      VkCommandBufferSubmitInfo cs[2] {};
      uint32_t n (0);

      if (recording_open_)
      {
        vk_check (vkEndCommandBuffer (recording_.commands),
                  "unable to end upload command buffer");

        cs[n].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cs[n++].commandBuffer = recording_.commands;
      }

      cs[n].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
      cs[n++].commandBuffer = frame;

      serial = submit (cs, n, &fs);

      if (recording_open_)
      {
        recording_.serial = serial;
        pending_.push_back (exchange (recording_, batch {}));
        recording_open_ = false;
      }
    }

    // Now that we know the serial, assign it to the functions retired since
    // the last frame and to the frame itself. The frame counts as completed
    // once the serial does (see collect()).
    //
    {
      scoped_lock l (retire_mutex_);

      for (auto& f: retiring_)
        retired_.push_back (retired {serial, move (f)});

      retiring_.clear ();

      frames_.emplace_back (frame_.fetch_add (1, memory_order_acq_rel),
                            serial);
    }

    return serial;
  }

  void vk_device::
  flush_uploads ()
  {
    // Submit the upload batch alone and wait for it to complete. This
    // is for uploads that can't wait for the next frame.
    //
    uint64_t serial (0);

    {
      scoped_lock l (upload_mutex_);

      if (!recording_open_)
        return;

      vk_check (vkEndCommandBuffer (recording_.commands),
                "unable to end upload command buffer");

      VkCommandBufferSubmitInfo c {};
      c.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
      c.commandBuffer = recording_.commands;

      serial = submit (&c, 1, nullptr);

      recording_.serial = serial;
      pending_.push_back (exchange (recording_, batch {}));
      recording_open_ = false;
    }

    wait (serial);
  }

  uint64_t vk_device::
  completed ()
  {
    uint64_t c (0);
    vk_check (vkGetSemaphoreCounterValue (device_, timeline_, &c),
              "unable to query timeline semaphore");

    collect (c);
    return c;
  }

  void vk_device::
  collect (uint64_t c)
  {
    // Free the completed upload batches' staging buffers and keep their
    // command buffers for reuse.
    //
    {
      scoped_lock l (upload_mutex_);

      while (!pending_.empty () && pending_.front ().serial <= c)
      {
        batch& b (pending_.front ());

        for (vk_staging& s: b.staging)
          destroy_buffer (s.buffer, s.memory);

        vkResetCommandBuffer (b.commands, 0);
        spare_.push_back (b.commands);
        pending_.pop_front ();
      }
    }

    // Collect the retired functions whose serial has completed and advance
    // the completed frame. Note that we run the functions after releasing
    // the lock since they may retire further objects.
    //
    vector<move_only_function<void ()>> fs;
    {
      scoped_lock l (retire_mutex_);

      while (!retired_.empty () && retired_.front ().serial <= c)
      {
        fs.push_back (move (retired_.front ().function));
        retired_.pop_front ();
      }

      while (!frames_.empty () && frames_.front ().second <= c)
      {
        completed_frame_.store (frames_.front ().first, memory_order_release);
        frames_.pop_front ();
      }
    }

    for (auto& f: fs)
      f ();
  }

  void vk_device::
  wait (uint64_t serial)
  {
    VkSemaphoreWaitInfo wi {};
    wi.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    wi.semaphoreCount = 1;
    wi.pSemaphores = &timeline_;
    wi.pValues = &serial;

    vk_check (vkWaitSemaphores (device_, &wi, UINT64_MAX),
              "unable to wait for submission {}", serial);

    collect (serial);
  }

  void vk_device::
  wait_idle ()
  {
    // Wait for the last submission. Note that with nothing submitted the
    // serial is 0, which is the timeline semaphore's initial value.
    //
    uint64_t serial;
    {
      scoped_lock l (queue_mutex_);
      serial = next_serial_ - 1;
    }

    wait (serial);
  }

  VkResult vk_device::
  present (const VkPresentInfoKHR& pi)
  {
    // The submissions use the same queue (see submit()).
    //
    scoped_lock l (queue_mutex_);
    return vkQueuePresentKHR (queue_, &pi);
  }
}
