#include <zlib.h>

#include "ZoneConvert.hpp"

#include "../../Sentry/Report.hpp"

using namespace std;

namespace zone
{
  namespace fs = std::filesystem;

  // Diagnostics.
  //
  namespace
  {
    const char* const log_file ("zone-conversion.log");

    void
    log (const string& m)
    {
      // Note that OutputDebugStringA does not give us the usual line
      // handling of a stream, hence the separate newline.
      //
      OutputDebugStringA (m.c_str ());
      OutputDebugStringA ("\n");

      ofstream os (log_file, ios::app);

      if (os.is_open ())
        os << m << '\n';
    }

    template <typename A0, typename... A>
    void
    log (const string_view& f, A0&& a0, A&&... a)
    {
      log (vformat (f, make_format_args (a0, a...)));
    }

    [[noreturn]] void
    fail (const string& m)
    {
      log (m);

      MessageBoxA (nullptr, m.c_str (), "IW4x", MB_ICONERROR | MB_OK);

      ExitProcess (1);
    }

    // As above but wait for the thread before exiting, since what it does
    // (such as writing a report) shouldn't be cut short. Note that we show
    // the message first so that the player doesn't wait on it for nothing.
    //
    [[noreturn]] void
    fail (const string& m, jthread& t)
    {
      log (m);

      MessageBoxA (nullptr, m.c_str (), "IW4x", MB_ICONERROR | MB_OK);

      t.join ();

      ExitProcess (1);
    }

    template <typename A0, typename... A>
    [[noreturn]] void
    fail (const string_view& f, A0&& a0, A&&... a)
    {
      fail (vformat (f, make_format_args (a0, a...)));
    }
  }

  // Layout.
  //
  namespace
  {
    // The game keeps its fastfiles under zone. The x64 game reads the stock
    // ones in place, which leaves IW4x's own fastfiles. These are in the x86
    // layout under zone/iw4x/x86, where the x86 game reads them, and we put
    // the converted copy under zone/iw4x/stage.
    //
    // Why not zone/iw4x/x64? For now that's where our mm variant keeps its
    // fastfiles (and main/iw4x/x64 its loose files, which is why BASEGAME is
    // main/iw4x/stage), though once it moves elsewhere (zone/iw4x/mm, say),
    // we can probably take both over. Note also that we leave zone/iw4x/x86
    // and main/iw4x/x86 as they are, so that a player can go back to the x86
    // game (say, while we fix something on x64 that keeps them from moving
    // over).
    //
    // Note that on x86 we convert the other way around, writing x86 copies
    // of the stock fastfiles under zone/iw4x/x86.
    //
    const fs::path zone_root ("zone");
    const fs::path source_root ("zone/iw4x/x86");
    const fs::path converted_root ("zone/iw4x/stage");

    // The loose IW4x files are a little different. The game loads these from
    // fs_basegame, which we point at BASEGAME. The x86 game reads them from
    // main/iw4x/x86, and older installations still have them in
    // BASEGAME_LEGACY. None of them depends on the word size, as far as we
    // can tell, so we simply copy what the other two locations have.
    //
    const fs::path basegame_root (BASEGAME);
    const fs::path source_basegame_root ("main/iw4x/x86");
    const fs::path legacy_basegame_root (BASEGAME_LEGACY);
  }

  // The fastfiles.
  //
  namespace
  {
    // There are two fastfile layouts that we care about. x64 is the layout
    // the game reads and the one shipped by recent Steam installations. x86
    // is the layout of IW4x's own fastfiles, which is, of course, what we are
    // looking for here, since those are the files that have to go through
    // the converter.
    //
    enum class arch {x86, x64};

    // Before the actual deflate stream there is a small fastfile
    // header. All variants start with an eight byte magic followed by
    // the zone version and the rest of the preamble. The signed
    // variants then have two 0x2000 byte authentication blocks before
    // we finally get to the compressed zone.
    //
    // Keep the payload offset with the magic. There are only three
    // variants we understand and this saves having to rediscover later
    // whether a particular magic is one of the signed ones.
    //
    const size_t preamble_size (8 + 4 + 1 + 8);
    const size_t authed_size (0x2000);
    const int zone_version (276);
    const int alpha_version_first (316);
    const int alpha_version_last (360);

    struct magic_type
    {
      const char* id;      // Eight characters, compared without a terminator.
      size_t      payload; // Where the deflate stream starts.
    };

    const magic_type magics[] = {
      {"IWffu100", preamble_size},
      {"IWff0100", preamble_size + 2 * authed_size},
      {"ABff0100", preamble_size + 2 * authed_size} // OAT
    };

    // Now, we don't actually have to inflate the whole zone to tell the
    // two layouts apart. Near the beginning is the size header followed
    // by the asset lists and the latter contain pointers. This gives us
    // a fairly convenient word size tell.
    //
    // More precisely, an asset list starts with a 32-bit count followed
    // by a pointer. In the x64 layout the pointer has to be aligned to
    // eight bytes, leaving four bytes of padding after the count, and
    // is itself eight bytes wide. In the x86 layout the four byte
    // pointer follows the count directly. So if we inflate far enough
    // to get the first couple of lists, we should have everything
    // needed to distinguish the two.
    //
    // The size header happens to be 40 bytes in both cases. After that
    // we want 32 bytes, enough to look at two lists as x64 records.
    // Perhaps we could get away with less, but checking two of them
    // makes an accidental match much less likely and still leaves us
    // inflating next to nothing.
    //
    const size_t size_header_size (40);
    const size_t asset_lists_size (32);
    const size_t wanted_size (size_header_size + asset_lists_size);

