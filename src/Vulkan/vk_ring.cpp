#include <Vulkan/vk_ring.hpp>

#include <cassert>

using namespace std;

namespace Vulkan
{
  vk_ring::
  vk_ring (uint64_t size)
    : size_ (size)
  {
    assert (size != 0);
  }

  optional<uint64_t> vk_ring::
  allocate (uint64_t size, uint64_t alignment, uint64_t serial)
  {
    assert (size != 0);
    assert (alignment != 0 && (alignment & (alignment - 1)) == 0);
    assert (spans_.empty () || spans_.back ().serial <= serial); // In order.

    auto align = [alignment] (uint64_t o)
    {
      return (o + alignment - 1) & ~(alignment - 1);
    };

    // The memory in use runs from the tail to the head, possibly wrapping
    // around the end. If it doesn't wrap, then the free memory is between the
    // head and the end plus between the start and the tail. If it does, then
    // the free memory is between the head and the tail.
    //
    // Note that with something in use, the head being equal to the tail
    // means that the ring is full. When nothing is in use, we start over
    // from the beginning, which makes the whole ring available in one piece.
    //
    optional<uint64_t> r;

    if (spans_.empty ())
    {
      head_ = tail_ = 0;

      if (size <= size_)
        r = 0;
    }
    else if (tail_ < head_)
    {
      uint64_t o (align (head_));

      // If it doesn't fit before the end, then wrap around. The memory
      // between the head and the end stays unused until the tail wraps as
      // well.
      //
      if (o <= size_ && size_ - o >= size)
        r = o;
      else if (size <= tail_)
        r = 0;
    }
    else
    {
      uint64_t o (align (head_));

      if (o <= tail_ && tail_ - o >= size)
        r = o;
    }

    if (!r)
      return nullopt;

    head_ = *r + size;

    // We only need to know where the memory of each serial ends since the
    // next serial's memory starts there. So allocations with the same
    // serial extend the last span.
    //
    if (!spans_.empty () && spans_.back ().serial == serial)
      spans_.back ().end = head_;
    else
      spans_.push_back (span {serial, head_});

    return r;
  }

  void vk_ring::
  reclaim (uint64_t completed)
  {
    // The spans are in the allocation order, so the tail moves up to the
    // last completed span's end.
    //
    while (!spans_.empty () && spans_.front ().serial <= completed)
    {
      tail_ = spans_.front ().end;
      spans_.pop_front ();
    }
  }
}
