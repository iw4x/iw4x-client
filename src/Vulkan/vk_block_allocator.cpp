#include <Vulkan/vk_block_allocator.hpp>

#include <cassert>

using namespace std;

namespace Vulkan
{
  vk_block_allocator::
  vk_block_allocator (uint64_t size)
    : size_ (size)
  {
    assert (size != 0);
    free_.emplace (0, size);
  }

  optional<uint64_t> vk_block_allocator::
  allocate (uint64_t size, uint64_t alignment)
  {
    assert (size != 0);
    assert (alignment != 0 && (alignment & (alignment - 1)) == 0);

    // Use the first free range (lowest offset) that still has room for the
    // request once its start is aligned up.
    //
    for (auto i (free_.begin ()); i != free_.end (); ++i)
    {
      uint64_t b (i->first);
      uint64_t e (b + i->second);
      uint64_t o ((b + alignment - 1) & ~(alignment - 1));

      if (o >= e || e - o < size) // Not enough room after aligning.
        continue;

      // Return what is left before and after the allocation to the free
      // ranges.
      //
      free_.erase (i);

      if (o != b)
        free_.emplace (b, o - b);

      if (o + size != e)
        free_.emplace (o + size, e - (o + size));

      used_ += size;
      return o;
    }

    return nullopt;
  }

  void vk_block_allocator::
  free (uint64_t offset, uint64_t size)
  {
    assert (size != 0 && offset + size <= size_);
    assert (used_ >= size);

    used_ -= size;

    uint64_t b (offset);
    uint64_t e (offset + size);

    // Merge the range with its free neighbors. Otherwise adjacent free
    // ranges would stay split and a larger request could fail even though
    // there is enough contiguous space.
    //
    auto n (free_.lower_bound (b)); // First free range after this one.

    if (n != free_.begin ())
    {
      auto p (prev (n));
      assert (p->first + p->second <= b); // Overlap means double free.

      if (p->first + p->second == b)
      {
        b = p->first;
        free_.erase (p);
      }
    }

    if (n != free_.end ())
    {
      assert (n->first >= e); // As above.

      if (n->first == e)
      {
        e += n->second;
        free_.erase (n);
      }
    }

    free_.emplace (b, e - b);
  }
}
