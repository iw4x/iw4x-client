#include <zlib.h>

#include "ZoneConvert.hpp"

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
    // The game keeps its fastfiles under zone and we put the converted
    // copy under zone/iw4x/x86.
    //
    // Note that this means iw4x will turn up when we enumerate
    // directories under zone looking for zone groups. So keep the name
    // separately and use it to recognize our own subtree. We could
    // extract it from converted_root, of course, though spelling it out
    // here makes this special case below a bit easier to see.
    //
    const fs::path zone_root ("zone");
    const fs::path converted_root ("zone/iw4x/x86");
    const string converted_name ("iw4x");

    // The loose IW4x files are a little different. The game loads these
    // from fs_basegame which we point at BASEGAME. Older installations
    // used BASEGAME_LEGACY, so for now we have to know about both
    // locations.
    //
    // Presumably at some point the legacy location can disappear
    // together with the migration code. Until then there isn't much to
    // gain from trying to hide this distinction elsewhere.
    //
    const fs::path basegame_root (BASEGAME);
    const fs::path legacy_basegame_root (BASEGAME_LEGACY);

    // The old converter left converted.txt in the backup directories it
    // made. Nothing creates this file anymore, though naturally
    // installations that went through the old converter can still have
    // it. So keep recognizing it as part of the old layout.
    //
    const string legacy_marker ("converted.txt");

    // And this is the other part of that layout. The language backup
    // was simply called "old" while the other zone groups had "_old"
    // appended to their names.
    //
    // This is perhaps a little too liberal as a test since any
    // directory with such a name will match. It is, though, the
    // convention the old converter used and at this point all we need
    // to do is recognize it.
    //
    bool
    legacy_backup (const string& n)
    {
      return n == "old" || n.ends_with ("_old");
    }
  }

  // The fastfiles.
  //
  namespace
  {
    // There are two fastfile layouts that we care about. x86 is the
    // layout the game reads and x64 is the one shipped by recent Steam
    // installations. The latter is, of course, what we are looking for
    // here since those are the files that have to go through the
    // converter.
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

    // This is deliberately just the group-name test. Once the caller
    // has found one of our groups it can use the entry above to decide
    // whether a particular fastfile is, after all, one of the game's.
    //
    bool
    own_group (const string& n)
    {
      return ranges::any_of (own_groups,
                             [&n] (const group_type& g)
      {
        return n == g.name;
      });
    }
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
    enum class action
    {
      move,    // Move the file with nothing already at the destination.
      replace, // Move the file over what is already at the destination.
      drop,    // Remove the file since the destination is newer.
      convert, // Convert the file and write the result to `to`.
      prune    // Remove the directory together with empty directories below it.
    };

    // A step is one such operation. For move, replace, and convert we
    // need both sides. For drop and prune there is naturally nowhere to
    // move anything to, so `to` is simply left empty.
    //
    // Perhaps we could have separate representations for those cases,
    // though there doesn't seem to be much gained by making the plan
    // itself harder to walk.
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
    // There is first the old IW4x directory next to the game. What used
    // to live there is now read from BASEGAME, so if we still find the
    // old directory we want to fold it into the new one instead of
    // leaving it behind indefinitely.
    //
    // The only slightly interesting case is a file that already exists
    // in both places. This normally means the launcher has installed a
    // newer copy into BASEGAME since the old layout was created. So
    // keep that one and throw away the old copy instead of moving it
    // over the top.
    //
    void
    plan_basegame (plan& p)
    {
      if (!fs::is_directory (legacy_basegame_root))
        return;

      for (const fs::directory_entry& e:
             fs::recursive_directory_iterator (legacy_basegame_root))
      {
        if (!e.is_regular_file ())
          continue;

        fs::path r (e.path ().lexically_relative (legacy_basegame_root));
        fs::path t (basegame_root / r);

        if (fs::exists (t))
          p.push_back ({action::drop, e.path (), {}});
        else
          p.push_back ({action::move, e.path (), move (t)});
      }

      // And once all the files have somewhere to go, the old directory itself
      // can disappear.
      //
      p.push_back ({action::prune, legacy_basegame_root, {}});
    }

    // Recovery is a little more involved. An older converter used to
    // replace the fastfiles in a group with their x86 copies and move
    // the originals into a backup directory next to it. So, for
    // example, what started as dlc could end up split between dlc and
    // dlc_old.
    //
    // What we want now is fairly clear: put the originals back where
    // the game expects them and move the converted copies under
    // converted_root. The trouble is that an installation may have
    // changed since the old converter ran, and in a few states there is
    // no longer enough information to tell which file came from where.
    //
    // Perhaps the safest thing in those cases is simply not to invent
    // an answer. We have looked at the whole installation before
    // executing the plan, so a refusal here still leaves it exactly as
    // we found it.
    //
    [[noreturn]] void
    refuse (const fs::path& b, const fs::path& g, const string& w)
    {
      fail ("{}\n\n"
            "This installation was converted by an older version of IW4x, and its "
            "fastfiles cannot be put back where they belong without guessing.\n\n"
            "Move the fastfiles in \"{}\" back to \"{}\", delete \"{}\", and start "
            "the game again. See \"{}\" for what was found.",
            w,
            b.generic_string (),
            g.generic_string (),
            b.generic_string (),
            log_file);
    }

    // Now, "old" is a bit special. The other backup directories have
    // the original group name in "<group>_old", while here the old
    // converter simply used "old" regardless of the language. So we
    // have to recover the language name from what is still under zone.
    //
    string
    backup_language (const fs::path& b)
    {
      vector<string> ls;

      for (const fs::directory_entry& e: fs::directory_iterator (zone_root))
      {
        if (!e.is_directory ())
          continue;

        string n (e.path ().filename ().generic_string ());

        if (n == converted_name || own_group (n) || legacy_backup (n))
          continue;

        ls.push_back (move (n));
      }

      // If there is only one language directory, then that's that.
      //
      if (ls.size () == 1)
        return ls.front ();

      // Otherwise things get a little more interesting. The old
      // converter moved the originals out and put the converted files
      // back under the same names. So the language directory the backup
      // came from should still have, at least to some degree, the same
      // set of filenames.
      //
      // Let's count these matches and see if one language comes out
      // ahead.
      //
      vector<pair<size_t, string>> rs;

      for (string& l: ls)
      {
        size_t n (0);

        for (const fs::directory_entry& e: fs::directory_iterator (b))
        {
          if (fs::exists (zone_root / l / e.path ().filename ()))
            ++n;
        }

        rs.emplace_back (n, move (l));
      }

      ranges::sort (rs, ranges::greater ());

      // There are two ways this can fail to tell us anything. We may
      // have no matches at all or two languages may match equally well.
      // Perhaps we could try to be cleverer here, but a wrong guess
      // means putting the originals into the wrong language directory
      // and then carrying on as if everything was fine. Better leave
      // that decision to the user.
      //
      if (rs.empty () ||
          rs.front ().first == 0 ||
          (rs.size () > 1 && rs.front ().first == rs[1].first))
      {
        fail ("\"{}\" holds the fastfiles an older version of IW4x replaced, and "
              "there is no telling which of the language directories under \"{}\" "
              "they came out of.\n\n"
              "Move them back into the directory they belong to, delete \"{}\", "
              "and start the game again.",
              b.generic_string (),
              zone_root.generic_string (),
              b.generic_string ());
      }

      return rs.front ().second;
    }

    void
    plan_recovery (plan& p)
    {
      if (!fs::is_directory (zone_root))
        return;

      // Now go through the backups. For "<group>_old" the group name is
      // right there in the directory name. For "old" we have just gone
      // through the trouble of figuring it out above.
      //
      for (const fs::directory_entry& e: fs::directory_iterator (zone_root))
      {
        if (!e.is_directory ())
          continue;

        const fs::path& b (e.path ());
        string n (b.filename ().generic_string ());

        if (!legacy_backup (n))
          continue;

        string g (n == "old"
                  ? backup_language (b)
                  : n.substr (0, n.size () - strlen ("_old")));

        fs::path sd (zone_root / g);       // Where the group is read from.
        fs::path cd (converted_root / g);  // Where iw4x reads it from.

        for (const fs::directory_entry& f: fs::directory_iterator (b))
        {
          string fn (f.path ().filename ().generic_string ());

          // The old marker is easy enough. It was only there for the converter
          // and has no meaning once we have recovered the directory.
          //
          if (fn == legacy_marker)
          {
            p.push_back ({action::drop, f.path (), {}});
            continue;
          }

          // Everything else is supposed to be one of the originals.
          // Bail out if it isn't even a regular file since at that
          // point the old layout no longer has the shape we know how to
          // recover.
          //
          if (!f.is_regular_file ())
            refuse (b, sd, "There is something in the backup directory that is not a fastfile.");

          fs::path s (sd / fn); // The converted copy, if it is still in place.
          fs::path c (cd / fn); // Where it belongs now.

          // First, we may already be halfway through this recovery. If
          // c exists, then presumably we moved the converted copy there
          // on an earlier start and stopped before putting the original
          // back.
          //
          // This is fine as long as s is now empty. If it isn't, then
          // we have a file in both places and there is no useful way to
          // tell which state we are looking at.
          //
          if (fs::exists (c))
          {
            if (fs::exists (s))
              refuse (b, sd, "There is a copy of this fastfile in both places, and "
                             "nothing to say which of the two the game should read.");

            p.push_back ({action::move, f.path (), move (s)});
            continue;
          }

          // If s doesn't exist either, things are simpler. There is no
          // converted copy left to preserve, so just put the original
          // back.
          //
          // Perhaps the old converter never got to this file. Or
          // perhaps the converted copy was removed later. It doesn't
          // really matter here.
          //
          if (!fs::exists (s))
          {
            p.push_back ({action::move, f.path (), move (s)});
            continue;
          }

          // Which leaves the old converter's normal state: the original
          // is in the backup and something with the same name is still
          // in the group. That something should be the x86 copy.
          //
          // Check this rather than assuming it from the name. If it is
          // x64, then it plainly isn't the converted copy. If we cannot
          // tell what it is, then we are back to guessing.
          //
          optional<arch> a (probe (s));

          if (a != arch::x86)
          {
            refuse (b, sd,
                    a
                    ? "The fastfile in the group directory is itself in the x64 "
                      "layout, so it is not the converted copy of the one in the "
                      "backup directory."
                    : "The fastfile in the group directory cannot be read, so "
                      "there is no telling whether it is the converted copy of "
                      "the one in the backup directory.");
          }

          // Now move the converted copy out of the way and put the
          // original back. Note that the order matters. If we stop
          // after the first move, then on the next start c exists and s
          // doesn't, which is precisely the interrupted case above.
          //
          p.push_back ({action::move, s, move (c)});
          p.push_back ({action::move, f.path (), move (s)});
        }

        p.push_back ({action::prune, b, {}});
      }
    }

    // With the old backups dealt with, let's look at the groups that
    // are ours. Our fastfiles belong under converted_root now. The x64
    // ones are useful where they are for a little longer since
    // plan_conversion() still needs them as input.
    //
    void
    plan_adoption (plan& p)
    {
      for (const group_type& g: own_groups)
      {
        fs::path d (zone_root / g.name);

        if (!fs::is_directory (d))
          continue;

        for (const fs::directory_entry& e: fs::directory_iterator (d))
        {
          if (!e.is_regular_file ())
            continue;

          string n (e.path ().filename ().generic_string ());

          // dlc is/was shared, as discussed above, so leave its stock
          // files where Steam put them.
          //
          if (g.shared != nullptr && g.shared (n))
            continue;

        // And leave x64 alone for now. These are the files the next pass will
        // actually convert.
        //
          if (probe (e.path ()) == arch::x64)
            continue;

          // There can already be a file with this name under
          // converted_root. In that case what we have here is the copy
          // installed afterwards, so it is the one we want to keep.
          //
          p.push_back ({action::replace, e.path (), converted_root / g.name / n});
        }

        p.push_back ({action::prune, move (d), {}});
      }
    }

    // Now all that should be left for us in the normal zone tree are
    // the x64 fastfiles. Keep those where steam put them and write the
    // converted copy under the corresponding group in converted_root.
    //
    void
    plan_conversion (plan& p)
    {
      if (!fs::is_directory (zone_root))
        return;

      for (const fs::directory_entry& e: fs::directory_iterator (zone_root))
      {
        if (!e.is_directory ())
          continue;

        string g (e.path ().filename ().generic_string ());

        // Our converted tree will, naturally, turn up in this walk as
        // well. Old backups can turn up here too, though those should
        // have been dealt with above. Neither is input to the
        // converter.
        //
        if (g == converted_name || legacy_backup (g))
          continue;

        for (const fs::directory_entry& f: fs::directory_iterator (e.path ()))
        {
          if (!f.is_regular_file ())
            continue;

          fs::path t (converted_root / g / f.path ().filename ());

           // Perhaps we got this far on an earlier start. If the result
          // is already there, then there isn't anything left to do for
          // this file.
          //
          if (fs::exists (t))
            continue;

          // And by now only x64 fastfiles are interesting. Everything else was
          // either adopted above or isn't something this conversion
          // knows what to do with.
          //
          if (probe (f.path ()) != arch::x64)
            continue;

          p.push_back ({action::convert, f.path (), move (t)});
        }
      }
    }

    // Finally, there is a reason these are separate phases and, perhaps
    // more importantly, why they are in this order.
    //
    // Recovery can put fastfiles back into one of our groups. Adoption
    // then takes our files out of those groups and puts them under
    // converted_root. What remains after that is what conversion gets
    // to look at.
    //
    // We could perhaps make one pass do all of this, though then it
    // would have to reason about filesystem changes that have not
    // happened yet. Keeping the phases separate means each one sees the
    // layout left by the one before it.
    //
    struct phase_type
    {
      const char* name;
      void (*make) (plan&);
    };

    const phase_type phases[] = {
      {"moving IW4x's own files under \"" BASEGAME "\"", &plan_basegame},
      {"putting the stock layout back",                  &plan_recovery},
      {"taking over IW4x's fastfiles",                   &plan_adoption},
      {"converting fastfiles",                           &plan_conversion}
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
        case WM_CLOSE: 			return 0;
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
      HDC 		b  (CreateCompatibleDC (dc));
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
    do_move (const step& s)
    {
      // The plan doesn't contain mkdir steps. Maybe it could, but then
      // every move would have to carry around the fact that its parent
      // may or may not exist. It is simpler to deal with that here
      // where we actually need it.
      //
      mkdir_p (s.to.parent_path ());

      error_code ec;
      fs::rename (s.from, s.to, ec);

      if (ec)
        fail ("\"{}\" could not be moved to \"{}\": {}.\n\n"
              "This game reads that file from where it was being moved to, so it "
              "cannot start until the move goes through.",
              s.from.generic_string (), s.to.generic_string (), ec.message ());
    }

    void
    do_replace (const step& s)
    {
      // This is basically a move except there may already be something
      // where we are going. Try to get it out of the way first.
      //
      // We could perhaps diagnose remove() separately. But if it failed
      // in a way that matters then do_move() is going to fail on the
      // same destination immediately afterwards and that is the more
      // useful operation to report.
      //
      error_code ec;
      fs::remove (s.to, ec);

      do_move (s);
    }

    void
    do_drop (const step& s)
    {
      // A drop is best effort. If the file stays around then the rest
      // of the plan can still make progress, so remember what happened
      // and carry on.
      //
      error_code ec;
      fs::remove (s.from, ec);

      if (ec)
        log ("\"{}\" could not be removed: {}",
             s.from.generic_string (), ec.message ());
    }

    void
    do_prune (const step& s)
    {
      // It may already be gone. If so, then pruning has rather
      // conveniently been done for us.
      //
      if (!fs::is_directory (s.from))
        return;

      // At this point the files we cared about have been moved out and
      // what should remain is mostly their old directory structure.
      // "Should" is doing some work here since a user, or perhaps Steam
      // may have put something else in there.
      //
      // We don't know what such a thing is and, more importantly, it
      // isn't part of the plan. So don't inspect files and decide
      // whether they look disposable. Just try to remove directories. A
      // directory with something left in it will refuse to go away,
      // which is exactly what we want.
      //
      // There is the usual little wrinkle that the children have to go
      // before their parents. Remember them during the walk and then
      // replay the list backwards.
      //
      vector<fs::path> ds;

      for (const fs::directory_entry& e:
             fs::recursive_directory_iterator (s.from))
      {
        if (e.is_directory ())
          ds.push_back (e.path ());
      }

      error_code ec;

      for (const fs::path& d: ds | views::reverse)
        fs::remove (d, ec);

      // And finally see if the root can go as well. If it cannot, then
      // presumably one of those unknown things is still somewhere below
      // it. Leave the whole remainder alone and say so once.
      //
      fs::remove (s.from, ec);

      if (fs::exists (s.from))
        log ("\"{}\" still holds something and was left alone", s.from.generic_string ());
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
      {"moving",     &do_move},
      {"replacing",  &do_replace},
      {"dropping",   &do_drop},
      {"converting", nullptr},
      {"pruning",    &do_prune}
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
      R"("{0}" --game IW4MS --convert-to IW4 -o "{1}" "{2}")");

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
      if (probe (s.to) != arch::x86)
        return format ("\"{}\" did not come out in the x86 layout", s.to.generic_string ());

      return {};
    }

    vector<string>
    spawn (const vector<const step*>& ss, progress& pg)
    {
      if (!fs::exists (converter_name))
        fail ("This installation ships fastfiles in the x64 layout, which have to "
              "be converted before the game can read them, and {} was not found "
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
      vector<string> failed;

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
          failed.push_back (s.from.filename ().generic_string ());
        }

        // "Finished" here means this converter is no longer running. A
        // rejected result still counts, otherwise the bar would stop
        // short with no work left in rs.
        //
        pg.finished (++done);
      }

      return failed;
    }

    vector<string>
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
      return ss.empty () ? vector<string> () : spawn (ss, pg);
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

      vector<string> f;

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
      // throw that work away. So spawn() collects the names and we
      // report the lot here once every converter has had its chance.
      //
      if (!f.empty ())
      {
        string ns;

        for (const string& s: f)
          ns.append (s).append ("\n");

        fail ("{} of {} fastfiles could not be converted:\n\n{}\n"
              "See \"{}\" for what {} said about them.",
              f.size (), n, ns, log_file, converter_name);
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
