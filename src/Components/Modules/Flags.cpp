#include "Flags.hpp"

namespace Components
{
	std::vector<std::string> Flags::EnabledFlags;

	bool Flags::HasFlag(const std::string& flag)
	{
		ParseFlags();

		for (const auto& entry : EnabledFlags)
		{
			if (Utils::String::ToLower(entry) == Utils::String::ToLower(flag))
			{
				return true;
			}
		}

		return false;
	}

  void Flags::ParseFlags ()
  {
    static bool p (false);

    if (p)
      return;

    p = true;

		// Note that the engine's parser mishandles trailing quotes. So here we
		// strip them all globally from the raw OS buffers by shifting characters
		// in-place. We cannot just replace them with spaces because that would
		// naturally break space-separated values.
		//
    if (char* b = GetCommandLineA ())
    {
      char* r (b);
      char* w (b);

      while (*r != '\0')
      {
        if (*r != '\"')
          *w++ = *r;

        r++;
      }

      *w = '\0';
    }

    if (wchar_t * b = GetCommandLineW ())
    {
      wchar_t* r (b);
      wchar_t* w (b);

      while (*r != L'\0')
      {
        if (*r != L'\"')
          *w++ = *r;

        r++;
      }

      *w = L'\0';
    }

    int c (0);

		// Parse the arguments. Since we already removed the quotes from the OS
		// buffer above, CommandLineToArgvW() will parse the resulting clean
		// string. This is perfectly safe as we are only looking for the '-'
		// prefix anyway.
		//
    const auto a (CommandLineToArgvW (GetCommandLineW (), &c));

    if (a)
    {
      for (int i (0); i < c; ++i)
      {
        std::wstring f (a[i]);

        if (f[0] == L'-')
        {
          f.erase (f.begin ());
          EnabledFlags.emplace_back (Utils::String::Convert (f));
        }
      }

      LocalFree (a);
    }

		// Work around a Wine issue. If we are running dedicated and they did not
		// specify output channels, force stdout so we actually see the logs.
		//
		if (Utils::IsWineEnvironment () &&
      	Dedicated::IsEnabled ()     &&
				!HasFlag ("console")        &&
				!HasFlag ("stdout"))
    {
      EnabledFlags.emplace_back ("stdout");
    }
  }
}
