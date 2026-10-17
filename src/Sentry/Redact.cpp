#include "Redact.hpp"

#include "Types.hpp"

namespace Sentry
{
  bool
  sensitive (std::string_view s)
  {
    std::string l (s);
    std::ranges::transform (l, l.begin (),
                            [] (unsigned char c) {return std::tolower (c);});

    return l.find ("password") != std::string::npos ||
           l.find ("rcon") != std::string::npos ||
           l.find ("socks") != std::string::npos;
  }

  std::string
  redact_lines (std::string s)
  {
    std::string r;
    r.reserve (s.size ());

    for (size_t b (0); b < s.size (); )
    {
      size_t e (s.find ('\n', b));
      e = e != std::string::npos ? e + 1 : s.size ();

      const std::string_view l (std::string_view (s).substr (b, e - b));

      if (!sensitive (l))
        r += l;
      else
      {
        r += "// line removed since it may hold a password";

        if (l.ends_with ("\r\n"))
          r += "\r\n";
        else if (l.ends_with ('\n'))
          r += '\n';
      }

      b = e;
    }

    return r;
  }

  std::string
  redact_arguments (const std::vector<std::string>& as)
  {
    std::string r;
    bool hiding (false);

    for (const std::string& a: as)
    {
      if (!a.empty () && (a.front () == '+' || a.front () == '-'))
        hiding = false;

      if (!r.empty ())
        r += ' ';

      r += hiding ? "<removed>" : a;

      if (sensitive (a))
        hiding = true;
    }

    return r;
  }
}
