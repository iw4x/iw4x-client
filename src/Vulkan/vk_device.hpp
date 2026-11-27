#pragma once

#include <mutex>
#include <deque>
#include <atomic>
#include <string>
#include <vector>
#include <memory>       // unique_ptr
#include <cstdint>
#include <functional>   // move_only_function
#include <string_view>

#include <Vulkan/vk.hpp>
#include <Vulkan/vk_window.hpp>
#include <Vulkan/vk_block_allocator.hpp>

namespace Vulkan
{
  // Range of device memory as returned by vk_device::allocate(). Converts
  // to false if empty (no memory).
  //
  struct vk_allocation
  {
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    VkDeviceSize size = 0;
    void* mapped = nullptr; // NULL unless the memory is host-visible.

    // Used by vk_device::free() to return the range. The block is ~0u if the
    // allocation has dedicated memory.
    //
    std::uint32_t type = 0;
    std::uint32_t block = 0;

    explicit
    operator bool () const noexcept {return memory != VK_NULL_HANDLE;}
  };

  // Mapped source buffer for uploads (see vk_device::stage()).
  //
  struct vk_staging
  {
    VkBuffer buffer = VK_NULL_HANDLE;
    vk_allocation memory;

    void*
    data () const noexcept {return memory.mapped;}

    VkDeviceSize
    size () const noexcept {return memory.size;}
  };

  // The Vulkan device that renders into the window.
  //
  // Besides owning the Vulkan objects, the device suballocates memory for
  // other renderer parts and tracks what the GPU has finished. Every
  // queue submission gets a serial and the device's timeline semaphore
  // reaches that serial once the submission completes. Frames are numbered
  // separately, starting from 1. A frame is complete once its submission
  // is.
  //
  // The member functions are thread-safe.
  //
  class vk_device
  {
  public:
    // The device logs its diagnostics (including the validation messages)
    // through this function, one line per call. The calls never overlap.
    //
    using log_function = std::move_only_function<void (std::string_view)>;

    struct options
    {
      // If true, then enable the validation layer if it is installed.
      //
      bool validation = false;

      log_function log;
    };

    // Throw vk_error if there is no usable device (see vk_select_device())
    // and on any other failure.
    //
    vk_device (const vk_window&, options);

    // Wait for the GPU to finish and run the pending retire() functions
    // before destroying the device.
    //
    ~vk_device ();

    vk_device (const vk_device&) = delete;
    vk_device& operator= (const vk_device&) = delete;

  public:
    VkInstance
    instance () const noexcept {return instance_;}

    VkPhysicalDevice
    physical () const noexcept {return physical_;}

    VkDevice
    handle () const noexcept {return device_;}

    VkQueue
    queue () const noexcept {return queue_;}

    std::uint32_t
    queue_family () const noexcept {return family_;}

    VkSurfaceKHR
    surface () const noexcept {return surface_;}

    const VkPhysicalDeviceProperties&
    properties () const noexcept {return properties_;}

    const VkPhysicalDeviceLimits&
    limits () const noexcept {return properties_.limits;}

    const std::string&
    name () const noexcept {return name_;}

    // Return the device-local memory size, summed over the heaps.
    //
    VkDeviceSize
    local_memory () const noexcept;

    // Log the line with the options' log function. Do nothing if there is
    // none.
    //
    void
    log (std::string_view) const;

    PFN_vkCmdPushDescriptorSetKHR push_descriptor_set = nullptr;

  public:
    // Memory allocation.
    //

    // Allocate memory that satisfies the requirements and has the required
    // properties. If a memory type that also has the preferred properties
    // runs out of memory, then fall back to a type that only has the
    // required ones. Throw vk_error on failure.
    //
    // Host-visible memory comes back mapped and stays mapped until it is
    // freed.
    //
    vk_allocation
    allocate (const VkMemoryRequirements&,
              VkMemoryPropertyFlags required,
              VkMemoryPropertyFlags preferred = 0);

    // Return the memory to the device. Note that an empty allocation is
    // allowed and ignored.
    //
    void
    free (const vk_allocation&) noexcept;

    // Create a buffer with memory from allocate() bound to it. Throw
    // vk_error on failure.
    //
    std::pair<VkBuffer, vk_allocation>
    create_buffer (VkDeviceSize size,
                   VkBufferUsageFlags,
                   VkMemoryPropertyFlags required,
                   VkMemoryPropertyFlags preferred = 0);

    // Destroy a buffer from create_buffer() together with its memory.
    //
    void
    destroy_buffer (VkBuffer, const vk_allocation&) noexcept;