    // Determine which of the two layouts a fastfile uses.
    //
    // Be conservative here. This is called on files we discover in the
    // installation and a file that merely looks vaguely like a fastfile
    // should not suddenly become a conversion candidate. So nullopt
    // means that we could not establish that this is one of the
    // fastfiles we understand. The caller can then decide whether that
    // matters in its particular walk.
    //
    optional<arch>
    probe (const fs::path& f)
    {
      ifstream is (f, ios::binary);

      if (!is.is_open ())
        return nullopt;

      // Start with the fixed part that all the supported variants have in
      // common. A short read means there isn't enough here to be one of
      // them, so bail out before trying to interpret anything in the
      // buffer.
      //
      uint8_t h[preamble_size] {};
      is.read (reinterpret_cast<char*> (h), sizeof (h));

      if (is.gcount () != static_cast<streamsize> (sizeof (h)))
        return nullopt;

      // The zone version is at the same place for all three magics. We
      // only know the layout used by this version, so accepting another
      // one and applying the pointer test below would mostly be
      // guesswork.
      //
      int v;
      memcpy (&v, h + 8, sizeof (v));

      // Pre-release zones (some of the IW4x DLC maps) have their own
      // versions and are always signed. We can't run the pointer test on
      // them since, from 319 on, their stream is encrypted. But they only
      // ever existed for the x86 game, so the header alone is enough to
      // tell that they need converting. The converter then reads them in
      // their old layouts.
      //
      if (v >= alpha_version_first && v <= alpha_version_last)
      {
        if (memcmp (h, "IWff0100", 8) != 0)
          return nullopt;

        return arch::x86;
      }

      if (v != zone_version)
        return nullopt;

      // Identify the wrapper and, as a side effect, obtain the place
      // where its deflate stream starts. In particular the signed
      // variants have their authentication data in between, so using
      // preamble_size for every file would make us feed that data to
      // zlib.
      //
      const magic_type* m (nullptr);

      for (const magic_type& c: magics)
      {
        if (memcmp (h, c.id, 8) == 0)
        {
          m = &c;
          break;
        }
      }

      if (m == nullptr)
        return nullopt;

      // Inflate just enough of the stream for the caller to inspect its
      // prefix.
      //
      // We still need some compressed input to get there, of course.
      // 64K is ample for the beginning of the zones we understand. If
      // it somehow isn't, then failing the probe seems preferable to
      // turning what should be a cheap test into a progressively larger
      // read of the fastfile.
      //
      // Note that reaching the end of the output buffer is perfectly
      // normal. In fact that is the usual case since we intentionally
      // ask zlib for only a small prefix. Reaching the end of the
      // deflate stream is fine as well. A short output is left for the
      // caller to deal with since the helper itself doesn't know how
      // much structure the caller needs.
      //
      auto inflate_head = [&is] (size_t o, size_t n) -> optional<string>
      {
        const size_t limit (0x10000);

        // Reset the stream state before repositioning it. There should
        // normally be nothing interesting set at this point, but
        // keeping the helper independent of the preceding reads is
        // cheap enough.
        //
        is.clear ();
        is.seekg (static_cast<streamoff> (o));

        if (!is)
          return nullopt;

        // Read at most limit bytes. A fastfile can of course have
        // considerably more compressed data after this, though none of
        // it is relevant to the probe.
        //
        string in (limit, '\0');
        is.read (in.data (), static_cast<streamsize> (in.size ()));
        in.resize (static_cast<size_t> (is.gcount ()));

        if (in.empty ())
          return nullopt;

        z_stream z {};

        if (inflateInit (&z) != Z_OK)
          return nullopt;

        string out (n, '\0');
        z.next_in = reinterpret_cast<Bytef*> (in.data ());
        z.avail_in = static_cast<uInt> (in.size ());
        z.next_out = reinterpret_cast<Bytef*> (out.data ());
        z.avail_out = static_cast<uInt> (out.size ());

        int r (inflate (&z, Z_NO_FLUSH));
        size_t got (out.size () - z.avail_out);
        inflateEnd (&z);

        // Z_OK normally means our output buffer filled before the
        // stream ended. Z_STREAM_END means the stream itself ended
        // first. Both give us useful output. Anything else means we
        // cannot safely make the layout guess.
        //
        if (r != Z_OK && r != Z_STREAM_END)
          return nullopt;

        out.resize (got);
        return out;
      };

      optional<string> p (inflate_head (m->payload, wanted_size));

      // We need the complete size header and both list records for the
      // test below. A valid deflate prefix that stops earlier is still
      // of no use to us, so treat it the same as any other failed
      // probe.
      //
      if (!p || p->size () < wanted_size)
        return nullopt;

      const uint8_t* l (reinterpret_cast<const uint8_t*> (p->data ()) +
                        size_header_size);

      // We only need the counts to make sense of the pointers below.
      // Read them separately since, at this point, we don't yet know
      // which asset-list layout we are looking at.
      //
      auto count = [] (const uint8_t* at)
      {
        uint32_t r;
        memcpy (&r, at, sizeof (r));
        return r;
      };

      // In the x64 layout the count is followed by four bytes of
      // padding before the pointer. So one fairly good indication that
      // we have such a layout is that these bytes are all zero.
      //
      // Of course, four zero bytes on their own don't tell us much. We
      // will use this together with what the pointer ought to contain
      // below.
      //
      auto blank = [] (const uint8_t* at, size_t n)
      {
        return ranges::all_of (views::counted (at, static_cast<ptrdiff_t> (n)),
                               [] (uint8_t b)
        {
          return b == 0;
        });
      };

      // The other useful property is the value of the pointer itself.
      // At this stage the zone hasn't been linked yet, so an empty list
      // has a null pointer and a non-empty one refers to the following
      // stream with -1 or -2.
      //
      // Now, if we read such a pointer as 64 bits, these values still
      // look like 0, -1, and -2 only if the pointer really is 64 bits
      // wide. For an x86 list the upper half will actually be the next
      // four bytes in the stream and should normally spoil the value.
      // This gives us a convenient way of telling the two layouts apart
      // without having to understand anything beyond these first asset
      // lists.
      //
      auto stream = [] (uint32_t n, const uint8_t* at)
      {
        uint64_t v;
        memcpy (&v, at, sizeof (v));

        const uint64_t follows (~uint64_t (0));

        return n == 0 ? v == 0 : v == follows || v == follows - 1;
      };

      // Finally, do this for two lists. One could perhaps look like an
      // x64 list by accident, especially with all the zeroes that tend
      // to occur here. Two consecutive lists having both the padding
      // and pointer values we expect is a considerably better
      // indication and costs us nothing extra since we already inflated
      // this much of the stream.
      //
      bool x64 (blank (l + 4, 4)  && stream (count (l), l + 8) &&
                blank (l + 20, 4) && stream (count (l + 16), l + 24));

      return x64 ? arch::x64 : arch::x86;
    }

