#include "Console.hpp"

#include "Types.hpp"

namespace Sentry
{
  void console::
  append (std::string_view s)
  {
    std::lock_guard l (mutex_);

    text_ += s;

    if (text_.size () > limit_)
    {
      // Drop what is over the limit plus the rest of that line, so that we
      // don't start in the middle of one.
      //
      const size_t n (text_.size () - limit_);
      const size_t p (text_.find ('\n', n));

      text_.erase (0, p != std::string::npos ? p + 1 : n);
    }

    ++generation_;
  }

  std::pair<std::string, uint64_t> console::
  snapshot () const
  {
    std::lock_guard l (mutex_);
    return {text_, generation_.load ()};
  }

  std::optional<std::pair<std::string, uint64_t>> console::
  try_snapshot () const
  {
    std::unique_lock l (mutex_, std::try_to_lock);

    if (!l.owns_lock ())
      return std::nullopt;

    return std::pair<std::string, uint64_t> (text_, generation_.load ());
  }
}
