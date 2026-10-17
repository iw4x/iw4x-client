#pragma once

#include "Types.hpp"

namespace Sentry
{
  // The most recent console output, kept for attaching to reports. We can't
  // rely on the game's own console log: it is only written when the logfile
  // dvar is set and may then well be left over from a previous run.
  //
  // The game prints from several threads, so this is thread-safe.
  //
  class console
  {
  public:
    // Keep at most limit bytes, dropping whole lines from the front.
    //
    explicit
    console (size_t limit) noexcept: limit_ (limit) {}

    console (const console&) = delete;
    console& operator= (const console&) = delete;

    void
    append (std::string_view);

    // Return the output along with its generation, which changes with every
    // append.
    //
    std::pair<std::string, uint64_t>
    snapshot () const;

    // As above but return nullopt rather than wait for a thread that is
    // appending. This is for when we are crashing and that thread may be the
    // one that crashed, with the lock still held.
    //
    std::optional<std::pair<std::string, uint64_t>>
    try_snapshot () const;

    uint64_t
    generation () const noexcept {return generation_.load ();}

  private:
    size_t limit_;

    mutable std::mutex mutex_;
    std::string text_;
    std::atomic<uint64_t> generation_ {0};
  };
}