    // Determine whether a fastfile is one of ours in a format the game no
    // longer loads.
    //
    // Older IW4x versions wrote their fastfiles with their own magic and
    // format version and installations can still have these lying around.
    // The game refuses to load anything older than the current version, so
    // such a file is of no use to anyone.
    //
    bool
    outdated (const fs::path& f)
    {
      ifstream is (f, ios::binary);

      uint32_t h[2] {};
      is.read (reinterpret_cast<char*> (h), sizeof (h));

      return is.gcount () == static_cast<streamsize> (sizeof (h)) &&
             h[0] == XFILE_HEADER_IW4X                            &&
             h[1] < XFILE_VERSION_IW4X;
    }
  }

  // The groups.
  //
  namespace
  {
    // There is one slightly awkward case in the zone layout. Steam puts
    // the Stimulus and Resurgence maps in zone/dlc, which is otherwise
    // one of the directories we use ourselves.
    //
    // We cannot simply move everything out of dlc, then. These files
    // belong to the game and, perhaps more importantly, Steam will put
    // them back whenever the user runs an integrity check. Keep the map
    // names here so the code that deals with our groups can leave these
    // particular files where they are.
    //
    const char* const official_dlc_maps[] = {
      "mp_complex", "mp_compact", "mp_storm",       "mp_overgrown", "mp_crash",
      "mp_abandon", "mp_vacant",  "mp_trailerpark", "mp_strike",    "mp_fuel2"
    };

    // Now a map is not represented by just one fastfile. Besides the
    // main zone there can be a loadscreen and localized variants, with
    // names such as localized_mp_crash or mp_crash_load. Comparing the
    // filenames directly would consequently make the list above rather
    // unpleasant.
    //
    // So reduce the filename back to the map name first. Strip the
    // extension, then the two decorations that the game adds around
    // that name. What remains is the name we can compare with
    // official_dlc_maps.
    //
    bool
    official_dlc (const string& f)
    {
      string m (fs::path (f).stem ().generic_string ());

      if (m.starts_with ("localized_"))
        m.erase (0, strlen ("localized_"));

      if (m.ends_with ("_load"))
        m.erase (m.size () - strlen ("_load"));

      return ranges::find (official_dlc_maps, m) !=
             ranges::end  (official_dlc_maps);
    }

    // With that special case out of the way we can describe the
    // directories that belong to us. Most are entirely ours. dlc is the
    // exception above: the directory is ours for the purpose of
    // arranging IW4x files, though some of the fastfiles in it are
    // Steam's.
    //
    // Perhaps the easiest way to represent this is to give each group
    // an optional test for such shared files. A null test means there
    // are none. Anything under zone that is not in this table is
    // treated as a language directory and left to the game.
    //
    struct group_type
    {
      const char* name;
      bool (*shared) (const string&); // NULL if none of it is shared.
    };

    const group_type own_groups[] = {
      {"zonebuilder", nullptr},
      {"patch",       nullptr},
      {"dlc",         &official_dlc}
    };

  }

  // The plan.
  //
  namespace
  {
    // We may discover halfway through figuring out what needs doing
    // that the installation is not one we are prepared to change. By
    // then we don't want to have already moved some files around and,
    // perhaps, removed others.
    //
    // So first work out everything that would have to happen and record
    // it here. Only once we have the complete plan do we start carrying
    // it out. A refusal before that point then leaves the installation
    // exactly as we found it.
    //
    // Note that on x86 we also rearrange files (moving some and dropping
    // others) to bring older layouts into shape. The x64 game has no older
    // layout to speak of, and since the x86 game keeps reading the
    // originals, we only ever copy or convert them here. Which is a relief,
    // really, since a copy we get wrong can always be made again.
    //
    enum class action
    {
      copy,    // Copy the file, leaving the original where it is.
      convert  // Convert the file and write the result to `to`.
    };

    // A step is one such operation, from one file to another.
    //
    struct step
    {
      zone::action act;
      fs::path from;
      fs::path to;
    };

    // And the plan is just these steps in the order in which we will
    // eventually perform them.
    //
    using plan = vector<step>;
  }

  // Planning.
  //
  namespace
  {
    // Whether t has to be brought up to date with f. That's the case if
    // there is nothing at t yet or if f has changed since we produced t.
    //
    // The latter matters since, unlike the stock fastfiles, IW4x's own files
    // change with every release. Note that a copy keeps the original's
    // modification time while a conversion is newer than its input, so
    // either way f is only newer once it has changed. Could we compare
    // contents instead? We could, but reading every fastfile on every start
    // seems a steep price for that.
    //
    bool
    stale (const fs::path& f, const fs::path& t)
    {
      error_code ec;
      fs::file_time_type tt (fs::last_write_time (t, ec));

      if (ec)
        return true;

      fs::file_time_type ft (fs::last_write_time (f, ec));
      return !ec && ft > tt;
    }