  public:
    // Uploads.
    //
    // Upload commands go into a batch that submit_frame() submits before the
    // next frame. Use flush_uploads() if the data is needed sooner.
    //

    // Create a host-visible staging buffer for copies to the GPU. Throw
    // vk_error on failure.
    //
    vk_staging
    stage (VkDeviceSize size);

    using upload_function =
      std::move_only_function<void (VkCommandBuffer, const vk_staging&)>;

    // Call the function with the batch's command buffer to record the copy
    // from the staging buffer. The device destroys the staging buffer once
    // the batch completes.
    //
    void
    upload (vk_staging&&, upload_function);

    // As above but for commands that need no staging buffer.
    //
    void
    record (std::move_only_function<void (VkCommandBuffer)>);

    // Submit the batch now and wait for it to complete. Do nothing if the
    // batch is empty.
    //
    void
    flush_uploads ();

  public:
    // Submission.
    //

    // Defer the function until the GPU is done with the work submitted so
    // far and with the next frame. This is how objects that the GPU may
    // still use get destroyed.
    //
    void
    retire (std::move_only_function<void ()>);

    // Swapchain synchronization for submit_frame(). The submission waits on
    // the wait semaphore before writing the image and signals the signal
    // semaphore when done. Each handle may be null.
    //
    struct frame_sync
    {
      VkSemaphore wait = VK_NULL_HANDLE;
      VkSemaphore signal = VK_NULL_HANDLE;
    };

    // Submit the frame's commands after the upload batch, if any, and move
    // on to the next frame. Return the serial. Throw vk_error on failure.
    //
    std::uint64_t
    submit_frame (VkCommandBuffer, const frame_sync&);

    // Return the current frame number.
    //
    std::uint64_t
    frame () const noexcept {return frame_.load (std::memory_order_acquire);}

    // Return the highest frame number that the GPU is known to have
    // finished, 0 if none. Note that the value is only updated by
    // completed() and wait().
    //
    std::uint64_t
    completed_frame () const noexcept
    {
      return completed_frame_.load (std::memory_order_acquire);
    }

    // Return the highest completed serial. As a side effect, recycle what
    // the completed submissions no longer need and run their retire()
    // functions.
    //
    std::uint64_t
    completed ();

    // Block until the serial completes, then recycle as above. Throw
    // vk_error on failure.
    //
    void
    wait (std::uint64_t serial);

    // Wait for everything submitted so far.
    //
    void
    wait_idle ();

    // Call vkQueuePresentKHR() on the device's queue and return its result.
    //
    VkResult
    present (const VkPresentInfoKHR&);

  private:
    void
    create_instance (const vk_window&, bool validation);

    void
    select_physical ();

    void
    create_device ();

    VkCommandBuffer
    upload_batch ();

    std::uint64_t
    submit (const VkCommandBufferSubmitInfo*, std::uint32_t,
            const frame_sync*);

    void
    collect (std::uint64_t completed);

  private:
    VkInstance instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT messenger_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physical_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue queue_ = VK_NULL_HANDLE;
    std::uint32_t family_ = 0;

    VkPhysicalDeviceProperties properties_ {};
    VkPhysicalDeviceMemoryProperties memory_ {};
    std::string name_;

    mutable log_function log_;
    mutable std::mutex log_mutex_;

    struct block
    {
      VkDeviceMemory memory;
      void* mapped;
      vk_block_allocator allocator;
    };

    std::vector<std::vector<std::unique_ptr<block>>> blocks_;
    std::mutex memory_mutex_;

    std::vector<bool> full_;

    VkSemaphore timeline_ = VK_NULL_HANDLE;
    std::uint64_t next_serial_ = 1;
    std::mutex queue_mutex_;

    struct batch
    {
      VkCommandBuffer commands = VK_NULL_HANDLE;
      std::vector<vk_staging> staging;
      std::uint64_t serial = 0;
    };

    VkCommandPool upload_pool_ = VK_NULL_HANDLE;
    batch recording_;
    bool recording_open_ = false;
    std::deque<batch> pending_;
    std::vector<VkCommandBuffer> spare_;
    std::mutex upload_mutex_;

    struct retired
    {
      std::uint64_t serial;
      std::move_only_function<void ()> function;
    };

    std::vector<std::move_only_function<void ()>> retiring_;
    std::deque<retired> retired_;

    std::deque<std::pair<std::uint64_t, std::uint64_t>> frames_;
    std::atomic<std::uint64_t> frame_ {1};
    std::atomic<std::uint64_t> completed_frame_ {0};

    std::mutex retire_mutex_;
  };
}
