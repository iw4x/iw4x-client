#include "Attachment.hpp"

#include "Types.hpp"

namespace Sentry
{
  namespace
  {
    constexpr attachment table[] =
    {
      {"logs/console_mp.log", "text/plain"},
      {"zone-conversion.log", "text/plain"},
    };
  }

  std::span<const attachment>
  attachments () noexcept
  {
    return table;
  }

  void
  attach (const context& ctx, const std::filesystem::path& base)
  {
    for (const attachment& a: attachments ())
    {
      const std::filesystem::path p (base / a.relative_path);

      sentry_attachment_t* h (sentry_attach_filew (p.c_str ()));

      if (h == nullptr)
      {
        ctx.report (severity::warning, facility::attachment,
                    errc::attachment_missing,
                    std::format ("{} could not be attached", a.relative_path));
        continue;
      }

      sentry_attachment_set_content_type (h, a.content_type);
    }
  }
}