    // There are first the loose IW4x files. The launcher installs them into
    // source_basegame_root, and installations that go back far enough still
    // have them in legacy_basegame_root, which the x86 game folds into the
    // former. We leave both alone, since the x86 game keeps reading them,
    // and copy them into basegame_root.
    //
    // What if a file is in both places? That happens if the x86 game hasn't
    // got round to folding it yet. The copy in source_basegame_root is then
    // the one the launcher installed later, so that's the one we keep.
    //
    void
    plan_basegame (plan& p)
    {
      set<fs::path> seen;

      for (const fs::path& b: {source_basegame_root, legacy_basegame_root})
      {
        if (!fs::is_directory (b))
          continue;

        for (const fs::directory_entry& e: fs::recursive_directory_iterator (b))
        {
          if (!e.is_regular_file ())
            continue;

          fs::path r (e.path ().lexically_relative (b));

          if (!seen.insert (r).second)
            continue;

          fs::path t (basegame_root / r);

          if (stale (e.path (), t))
            p.push_back ({action::copy, e.path (), move (t)});
        }
      }
    }

    // Then IW4x's own fastfiles. The x86 game moves them under source_root
    // from the groups the launcher installs them into, so an installation it
    // hasn't started on yet still has them in the groups. So we look in both
    // places and convert each into the same group under converted_root.
    //
    // A fastfile can again be in both places, in which case the one in the
    // group is the copy installed later and so the one we convert.
    //
    // Note that Steam's map packs in dlc are in the x64 layout and the game
    // reads them where Steam put them. Their x86 copies under source_root
    // are, naturally, of no use to us.
    //
    void
    plan_conversion (plan& p)
    {
      for (const group_type& g: own_groups)
      {
        set<string> seen;

        for (const fs::path& d: {zone_root / g.name, source_root / g.name})
        {
          if (!fs::is_directory (d))
            continue;

          for (const fs::directory_entry& e: fs::directory_iterator (d))
          {
            if (!e.is_regular_file ())
              continue;

            string n (e.path ().filename ().generic_string ());

            if ((g.shared != nullptr && g.shared (n)) || !seen.insert (n).second)
              continue;

            // An old IW4x fastfile is something the game would refuse to load
            // whatever its layout, and anything that isn't in the x86 layout
            // is not something this conversion knows what to do with. Either
            // way, better to leave it alone than to guess.
            //
            if (outdated (e.path ()) || probe (e.path ()) != arch::x86)
              continue;

            fs::path t (converted_root / g.name / n);

            if (stale (e.path (), t))
              p.push_back ({action::convert, e.path (), move (t)});
          }
        }
      }
    }

    // Each phase is planned once the one before it has been carried out, so
    // that it sees the layout left behind. Note that on x86 we also recover
    // the stock layout from an older converter and take over IW4x's
    // fastfiles before converting, which the x64 game has no use for.
    //
    struct phase_type
    {
      const char* name;
      void (*make) (plan&);
    };

    const phase_type phases[] = {
      {"copying IW4x's own files into \"" BASEGAME "\"", &plan_basegame},
      {"converting IW4x's fastfiles",                     &plan_conversion}
    };
  }

  // Progress.
  //
  namespace
  {
    // We don't really want the conversion machinery to know whether
    // there is a window watching it. In fact, for some of the work
    // below there cannot be one yet. So keep the reporting interface
    // small enough that we can use the same conversion path in either
    // case.
    //
    // There is one slightly awkward case here. A converter can sit on
    // the same fastfile for a few minutes, during which neither the
    // running set nor the completed count changes. We still need to
    // give the presentation side a chance to move. And this is what
    // tick() is for.
    //
    class progress
    {
    public:
      virtual
      ~progress () = default;

      // Keep the names in start order. The first one will normally be
      // the one we show if there is not enough room to show them all.
      //
      virtual void
      running (const vector<string>&) = 0;

      // Pass the completed count rather than "one more". This way whoever
      // is watching does not have to keep another copy of the conversion
      // accounting.
      //
      virtual void
      finished (size_t) = 0;

      // A conversion may sit on the same fastfile for minutes. In that
      // case neither running() nor finished() gives the presentation
      // side anything to do. tick() is the way through which animation
      // and window messages continue to make progress in the meantime.
      //
      virtual void
      tick () = 0;
    };

    // Before the window appears there is nowhere for progress to go.
    // Perhaps the obvious alternative would be to sprinkle tests around
    // the conversion calls. A sink is simpler and, more importantly,
    // keeps this distinction out of the conversion code.
    //
    class silent_progress: public progress
    {
    public:
      void
      running (const vector<string>&) override {}

      void
      finished (size_t) override {}

      void
      tick () override {}
    };

    const char* const window_class ("IW4xZoneConvert");

    const int window_width  (480);
    const int window_height (170);
    const int margin        (20);
    const int bar_height    (16);

    const float sweep_period   (1.9f);
    const float sweep_extent   (0.32f);
    const float sweep_strength (0.5f);

    const float fill_speed (6.0f);

    // COLORREF has the channels packed in the somewhat unfortunate BGR
    // order. Perhaps the easiest way to avoid thinking about that
    // throughout paint() is to interpolate one byte at a time here and
    // put the result back together.
    //
    COLORREF
    blend (COLORREF f, COLORREF t, float a)
    {
      auto c = [f, t, a] (int s)
      {
        float x (static_cast<float> ((f >> s) & 0xFF));
        float y (static_cast<float> ((t >> s) & 0xFF));

        return static_cast<COLORREF> (lround (x + (y - x) * a)) & 0xFF;
      };

      return c (0) | (c (8) << 8) | (c (16) << 16);
    }

    // Now, with several converters there isn't really a single useful
    // name for what is happening. Showing every name would presumably
    // be worse since the line would constantly change width. So use the
    // first one and mention how many are behind it.
    //
    // The empty case is the little gap after the converters have gone
    // away but before the surrounding work is done.
    //
    string
    describe (const vector<string>& ns)
    {
      if (ns.empty ())
        return "Finishing up";

      if (ns.size () == 1)
        return format ("Converting {}", ns.front ());

      return format ("Converting {} (+{} more)", ns.front (), ns.size () - 1);
    }

