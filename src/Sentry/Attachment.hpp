#pragma once

#include "Types.hpp"

#include "Context.hpp"

namespace Sentry
{
  struct attachment
  {
    const char* relative_path;
    const char* content_type;
  };

  std::span<const attachment>
  attachments () noexcept;

  void
  attach (const context&, const std::filesystem::path& base);
}
