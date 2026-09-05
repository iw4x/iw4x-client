#pragma once

#include "Types.hpp"

namespace Sentry
{
  enum class placement : uint8_t
  {
    tag,
    context,
  };

  struct field
  {
    const char* key;
    placement where;
    const char* group;
    std::string (*read) ();
    bool trail;
  };

  std::span<const field>
  fields () noexcept;

  class scope
  {
  public:
    scope ();

    scope (const scope&) = delete;
    scope& operator= (const scope&) = delete;
    scope (scope&&) = delete;
    scope& operator= (scope&&) = delete;

    void
    publish ();

  private:
    std::vector<std::string> cached_;
    bool primed_ {false};
  };
}