    // It is tempting to use the system progress control here. The
    // problem is that we need two different pieces of information
    // from the same bar. Its filled part tells us how much work is
    // complete. At the same time we need some motion when one
    // fastfile leaves that fill unchanged for a long time.
    //
    // The system control can give us a determinate bar or a marquee,
    // but not quite this combination. So draw the small thing
    // ourselves and run a shimmer through the part that is already
    // filled.
    //
    // If creating the window fails, carry on without it. There is
    // nothing in this UI that controls the conversion, so losing the
    // UI should not change what the conversion does.
    //
    // @@: But maybe we should error out?
    //
    class window_progress: public progress
    {
    public:
      window_progress (const string& title, size_t total);

      ~window_progress () override;

      window_progress (const window_progress&) = delete;
      window_progress& operator= (const window_progress&) = delete;

      void
      running (const vector<string>&) override;

      void
      finished (size_t) override;

      void
      tick () override;

    private:
      static LRESULT CALLBACK
      proc (HWND, UINT, WPARAM, LPARAM);

      void
      paint (HDC);

      void
      pump () const;

      HWND w_ = nullptr;
      HFONT font_ = nullptr;

      vector<string> running_;
      size_t done_ = 0;
      size_t total_ = 0;

      // Do not advance either animation by "one frame". There isn't
      // really such a thing here since tick() is called by the
      // conversion and may arrive quite irregularly. Instead remember
      // when the last one happened and advance from elapsed time.
      //
      uint64_t frame_ = 0;
      float sweep_ = 0.0f;
      float fill_ = 0.0f;
    };

    window_progress::
    window_progress (const string& t, size_t n)
        : total_ (n)
    {
      static const bool registered = []
      {
        WNDCLASSEXA c {};
        c.cbSize = sizeof (c);
        c.lpfnWndProc = &window_progress::proc;
        c.hInstance = GetModuleHandleA (nullptr);
        c.hCursor = LoadCursorA (nullptr, IDC_ARROW);
        c.lpszClassName = window_class;

        return RegisterClassExA (&c) != 0;
      } ();

      if (!registered)
        return;

      int x ((GetSystemMetrics (SM_CXSCREEN) - window_width) / 2);
      int y ((GetSystemMetrics (SM_CYSCREEN) - window_height) / 2);

      w_ = CreateWindowExA (WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
                            window_class,
                            t.c_str (),
                            WS_POPUP | WS_CAPTION | WS_VISIBLE,
                            x, y, window_width, window_height,
                            nullptr, nullptr, GetModuleHandleA (nullptr), this);

      if (w_ == nullptr)
        return;

      NONCLIENTMETRICSA m {};
      m.cbSize = sizeof (m);

      if (SystemParametersInfoA (SPI_GETNONCLIENTMETRICS, sizeof (m), &m, 0))
        font_ = CreateFontIndirectA (&m.lfMessageFont);

      frame_ = GetTickCount64 ();

      SetForegroundWindow (w_);
      tick ();
    }

    window_progress::
    ~window_progress ()
    {
      if (w_ != nullptr)
      {
        DestroyWindow (w_);
        w_ = nullptr;

        // DestroyWindow() is synchronous as far as destruction itself
        // is concerned, though it may leave ordinary messages queued
        // for this thread. Give them the same treatment as during
        // conversion.
        //
        pump ();
      }

      if (font_ != nullptr)
      {
        DeleteObject (font_);
        font_ = nullptr;
      }
    }

    void window_progress::
    running (const vector<string>& ns)
    {
      running_ = ns;
      tick ();
    }

    void window_progress::
    finished (size_t n)
    {
      done_ = n;
      tick ();
    }

    void window_progress::
    tick ()
    {
      if (w_ == nullptr)
        return;

      uint64_t now (GetTickCount64 ());
      float d (static_cast<float> (now - frame_) / 1000.0f);

      frame_ = now;
      sweep_ = fmod (sweep_ + d / sweep_period, 1.0f);

      float t (total_ != 0
               ? static_cast<float> (done_) / static_cast<float> (total_)
               : 0.0f);

      fill_ += (t - fill_) * std::min (1.0f, d * fill_speed);

      InvalidateRect (w_, nullptr, FALSE);
      UpdateWindow (w_);

      pump ();
    }

    LRESULT CALLBACK window_progress::
    proc (HWND w, UINT m, WPARAM wp, LPARAM lp)
    {
      if (m == WM_NCCREATE)
      {
        auto* c (reinterpret_cast<CREATESTRUCTA*> (lp));
        SetWindowLongPtrA (w, GWLP_USERDATA,
                           reinterpret_cast<LONG_PTR> (c->lpCreateParams));
      }

      auto* p (reinterpret_cast<window_progress*> (
                 GetWindowLongPtrA (w, GWLP_USERDATA)));

      switch (m)
      {
        case WM_PAINT:
        {
          if (p == nullptr)
            break;

          PAINTSTRUCT ps {};
          p->paint (BeginPaint (w, &ps));
          EndPaint (w, &ps);

          return 0;
        }

        case WM_ERASEBKGND: return 1;
        case WM_CLOSE:          return 0;
      }

      return DefWindowProcA (w, m, wp, lp);
    }

    void window_progress::
    paint (HDC dc)
    {
      RECT cl {};
      GetClientRect (w_, &cl);

      LONG w (cl.right - cl.left);
      LONG h (cl.bottom - cl.top);

      // Draw into a bitmap and blit it in one go. The bar is painted column by
      // column, which straight onto the window would show as tearing.
      //
      HDC       b  (CreateCompatibleDC (dc));
      HBITMAP bm (CreateCompatibleBitmap (dc, w, h));
      HGDIOBJ ob (SelectObject (b, bm));

      COLORREF back (GetSysColor (COLOR_BTNFACE));
      COLORREF accent (GetSysColor (COLOR_HIGHLIGHT));
      COLORREF track (blend (back, GetSysColor (COLOR_BTNSHADOW), 0.45f));

      auto fill = [b] (const RECT& r, COLORREF c)
      {
        HBRUSH br (CreateSolidBrush (c));
        FillRect (b, &r, br);
        DeleteObject (br);
      };

      fill (cl, back);

      SetBkMode (b, TRANSPARENT);
      SetTextColor (b, GetSysColor (COLOR_BTNTEXT));

      HGDIOBJ of (font_ != nullptr ? SelectObject (b, font_) : nullptr);

      auto write = [b, w] (const string& s, LONG top, UINT fm)
      {
        RECT r {margin, top, w - margin, top + 20};
        DrawTextA (b, s.c_str (), -1, &r, fm | DT_SINGLELINE | DT_NOPREFIX);
      };

      write (describe (running_), margin, DT_LEFT | DT_PATH_ELLIPSIS);

      RECT bar {margin, margin + 34, w - margin, margin + 34 + bar_height};

      write (format ("{} of {} fastfiles", done_, total_),
             bar.bottom + 14,
             DT_LEFT);

      fill (bar, track);

      float span (static_cast<float> (bar.right - bar.left));
      float filled (span * std::clamp (fill_, 0.0f, 1.0f));

      float sw (span * sweep_extent);
      float at (-sw + sweep_ * (span + 2.0f * sw));

      for (float c (0.0f); c < filled; ++c)
      {
        float d (std::abs (c - at) / sw);
        float s (d < 1.0f ? (1.0f - d) * (1.0f - d) * sweep_strength : 0.0f);

        LONG l (bar.left + static_cast<LONG> (c));
        RECT r {l, bar.top, l + 1, bar.bottom};

        fill (r, blend (accent, RGB (255, 255, 255), s));
      }

      if (of != nullptr)
        SelectObject (b, of);

      BitBlt (dc, 0, 0, w, h, b, 0, 0, SRCCOPY);

      SelectObject (b, ob);
      DeleteObject (bm);
      DeleteDC (b);
    }

    void window_progress::
    pump () const
    {
      MSG m;

      while (PeekMessageA (&m, nullptr, 0, 0, PM_REMOVE))
      {
        TranslateMessage (&m);
        DispatchMessageA (&m);
      }
    }
  }

  // Execution.
  //
  namespace
  {
    void
    mkdir_p (const fs::path& d)
    {
      // There isn't much to do here. Some prefix of d will presumably
      // already exist and create_directories() is happy with that. So
      // just ask for the whole thing and see what came of it.
      //
      error_code ec;
      fs::create_directories (d, ec);

      if (ec)
        fail ("\"{}\" could not be created: {}.\n\n"
              "The game has nowhere to put the files it reads, so it cannot start.",
              d.generic_string (), ec.message ());
    }

    void
    do_copy (const step& s)
    {
      // The plan doesn't contain mkdir steps. Maybe it could, but then every
      // copy would have to carry around the fact that its parent may or may
      // not exist. It's simpler to deal with that here where we actually need
      // it.
      //
      // Note that the original stays where it is, since the x86 game keeps
      // reading it.
      //
      mkdir_p (s.to.parent_path ());

      error_code ec;
      fs::copy_file (s.from, s.to, fs::copy_options::overwrite_existing, ec);

      if (ec)
        fail ("\"{}\" could not be copied to \"{}\": {}.\n\n"
              "This game reads that file from where it was being copied to, so it "
              "cannot start until the copy goes through.",
              s.from.generic_string (), s.to.generic_string (), ec.message ());
    }

    // Most actions are things we can do here and now. Conversion is a
    // little different since doing it means starting Unlinker and
    // waiting for it later.
    //
    // We could represent that separately. But there is only one such
    // action, so perhaps the simplest representation is no apply
    // function. This gives execute() the distinction it needs without
    // teaching it about individual action values.
    //
    struct action_type
    {
      const char* doing;
      void (*apply) (const step&); // NULL for the actions that spawn.
    };

    // Note that this is indexed by action below, so its order is the
    // enum's order.
    //
    const action_type actions[] = {
      {"copying",    &do_copy},
      {"converting", nullptr}
    };

    const action_type&
    info (action a)
    {
      return actions[static_cast<size_t> (a)];
    }

    // Unlinker sits next to the game, which makes starting it fairly
    // simple. The command line looks slightly odd with the executable
    // in it as {0} despite passing c separately to CreateProcessA().
    // That is argv[0] for the child. {1} is the output directory and
    // {2} is the fastfile.
    //
    const char* const converter_name ("Unlinker.exe");
    const char* const converter_args (
      R"("{0}" --game IW4 --convert-to IW4MS -o "{1}" "{2}")");

    // The converters take much longer than this, of course. This is
    // just how long we let WaitForMultipleObjects() keep us before
    // giving the progress window another chance to animate.
    //
    const DWORD frame_interval (16);

    HANDLE
    start (const step& s)
    {
      string c (converter_name);
      string o (s.to.parent_path ().generic_string ());
      string f (s.from.generic_string ());

      // We would like Unlinker to write into the same log as us. Since
      // it is a separate process, that basically means giving it an
      // inheritable handle and wiring that handle up as stdout and
      // stderr.
      //
      SECURITY_ATTRIBUTES sa {};
      sa.nLength = sizeof (sa);
      sa.bInheritHandle = TRUE;

      // There may be several Unlinkers writing at once, so open for
      // append and let the other instances keep the file open too.
      // Their lines may end up mixed together. Presumably that is fine
      // as long as each diagnostic says which fastfile it belongs to.
      //
      HANDLE l (CreateFileA (log_file,
                             FILE_APPEND_DATA,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             &sa,
                             OPEN_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL,
                             nullptr));

      STARTUPINFOA si {};
      si.cb = sizeof (si);
      si.dwFlags = STARTF_USESHOWWINDOW;
      si.wShowWindow = SW_HIDE;

      // Failing to open the log should not turn into failing the
      // conversion. If that happened then maybe the log itself would be
      // the reason the game could no longer start. So redirect if we
      // have a handle and otherwise let Unlinker run without it.
      //
      if (l != INVALID_HANDLE_VALUE)
      {
        si.dwFlags |= STARTF_USESTDHANDLES;
        si.hStdOutput = l;
        si.hStdError = l;
      }

      string cl (vformat (converter_args, make_format_args (c, o, f)));

      PROCESS_INFORMATION pi {};

      // CreateProcessA() may modify the command line, hence the mutable
      // data().
      //
      // Handle inheritance is interesting only when l exists. If it
      // doesn't, leave inheritance off instead of perhaps handing the
      // child some other inheritable handle that happens to exist in
      // the game.
      //
      bool r (CreateProcessA (c.c_str (),
                              cl.data (),
                              nullptr,
                              nullptr,
                              l != INVALID_HANDLE_VALUE,
                              CREATE_NO_WINDOW,
                              nullptr,
                              nullptr,
                              &si,
                              &pi) != FALSE);

      // Save this before doing any cleanup. GetLastError() is basically
      // a bit of ambient state and some innocent Win32 call below could
      // replace it.
      //
      string e (r ? string () : Utils::GetLastWindowsError ());

      if (l != INVALID_HANDLE_VALUE)
        CloseHandle (l);

      // If one Unlinker cannot even start, trying another fastfile is
      // unlikely to teach us anything new. This is presumably an
      // installation problem, or perhaps antivirus got involved, so
      // stop here.
      //
      //
      // @@: Maybe we should disambiguate the cause?
      //
      if (!r)
        fail ("\"{}\" could not be started: {}\n\n"
              "Antivirus software such as Windows Defender often blocks or "
              "quarantines it. Add an exclusion for the IW4x folder and start the "
              "game again.",
              c, e);

      // We only keep the process around so we can wait for it. The
      // initial thread handle has no further use here.
      //
      CloseHandle (pi.hThread);

      return pi.hProcess;
    }

    string
    reject (const step& s, DWORD e)
    {
      // There are a few ways a conversion can look finished without
      // giving us a usable fastfile. Start with what Unlinker itself
      // reported.
      //
      if (e != 0)
        return format ("{} exited with code {}", converter_name, e);

      // Maybe it returned success but never produced the file. Strange,
      // but there is nothing for the game to load in that case.
      //
      if (!fs::exists (s.to))
        return format ("{} wrote nothing to \"{}\"", converter_name, s.to.generic_string ());

      // And maybe there is a file but it isn't actually the conversion
      // we asked for. Probe it rather than assuming a zero exit code
      // implies the right layout.
      //
      if (probe (s.to) != arch::x64)
        return format ("\"{}\" did not come out in the x64 layout", s.to.generic_string ());

      return {};
    }

    vector<fs::path>
    spawn (const vector<const step*>& ss, progress& pg)
    {
      if (!fs::exists (converter_name))
        fail ("IW4x's own fastfiles are in the x86 layout, which have to be "
              "converted before the game can read them, and {} was not found "
              "next to the game.\n\n"
              "If you did install it, antivirus software such as Windows Defender "
              "may have quarantined it. Add an exclusion for the IW4x folder and "
              "start the game again.",
              converter_name);

      // One converter per processor seems like a reasonable place to
      // start. There is one Win32 nuisance though.
      // WaitForMultipleObjects() can only watch MAXIMUM_WAIT_OBJECTS
      // handles in one call. Since the loop below waits on every
      // running converter together, that gives us a natural ceiling.
      //
      // And, of course, there is no point reserving more slots than
      // fastfiles.
      //
      SYSTEM_INFO si {};
      GetNativeSystemInfo (&si);

      size_t l (std::min<size_t> (
                  std::clamp<size_t> (si.dwNumberOfProcessors,
                                      1,
                                      MAXIMUM_WAIT_OBJECTS),
                  ss.size ()));

      log ("converting {} fastfiles, {} at a time", ss.size (), l);

      // The interesting thing about a running converter is its process
      // handle and which step produced it. We cannot use the vector
      // position for the latter since entries disappear from the middle
      // as processes finish.
      //
      struct running
      {
        HANDLE h;
        size_t s;
      };

      vector<running> rs;
      vector<fs::path> failed;

      // next walks the not-yet-started part of ss. done is for the
      // progress bar and counts finished attempts, successful or
      // otherwise.
      //
      size_t next (0);
      size_t done (0);

      // Keep going for as long as there is something waiting to start
      // or something we have started and still need to collect.
      //
      while (next < ss.size () || !rs.empty ())
      {
        // Remember the size before filling the free slots. If it
        // changes then the set of names shown by the progress window
        // changes too.
        //
        size_t n (rs.size ());

        // Fill all available slots before waiting. Then whenever one
        // converter finishes, the next trip around the loop starts its
        // replacement.
        //
        while (rs.size () < l && next < ss.size ())
        {
          rs.push_back ({start (*ss[next]), next});
          ++next;
        }

        if (rs.size () != n)
        {
          // Perhaps pg could take the steps directly, but then the
          // progress code would have to know what a step is. All it
          // really wants here are the names currently being worked on.
          //
          vector<string> ns;

          for (const running& r: rs)
            ns.push_back (ss[r.s]->from.filename ().generic_string ());

          pg.running (ns);
        }

        // WaitForMultipleObjects() wants just handles, so make that
        // view of the running set. It is small and only lives for this
        // wait.
        //
        vector<HANDLE> hs;

        for (const running& r: rs)
          hs.push_back (r.h);

        DWORD w (WaitForMultipleObjects (static_cast<DWORD> (hs.size ()),
                                         hs.data (),
                                         FALSE,
                                         frame_interval));

        // This is by far the normal result. Sixteen milliseconds is
        // nothing compared to converting a fastfile, so usually all we
        // have learned is that the progress window gets another frame.
        //
        if (w == WAIT_TIMEOUT)
        {
          pg.tick ();
          continue;
        }

        // If the wait itself fails then we have lost the useful way of
        // telling which converter is done. Maybe we could start polling
        // every process individually at this point, but there isn't
        // much value in inventing a second scheduler for what is
        // already a fairly exceptional failure.
        //
        if (w == WAIT_FAILED)
          fail ("Waiting for {} failed: {}\n\n"
                "The game cannot tell whether the fastfiles were converted. Start "
                "it again.",
                converter_name, Utils::GetLastWindowsError ());

        // We asked for any one process, so Windows gives us its index
        // relative to WAIT_OBJECT_0. hs mirrors rs, which makes the
        // same index useful in both.
        //
        size_t i (w - WAIT_OBJECT_0);
        const step& s (*ss[rs[i].s]);

        // A signalled process handle should have an exit code waiting
        // for us. Still, start with failure. If GetExitCodeProcess()
        // somehow fails, then guessing success would be the unfortunate
        // direction to guess in.
        //
        DWORD e (1);
        GetExitCodeProcess (rs[i].h, &e);
        CloseHandle (rs[i].h);
        rs.erase (rs.begin () + static_cast<ptrdiff_t> (i));

        if (string r (reject (s, e)); !r.empty ())
        {
          // There may be a partial output from a failed conversion. Leaving it
          // there is particularly awkward since the next run may see a file in
          // the converted tree and conclude this step has already
          // happened.
          //
          // So remove whatever Unlinker left behind. If that removal
          // itself fails, the original conversion failure is still the
          // useful thing to report here.
          //
          error_code ec;
          fs::remove (s.to, ec);

          log ("could not convert \"{}\": {}", s.from.generic_string (), r);
          failed.push_back (s.from);
        }

        // "Finished" here means this converter is no longer running. A
        // rejected result still counts, otherwise the bar would stop
        // short with no work left in rs.
        //
        pg.finished (++done);
      }

      return failed;
    }

    vector<fs::path>
    execute (const plan& p, progress& pg)
    {
      // Most of the plan can simply be executed as we walk it.
      // Conversion is the odd bit since doing one at a time would spend
      // minutes serializing work that has no reason to wait for its
      // neighbour.
      //
      // So keep those steps aside and let spawn() deal with them
      // together once all the cheap work is done.
      //
      vector<const step*> ss;

      for (const step& s: p)
      {
        const action_type& a (info (s.act));

        if (a.apply == nullptr)
        {
          ss.push_back (&s);
          continue;
        }

        if (s.to.empty ())
          log ("{} \"{}\"", a.doing, s.from.generic_string ());
        else
          log ("{} \"{}\" to \"{}\"",
               a.doing,
               s.from.generic_string (),
               s.to.generic_string ());

        a.apply (s);
      }

      // If there were no conversions then we are done already. This
      // lets spawn() assume it has some actual work, which makes its
      // concurrency calculation pleasantly straightforward.
      //
      return ss.empty () ? vector<fs::path> () : spawn (ss, pg);
    }
  }

  void
  convert ()
  {
    // Build a phase immediately before running it. This matters since
    // the phase before it may have moved things around and the next
    // planner should see that new state.
    //
    for (const phase_type& ph: phases)
    {
      plan p;
      ph.make (p);

      if (p.empty ())
        continue;

      // Conversion is represented by the missing apply function, so use
      // the same test here. Apart from being consistent with execute(),
      // we keeps ourself from having to know which action happens to
      // mean conversion.
      //
      size_t n (static_cast<size_t> (
                  ranges::count_if (p,
                                    [] (const step& s)
                                    {
                                      return info (s.act).apply == nullptr;
                                    })));

      log ("{}: {} steps, {} of them conversions", ph.name, p.size (), n);

      vector<fs::path> f;

      // There is no point flashing a window for a phase that just moves
      // a few files around. If there are conversions, though, they may
      // sit here for minutes and then some visible sign of life is
      // useful.
      //
      // The scope is intentional too. Suppose a conversion failed. We
      // want the progress window gone before fail() below shows the
      // result. If it stayed behind the message box, a full bar that no
      // longer moves would perhaps look like another part of the
      // conversion was still hung.
      //
      if (n == 0)
      {
        silent_progress pg;
        f = execute (p, pg);
      }
      else
      {
        window_progress pg ("IW4x - Converting fastfiles", n);
        f = execute (p, pg);
      }

      // Individual conversion failures don't stop the batch. Maybe the
      // next fastfile is perfectly fine and there is little reason to
      // throw that work away. So spawn() collects the fastfiles and we
      // report the lot here once every converter has had its chance.
      //
      if (!f.empty ())
      {
        string ns;

        for (const fs::path& s: f)
          ns.append (s.filename ().generic_string ()).append ("\n");

        string m (format ("{} of {} fastfiles could not be converted:\n\n{}\n"
                          "See \"{}\" for what {} said about them.",
                          f.size (), n, ns, log_file, converter_name));

        // Offer to send the fastfiles along with the log, since we most
        // likely need the fastfile itself to tell what went wrong. Writing
        // the report may take a while with large fastfiles, so do it while
        // the player reads the message. Note that the reporter then shows up
        // on its own.
        //
        vector<fs::path> as {fs::path (log_file)};
        as.insert (as.end (), f.begin (), f.end ());

        jthread t (
          [o = Sentry::discover (),
           as = move (as),
           d = format ("{} of {} fastfiles could not be converted",
                       f.size (), n)] ()
          {
            Sentry::offer (o, "zone_conversion", d, as);
          });

        fail (m, t);
      }
    }
  }

  string
  search_path (string_view g)
  {
    // Internally the paths use generic separators, which is convenient
    // for constructing them. This string goes back to the game, though,
    // and it expects the usual Windows spelling. So construct it first
    // and fix the separators at the boundary.
    //
    // Note that this is a directory prefix, hence the trailing
    // separator.
    //
    string r (format ("{}/{}/", converted_root.generic_string (), g));
    ranges::replace (r, '/', '\\');

    return r;
  }
}

namespace Components
{
  std::string ZoneConvert::
  SearchPath (const std::string_view group)
  {
    return zone::search_path (group);
  }

  ZoneConvert::
  ZoneConvert ()
  {
    zone::convert ();
  }
}
