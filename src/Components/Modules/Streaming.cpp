#include "Streaming.hpp"

#include "AssetHandler.hpp"
#include "Events.hpp"
#include "FileSystem.hpp"
#include "Renderer.hpp"
#include "Scheduler.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <deque>
#include <mutex>
#include <optional>
#include <stop_token>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

namespace Components
{
  namespace
  {
    // Image_LoadFromFileWithReader() is where IW4 reads an .iwi file
    // into a texture, so that's where we jump in and load the image
    // ourselves if we stream it. Image_Release() tells us that an image
    // is going away and Image_Reload() that r_applyPicmip wants it at
    // another picmip.
    //
    // The tricky one is finding out what is actually drawn. We could
    // try to work it out from the scene (what's in view, how far away
    // it is) but that feels like a lot of guessing for little gain. So
    // what we do instead is watch R_SetSampler(), which binds the
    // images of a material and so sees every image right before it's
    // used.
    //
    // Note that IW4 also calls SetTexture() directly in a few places
    // (see RB_DrawSurfs() and friends). As far as we can tell, these
    // bind lightmaps and reflection probes, which we don't stream
    // anyway, and one of them unbinds everything. So it seems safe to
    // ignore them, at least for now.
    //
    constexpr uintptr_t r_set_sampler                    (0x140068EA0);
    constexpr uintptr_t image_load_from_file_with_reader (0x14006A160);
    constexpr uintptr_t image_release                    (0x140037650);
    constexpr uintptr_t image_reload                     (0x140037E90);

    // The picmip that r_picmip and friends end up as once applied, one
    // for each kind of image that IW4 picmips (see sharpest_tier()).
    // Note that IW4 clamps these to [0, 3] when it reads them, so we do
    // the same just in case someone pokes a funny value in there.
    //
    constexpr uintptr_t picmip_color    (0x1493CFDB8);
    constexpr uintptr_t picmip_normal   (0x1493CFDBC);
    constexpr uintptr_t picmip_specular (0x1493CFDC0);

    // IW4 never picmips the images whose names start with one of a list
    // of prefixes (weapon_ and ally_, for example). Presumably the idea
    // is that what the player looks at the most should stay sharp no
    // matter what. This is that check. We don't know what IW4 calls it,
    // so we call it what it does.
    //
    constexpr uintptr_t picmip_exempt   (0x140276990);

    // The $black image, which R_InitImages() looks up next to $white
    // and the others (rgp.blackImage).
    //
    constexpr uintptr_t black_image (0x148CCA510);

    // The zone loader's varGfxImage. While Load_GfxImage() loads an
    // image, this points to the zone's copy of it, which is also the
    // copy that Load_Texture() gets (see relocate() for why we care).
    //
    constexpr uintptr_t var_gfx_image (0x140DB5E28);

    // Note that R_SetSampler() takes the command buffer context by
    // value and so our hook has to as well, otherwise the arguments
    // won't line up.
    //
    using r_set_sampler_t = void (Game::GfxCmdBufContext,
                                  unsigned int sampler,
                                  unsigned char state,
                                  const Game::GfxImage*);

    using image_load_t    = bool (Game::GfxImage*, Game::Reader_t);
    using image_release_t = void (Game::GfxImage*);
    using image_reload_t  = void (Game::GfxImage*);

    r_set_sampler_t* r_set_sampler_original (nullptr);
    image_load_t*    image_load_original    (nullptr);
    image_release_t* image_release_original (nullptr);
    image_reload_t*  image_reload_original  (nullptr);

    // An .iwi file starts with a 32-byte header that, among other
    // things, has the size the file would be with its sharpest mipmaps
    // dropped, for picmip 0 to 3 (fileSizeForPicmip). The mipmaps
    // follow from the smallest to the largest and so the first
    // fileSizeForPicmip[p] bytes (header included) are a complete image
    // at picmip p. For example, a 2048x2048 DXT5 normal map is 5592464
    // bytes and its picmip 3 image (256x256) is the first 87440 of
    // them.
    //
    // This is what IW4 relies on when it applies picmip, and it turns
    // out to be exactly what we need to load an image one step at a
    // time. We call these four prefixes tiers, with tier 0 being the
    // sharpest. Could we go finer than that, say, one mipmap at a time?
    // Probably, but the header only vouches for these four and four
    // steps seem to be enough in practice.
    //
    // We use no_tier where there is no tier to speak of, for example,
    // when there is no read in flight for an image.
    //
    constexpr size_t   tier_count  (4);
    constexpr size_t   header_size (sizeof (Game::GfxImageFileHeader));
    constexpr uint8_t  no_tier     (0xFF);

    // The header flags that make IW4 load an image some other way. The
    // two lowest (no picmip, no mipmaps) make it ignore picmip and the
    // ones at 0x30000 make the image a cube map and a volume map. The
    // ones at 0x7000000 pick the kind of texture, a render target among
    // them (see Image_Setup()). Ours are always plain textures in the
    // managed pool and we would rather not find out what happens if we
    // pretend otherwise, so let's leave all of these to IW4.
    //
    constexpr uint32_t foreign_flags (0x3 | 0x30000 | 0x7000000);

    // How the formats that we stream are stored, in the file and in
    // video memory. The block is the edge of what the format compresses
    // together, 4 pixels for DXT and 1 for the bitmaps. RGB is the only
    // format whose size differs between the two since D3D has no 24-bit
    // format and IW4 expands it to XRGB with an opaque alpha (see
    // Image_LoadBitmap()).
    //
    // Note that we leave the wavelet formats to IW4. These seem rare
    // enough on PC that writing a decoder doesn't feel worth it, and
    // IW4 decodes them into the same bitmap formats anyway. If they
    // ever turn out to matter, then this is the place to add them.
    //
    struct format_info
    {
      Game::GfxImageFileFormat file;
      D3DFORMAT                d3d;
      uint32_t                 block;     // Edge in pixels.
      uint32_t                 file_size; // Bytes in the file.
      uint32_t                 card_size; // Bytes in video memory.
    };

    const format_info formats[]
    {
      {Game::IMG_FORMAT_BITMAP_RGBA,            D3DFMT_A8R8G8B8, 1, 4,  4},
      {Game::IMG_FORMAT_BITMAP_RGB,             D3DFMT_X8R8G8B8, 1, 3,  4},
      {Game::IMG_FORMAT_BITMAP_LUMINANCE_ALPHA, D3DFMT_A8L8,     1, 2,  2},
      {Game::IMG_FORMAT_BITMAP_LUMINANCE,       D3DFMT_L8,       1, 1,  1},
      {Game::IMG_FORMAT_BITMAP_ALPHA,           D3DFMT_A8,       1, 1,  1},
      {Game::IMG_FORMAT_DXT1,                   D3DFMT_DXT1,     4, 8,  8},
      {Game::IMG_FORMAT_DXT3,                   D3DFMT_DXT3,     4, 16, 16},
      {Game::IMG_FORMAT_DXT5,                   D3DFMT_DXT5,     4, 16, 16}
    };

    // The layout of an .iwi file that we can stream, as described by
    // its header and checked against it (see parse_layout()).
    //
    struct layout
    {
      const format_info* format;
      uint32_t           width;
      uint32_t           height;
      uint32_t           levels;    // Mipmaps down to 1x1.
      uint32_t           file_size;

      // The bytes after the header that make up each tier. Unlike
      // fileSizeForPicmip, these don't count the header.
      //
      array<uint32_t, tier_count> tier_size;
    };

    // Return the extent e at mipmap level m.
    //
    inline uint32_t
    mip_extent (uint32_t e, uint32_t m) noexcept
    {
      return max (e >> m, 1u);
    }

    // Return the number of blocks along the extent e at mipmap level m.
    //
    inline uint32_t
    mip_blocks (const layout& l, uint32_t e, uint32_t m) noexcept
    {
      const uint32_t b (l.format->block);
      return (mip_extent (e, m) + b - 1) / b;
    }

    // Return the size of mipmap level m in the file and in video
    // memory, respectively.
    //
    inline uint32_t
    file_mip_size (const layout& l, uint32_t m) noexcept
    {
      return mip_blocks (l, l.width, m) *
             mip_blocks (l, l.height, m) *
             l.format->file_size;
    }

    inline uint32_t
    card_mip_size (const layout& l, uint32_t m) noexcept
    {
      return mip_blocks (l, l.width, m) *
             mip_blocks (l, l.height, m) *
             l.format->card_size;
    }

    // Return the video memory that an image takes at tier t, that is,
    // its mipmaps from level t down to 1x1. This is also what IW4 puts
    // into GfxImage::cardMemory (see Image_TrackTexture()), though the
    // driver probably pads it some.
    //
    uint64_t
    card_size (const layout& l, uint32_t t) noexcept
    {
      uint64_t r (0);
      for (uint32_t m (t); m != l.levels; ++m)
        r += card_mip_size (l, m);
      return r;
    }

    // Return the layout of an .iwi file with the specified header and
    // size if we can stream it and nullopt if we should leave it to
    // IW4.
    //
    // Besides the formats we don't handle, we leave to IW4 whatever it
    // wouldn't picmip itself, which is what the flag and extent checks
    // below are about. We could probably stream a few more images if we
    // tried harder, but what would we gain? IW4 knows how to load
    // anything we don't expect while we would most likely get it wrong,
    // so it feels like being conservative is the right call here.
    //
    // @@: Actually, we may want to account for other versions here,
    // particularly for custom maps.
    //
    optional<layout>
    parse_layout (const Game::GfxImageFileHeader& h, uint32_t file_size)
    {
      // Version 8 is the one IW4 reads.
      //
      if (h.tag[0] != 'I' || h.tag[1] != 'W' || h.tag[2] != 'i' ||
          h.version != 8)
        return nullopt;

      if ((h.flags & foreign_flags) != 0)
        return nullopt;

      const auto& fs (formats);
      auto i (ranges::find (fs,
                            static_cast<Game::GfxImageFileFormat> (static_cast<uint8_t> (h.format)),
                            &format_info::file));
      if (i == ranges::end (fs))
        return nullopt;

      // A depth other than 1 means a volume map.
      //
      if (h.dimensions[0] <= 0 || h.dimensions[1] <= 0 || h.dimensions[2] != 1)
        return nullopt;

      const uint32_t w (static_cast<uint32_t> (h.dimensions[0]));
      const uint32_t v (static_cast<uint32_t> (h.dimensions[1]));

      // IW4 doesn't picmip an image with fewer than 32 pixels along an
      // edge, so there would be nothing to stream.
      //
      // We also insist on power of two extents. For others D3D creates
      // fewer levels than the file has (it rounds the logarithm down
      // while the file rounds it up) and IW4 would then upload into a
      // level that doesn't exist. Does IW4 ever ship such an image? Not
      // sure, but we would rather not find out.
      //
      if (min (w, v) < 32 || !has_single_bit (w) || !has_single_bit (v))
        return nullopt;

      layout r {&*i, w, v, static_cast<uint32_t> (bit_width (max (w, v))), file_size, {}};

      // Add the mipmaps up from the smallest and see that each tier
      // ends where the header says it does. If it doesn't, then our
      // idea of the layout is wrong for this file (a tool that writes
      // them differently, perhaps) and we had better not read it as
      // tiers.
      //
      uint64_t n (0);
      for (uint32_t m (r.levels); m-- != 0; )
      {
        n += file_mip_size (r, m);

        if (m < tier_count)
        {
          if (header_size + n != static_cast<uint64_t> (h.fileSizeForPicmip[m]))
            return nullopt;

          r.tier_size[m] = static_cast<uint32_t> (n);
        }
      }

      if (header_size + n != file_size)
        return nullopt;

      return r;
    }

    // Return the path that Image_LoadFromFileWithReader() opens for an
    // image.
    //
    string
    image_path (const Game::GfxImage& i)
    {
      return string ("images/") + i.name + ".iwi";
    }

    // Return true if we stream this image, provided its file agrees
    // (see parse_layout()).
    //
    // These are the images that IW4 loads from a file and would picmip,
    // which comes down to their semantic. The rest (the 2D images of
    // the menus and the HUD, say) are presumably meant to stay crisp
    // and are probably small anyway, so let's leave them alone.
    //
    bool
    streamable (const Game::GfxImage& i) noexcept
    {
      if (i.category != Game::IMG_CATEGORY_LOAD_FROM_FILE ||
          i.mapType != Game::MAPTYPE_2D                   ||
          i.noPicmip                                      ||
          i.name == nullptr)
        return false;

      switch (i.semantic)
      {
        case Game::TS_COLOR_MAP:
        case Game::TS_DETAIL_MAP:
        case Game::TS_WATER_MAP:
        case Game::TS_NORMAL_MAP:
        case Game::TS_SPECULAR_MAP: return true;
        default:                    return false;
      }
    }

    // Return the sharpest tier that IW4 would load this image at, that
    // is, its picmip. This mirrors Image_LoadFromFileWithReader() and
    // Image_Reload(), and if those ever disagree with us, then they
    // win.
    //
    uint8_t
    sharpest_tier (const Game::GfxImage& i) noexcept
    {
      if (Utils::Hook::Call<bool (const char*)> (picmip_exempt) (i.name))
        return 0;

      int p (0);
      switch (i.semantic)
      {
        case Game::TS_COLOR_MAP:
        case Game::TS_DETAIL_MAP:
        case Game::TS_WATER_MAP:    p = *reinterpret_cast<const int*> (picmip_color);    break;
        case Game::TS_NORMAL_MAP:   p = *reinterpret_cast<const int*> (picmip_normal);   break;
        case Game::TS_SPECULAR_MAP: p = *reinterpret_cast<const int*> (picmip_specular); break;
        default:                    break;
      }

      return static_cast<uint8_t> (clamp (p, 0, static_cast<int> (tier_count) - 1));
    }

    // Create a texture for tier t of an image and fill it from data,
    // which is the file's bytes after the header up to the end of that
    // tier. Return nullptr and set hr if D3D won't have it.
    //
    // This is what Image_Setup() and Image_LoadBitmap()
    // (Image_LoadDxtc() for DXT) come down to for the images that we
    // stream.
    //
    IDirect3DTexture9*
    create_texture (IDirect3DDevice9* d,
                    const layout& l,
                    uint32_t t,
                    const uint8_t* data,
                    HRESULT& hr)
    {
      const uint32_t levels (l.levels - t);

      // Create it in the managed pool with no usage, which is where IW4
      // puts these images too.
      //
      // We could create it in the default pool and upload through a
      // staging texture instead, which would probably save the system
      // memory copy that the managed pool keeps. But then we would also
      // have to recreate every texture after a device reset, and that
      // feels like a lot of work for now. Note also that IW4x's D3D9Ex
      // mode doesn't have the managed pool and D3D9Ex CreateTexture()
      // moves the texture into the default pool anyway.
      //
      IDirect3DTexture9* r (nullptr);
      hr = d->CreateTexture (mip_extent (l.width, t),
                             mip_extent (l.height, t),
                             levels,
                             0 /* usage */,
                             l.format->d3d,
                             D3DPOOL_MANAGED,
                             &r,
                             nullptr);
      if (FAILED (hr))
        return nullptr;

      // Make sure we got the levels we asked for. In theory a device
      // could cap them, and then our levels would no longer match the
      // file's. We haven't seen it happen, but checking is cheap.
      //
      if (r->GetLevelCount () != levels)
      {
        r->Release ();
        hr = E_FAIL;
        return nullptr;
      }

      // Upload the mipmaps, smallest first since that's how the file
      // has them. Note that the rows here are rows of blocks, so a
      // 16-pixel DXT mipmap has 4 of them, and that the texture's level
      // 0 is mipmap t.
      //
      const uint8_t* p (data);
      for (uint32_t m (l.levels); m-- != t; )
      {
        const uint32_t rows       (mip_blocks (l, l.height, m));
        const uint32_t row_blocks (mip_blocks (l, l.width, m));
        const uint32_t file_row   (row_blocks * l.format->file_size);
        const uint32_t card_row   (row_blocks * l.format->card_size);

        D3DLOCKED_RECT lr;
        hr = r->LockRect (m - t, &lr, nullptr, 0);
        if (FAILED (hr))
        {
          r->Release ();
          return nullptr;
        }

        uint8_t* q (static_cast<uint8_t*> (lr.pBits));
        for (uint32_t y (0); y != rows; ++y)
        {
          const uint8_t* s (p + static_cast<size_t> (y) * file_row);
          uint8_t*       o (q + static_cast<size_t> (y) * lr.Pitch);

          if (l.format->file_size == l.format->card_size)
            memcpy (o, s, card_row);
          else
          {
            // RGB to XRGB, as Image_LoadBitmap() does it.
            //
            for (uint32_t x (0); x != row_blocks; ++x)
            {
              o[x * 4 + 0] = s[x * 3 + 0];
              o[x * 4 + 1] = s[x * 3 + 1];
              o[x * 4 + 2] = s[x * 3 + 2];
              o[x * 4 + 3] = 0xFF;
            }
          }
        }

        r->UnlockRect (m - t);
        p += file_mip_size (l, m);
      }

      return r;
    }

    // Create a texture for tier t of an image from its current texture
    // at the sharper tier c by copying the mipmaps that the two have in
    // common. Return nullptr and set hr if D3D won't have it.
    //
    // This is how we take an image down. We could just as well read the
    // coarser tier from the file again, but why bother? The mipmaps we
    // need are already in the current texture. Note also that locking a
    // managed texture reads its copy in system memory, so this doesn't
    // have to wait for the GPU.
    //
    IDirect3DTexture9*
    shrink_texture (IDirect3DDevice9* d,
                    IDirect3DTexture9* from,
                    const layout& l,
                    uint32_t c,
                    uint32_t t,
                    HRESULT& hr)
    {
      assert (c < t);

      const uint32_t levels (l.levels - t);

      IDirect3DTexture9* r (nullptr);
      hr = d->CreateTexture (mip_extent (l.width, t),
                             mip_extent (l.height, t),
                             levels,
                             0 /* usage */,
                             l.format->d3d,
                             D3DPOOL_MANAGED,
                             &r,
                             nullptr);
      if (FAILED (hr))
        return nullptr;

      if (r->GetLevelCount () != levels) // As above.
      {
        r->Release ();
        hr = E_FAIL;
        return nullptr;
      }

      for (uint32_t m (t); m != l.levels; ++m)
      {
        const uint32_t rows     (mip_blocks (l, l.height, m));
        const uint32_t card_row (mip_blocks (l, l.width, m) * l.format->card_size);

        D3DLOCKED_RECT s;
        hr = from->LockRect (m - c, &s, nullptr, D3DLOCK_READONLY);
        if (FAILED (hr))
        {
          r->Release ();
          return nullptr;
        }

        D3DLOCKED_RECT o;
        hr = r->LockRect (m - t, &o, nullptr, 0);
        if (FAILED (hr))
        {
          from->UnlockRect (m - c);
          r->Release ();
          return nullptr;
        }

        for (uint32_t y (0); y != rows; ++y)
          memcpy (static_cast<uint8_t*> (o.pBits) + static_cast<size_t> (y) * o.Pitch,
                  static_cast<const uint8_t*> (s.pBits) + static_cast<size_t> (y) * s.Pitch,
                  card_row);

        r->UnlockRect (m - t);
        from->UnlockRect (m - c);
      }

      return r;
    }

    // Make x, the texture for tier t, the image's texture.
    //
    // We set the image's members the way IW4 leaves them after it loads
    // an image at picmip t (see Image_Setup() and
    // Image_TrackTexture()). In particular, the extent is the one at
    // that picmip, which is also what IW4's picmip does, and the second
    // picmip (and memory) is what IW4 always puts there for picmipped
    // images. Why it's 2 we don't really know, perhaps something left
    // over from the consoles. We just copy it.
    //
    void
    install (Game::GfxImage& i, IDirect3DTexture9* x, const layout& l, uint32_t t)
    {
      i.texture.map              = x;
      i.mapType                  = Game::MAPTYPE_2D;
      i.width                    = static_cast<uint16_t> (mip_extent (l.width, t));
      i.height                   = static_cast<uint16_t> (mip_extent (l.height, t));
      i.depth                    = 1;
      i.picmip.platform[0]       = static_cast<char> (t);
      i.picmip.platform[1]       = 2;
      i.cardMemory.platform[0]   = static_cast<int> (card_size (l, t));
      i.cardMemory.platform[1]   = static_cast<int> (card_size (l, 2));
    }

    // A tier of an image for the worker to read.
    //
    // Besides the tier we pass the file size and header that we loaded
    // the image with. What if the file system restarts before the
    // worker gets to the request (after an iwd was added, say)? Then
    // the file may come from a different iwd and we would mix tiers of
    // two different images. So the worker compares the two and fails
    // the read if they don't match.
    //
    struct request
    {
      uint32_t                 index;      // Of the record.
      uint32_t                 generation; // Of the record (see record).
      uint8_t                  tier;
      string                   path;
      uint32_t                 file_size;
      Game::GfxImageFileHeader header;
      uint32_t                 size;       // Bytes after the header.
    };

    // What the worker read for a request. The data is empty if it
    // couldn't read it, while the size is always the one from the
    // request.
    //
    struct completion
    {
      uint32_t        index;
      uint32_t        generation;
      uint8_t         tier;
      uint32_t        size;
      vector<uint8_t> data;
    };

    // An image that we stream.
    //
    // A record is live while it has an image. We bump the generation
    // every time we forget a record, which is how a read that completes
    // later finds out that the image it was for is gone (the record may
    // well have been reused by then). We could cancel the read instead,
    // but the worker may already be in the middle of it and this seems
    // simpler.
    //
    // The tiers satisfy sharpest <= resident <= coarsest, where
    // sharpest is IW4's picmip for the image and coarsest is the tier
    // we load it at and never go below.
    //
    struct record
    {
      Game::GfxImage*          image      = nullptr;
      IDirect3DTexture9*       texture    = nullptr; // Normally the image's.
      uint32_t                 generation = 0;
      layout                   info       {};
      Game::GfxImageFileHeader header     {};

      uint8_t  sharpest = 0;
      uint8_t  resident = 0;       // Tier of the texture.
      uint8_t  coarsest = 0;
      uint8_t  pending  = no_tier; // Tier being read.
      uint64_t reserved = 0;       // Memory the read will add.
      int      failures = 0;       // Failed reads and creates.

      bool     used      = false;  // Drawn at least once.
      int      last_used = 0;      // Sys_Milliseconds() of the last draw.

      // Set while the image is the zone's copy that the database is
      // about to copy into its pool (see relocate()). We leave such an
      // image alone since IW4 would draw the pool's copy with whatever
      // texture the image had at the time, and we would have no way to
      // tell.
      //
      bool     staged    = false;

      bool
      live () const noexcept {return image != nullptr;}
    };

    // A texture that we replaced and the frame in which we did (see
    // retire_textures()).
    //
    struct retired
    {
      IDirect3DTexture9* texture;
      uint32_t           frame;
    };

    // The images that the render thread binds in a frame (see mark()).
    //
    // There are two of these and the binds of a frame go into the one
    // that its number picks while we collect the other. We could just
    // take a lock and add to a set instead, but this runs for every
    // bind and a lock there could well show up as stutter.
    //
    // Note that this is deliberately approximate. A bind that races
    // with the frame change may get lost and, should a frame bind more
    // distinct images than there is room for, we drop the rest. Is that
    // a problem? Probably not, since both only delay when an image
    // counts as in use, and the hold time covers that anyway.
    //
    constexpr size_t mark_capacity (1 << 14);

    struct mark_buffer
    {
      array<atomic<const Game::GfxImage*>, mark_capacity> images {};
      atomic<uint32_t>                                    size   {0};
    };

    // The sampler states that R_SetSampler() has seen, so that we can
    // clear an image from their caches (see unbind()). IW4 seems to
    // only use a couple of these, so 8 should be enough. If it isn't,
    // then we would miss clearing some caches and a sharper image would
    // show up a little later, which seems harmless.
    //
    constexpr size_t state_capacity (8);

    // How many tiers the worker may have in flight and how many bytes
    // they may add up to. We don't expose these as dvars, at least for
    // now, since all they do is bound the memory that a burst of
    // requests takes. How fast images get sharper is up to the upload
    // limit. The numbers themselves are pretty arbitrary.
    //
    constexpr size_t   max_requests      (32);
    constexpr uint64_t max_request_bytes (256ull << 20);

    // How many failed reads we put up with before we leave an image
    // where it is. A file that changed under us (after an iwd was
    // added, say) will keep failing and there is no point in reading it
    // every frame. Why not give up on the first failure? Perhaps the
    // read failed for some passing reason, so a couple more tries seem
    // fair.
    //
    constexpr int max_failures (3);

    // How many frames a replaced texture stays alive (see
    // retire_textures()).
    //
    constexpr uint32_t retire_frames (2);

    // The streaming state.
    //
    // Most of it is protected by mutex_. The exception is the marks,
    // which the render thread writes on every bind and so without any
    // locking (see mark()). What we share with the worker is protected
    // by io_mutex_, which we take while holding mutex_ if we need both
    // (but never the other way around, since that could deadlock).
    //
    class streamer
    {
    public:
      // Hooks.
      //
      bool
      load (Game::GfxImage*, Game::Reader_t);

      void
      release (Game::GfxImage*);

      bool
      reload (Game::GfxImage*);

      void
      mark (const Game::GfxCmdBufContext&, const Game::GfxImage*) noexcept;

      const Game::GfxImage*
      substitute (const Game::GfxImage*);

      void
      relocate (const Game::GfxImage* loaded, Game::GfxImage* added);

      // Events.
      //
      void
      frame (IDirect3DDevice9*);

      void
      suspend ();

      void
      resume ();

      void
      shutdown ();

      void
      register_dvars ();

      void
      print_info ();

    private:
      // Records. Note that these and the functions below, up to the
      // worker, expect mutex_ to be held.
      //
      uint32_t
      add (Game::GfxImage*);

      void
      forget (uint32_t);

      record*
      find (const Game::GfxImage*);

      bool
      attached (const record&) const noexcept;

      void
      replace (record&, IDirect3DTexture9*, uint32_t tier);

      void
      unbind (const Game::GfxImage*) noexcept;

      // Streaming.
      //
      bool
      shrink (IDirect3DDevice9*, record&, uint8_t tier);

      void
      apply (IDirect3DDevice9*, completion&&);

      void
      plan (IDirect3DDevice9*, int now);

      bool
      issue (record&, uint8_t tier);

      uint64_t
      evict (IDirect3DDevice9*, uint64_t bytes, vector<uint32_t>& candidates, size_t& next);

      void
      collect_marks (int now);

      void
      retire_textures (bool all);

      uint64_t
      budget (IDirect3DDevice9*);

      bool
      enabled () const noexcept;

      bool
      upload_left () const noexcept;

      // Worker.
      //
      void
      start_worker ();

      void
      work (stop_token);

      optional<vector<uint8_t>>
      read (const request&);

    private:
      mutable mutex mutex_;

      // The records, with free_ listing the ones to reuse and index_
      // the live ones by image. We keep the records in a vector and
      // hand out indexes rather than pointers since the worker's
      // requests outlive many a push_back().
      //
      vector<record>                                   records_;
      vector<uint32_t>                                 free_;
      unordered_map<const Game::GfxImage*, uint32_t>   index_;
      deque<retired>                                   retired_;

      uint64_t resident_       = 0;     // Memory our textures take.
      uint64_t reserved_       = 0;     // Memory the reads will add.
      uint64_t auto_budget_    = 0;     // 0 until we ask (see budget()).
      uint64_t uploaded_       = 0;     // Uploaded in this frame.
      uint64_t upload_limit_   = 0;
      bool     suspended_      = false; // Device being reset.
      bool     announced_      = false; // Printed the automatic budget.

      // Statistics for r_streamInfo.
      //
      uint64_t upgrades_       = 0;
      uint64_t downgrades_     = 0;
      uint64_t read_failures_  = 0;
      uint64_t create_failures_ = 0;

      // Written without any locking (see mark()).
      //
      array<mark_buffer, 2>                                   marks_;
      atomic<uint32_t>                                        frame_ {1};
      atomic<uint64_t>                                        dropped_marks_ {0};
      array<atomic<Game::GfxCmdBufState*>, state_capacity>    states_ {};

      // The worker and what goes to and from it, protected by
      // io_mutex_. Note that the in flight counts are ours, though, and
      // protected by mutex_.
      //
      // Just one worker? Reading a few MB from an iwd doesn't seem to
      // be the bottleneck (uploading them is) and the database thread's
      // file handle only allows one reader at a time anyway (see
      // read()).
      //
      mutex                  io_mutex_;
      condition_variable_any io_condition_;
      deque<request>         requests_;
      deque<completion>      completions_;
      size_t                 in_flight_       = 0;
      uint64_t               in_flight_bytes_ = 0;
      jthread                worker_;
      once_flag              worker_started_;

      // Dvars. Note that r_loadForRenderer is IW4's (see load()).
      //
      const Game::dvar_t* r_stream_textures_     = nullptr;
      const Game::dvar_t* r_stream_budget_       = nullptr;
      const Game::dvar_t* r_stream_upload_limit_ = nullptr;
      const Game::dvar_t* r_stream_hold_time_    = nullptr;
      const Game::dvar_t* r_stream_show_unloaded_ = nullptr;
      const Game::dvar_t* r_load_for_renderer_   = nullptr;
    };

    // Note that we never destroy the streamer. Its worker may be in the
    // middle of a read when the process exits and by the time the
    // static destructors run IW4's file system is long gone. Leaking it
    // seems like the safer choice.
    //
    streamer& s (*new streamer);

    // Return true if r_streamTextures is on.
    //
    bool streamer::
    enabled () const noexcept
    {
      return r_stream_textures_ != nullptr && r_stream_textures_->current.enabled;
    }

    // Return true if this frame's uploads aren't used up yet.
    //
    bool streamer::
    upload_left () const noexcept
    {
      return uploaded_ < upload_limit_;
    }

    // Load an image from its file the way we stream it, that is, at its
    // coarsest tier. Return false if the image is for IW4 to load,
    // which it then does. Runs on the thread that loads the image,
    // normally the database thread.
    //
    bool streamer::
    load (Game::GfxImage* image, Game::Reader_t reader)
    {
      // There is no texture to create without a renderer, which
      // Image_Setup() checks with r_loadForRenderer as well. The
      // renderer registers that dvar, so let's look it up the first
      // time we get here.
      //
      if (r_load_for_renderer_ == nullptr)
        r_load_for_renderer_ = Game::Dvar_FindVar ("r_loadForRenderer");

      if (!enabled ()                                                    ||
          r_load_for_renderer_ == nullptr                                ||
          !r_load_for_renderer_->current.enabled                         ||
          !streamable (*image))
        return false;

      IDirect3DDevice9* device (*Game::dx_ptr);
      if (device == nullptr)
        return false;

      // Open the file with IW4's reader, which picks the file handle of
      // the thread we are on. Note that IW4 holds FileSystem::FSMutex
      // while it loads images (see LoadTextureSync()), and that's what
      // keeps the worker away from the database thread's handle while
      // we use it.
      //
      const string path (image_path (*image));

      Game::fileHandle_t h (0);
      const int n (reader (path.c_str (), &h));
      if (n < 0 || h == 0)
        return false;

      // Read the header and see if we can stream this file. If we
      // can't, then close it and let IW4 open it again. That's a wasted
      // open, but this way, if the load is going to fail, then it fails
      // with IW4's diagnostics and not ours.
      //
      Game::GfxImageFileHeader header;
      optional<layout> l;
      if (Game::FS_Read (&header, static_cast<int> (header_size), h) == static_cast<int> (header_size))
        l = parse_layout (header, static_cast<uint32_t> (n));

      if (!l)
      {
        Game::FS_FCloseFile (h);
        return false;
      }

      // Read the coarsest tier, which comes right after the header.
      // This is where the load time goes down: for a large image it's
      // normally a tiny fraction of the file.
      //
      const uint8_t sharpest (sharpest_tier (*image));
      const uint8_t coarsest (static_cast<uint8_t> (tier_count - 1));

      vector<uint8_t> data (l->tier_size[coarsest]);
      const bool read (Game::FS_Read (data.data (), static_cast<int> (data.size ()), h) == static_cast<int> (data.size ()));
      Game::FS_FCloseFile (h);

      if (!read)
        return false;

      HRESULT hr;
      IDirect3DTexture9* x (create_texture (device, *l, coarsest, data.data (), hr));
      if (x == nullptr)
      {
        Logger::Warning (Game::CON_CHANNEL_GFX,
                         "Couldn't create a texture for image '{}' ({:#x}), leaving it to the game\n",
                         image->name, static_cast<uint32_t> (hr));
        return false;
      }

      lock_guard g (mutex_);

      // If we already stream this image, then start over with it.
      // Normally IW4 releases an image before loading it again, but
      // what if it doesn't? If the image still has the texture we gave
      // it, then that texture is ours to release. Otherwise it would
      // leak since IW4 sets the new texture without releasing whatever
      // was there (its callers release it beforehand).
      //
      if (record* o = find (image))
      {
        if (attached (*o))
          retired_.push_back (retired {o->texture, frame_.load (memory_order_relaxed)});

        forget (static_cast<uint32_t> (o - records_.data ()));
      }

      // Make the texture the image's and remember the image. Note that
      // we do this with the lock held so that frame() never sees the
      // new texture without its record. It would take that for a
      // texture that IW4 has replaced and stop streaming the image (see
      // attached()), and such a race would be hard to track down.
      //
      install (*image, x, *l, coarsest);

      record& r (records_[add (image)]);
      r.texture  = x;
      r.info     = *l;
      r.header   = header;
      r.sharpest = sharpest;
      r.coarsest = max (coarsest, sharpest);
      r.resident = coarsest;

      // Load_Texture() loads into the zone's copy of the image, which
      // DB_AddXAsset() then copies into the pool (see relocate()),
      // while everything else loads what is already in the pool, the
      // delayed images included. So how do we tell the two apart? Our
      // first idea was to go by the thread, but the delayed images load
      // on the database thread as well (see DB_LoadDelayedImages()).
      // What does work is varGfxImage, which points to the copy that
      // Load_Texture() gets.
      //
      r.staged   = Game::Sys_IsDatabaseThread () &&
                   image == *reinterpret_cast<Game::GfxImage* const*> (var_gfx_image);

      resident_ += card_size (*l, coarsest);

      start_worker ();
      return true;
    }

    // Forget an image that IW4 is about to release. IW4 releases the
    // image's texture along with it, ours included, so there is nothing
    // else for us to do. Runs on the thread that unloads the image's
    // zone.
    //
    void streamer::
    release (Game::GfxImage* image)
    {
      lock_guard g (mutex_);

      if (record* r = find (image))
        forget (static_cast<uint32_t> (r - records_.data ()));
    }

    // Take an image to its new sharpest tier if its picmip changed.
    // Return false if we don't stream it, in which case IW4 reloads it.
    // Runs on the main thread.
    //
    // Note that we don't reload the image here. IW4 would read the
    // whole file again, which is exactly what we are trying to avoid.
    // Instead, we only update the image's tiers and let plan() move it
    // the way it moves any other image. If the image is now sharper
    // than it may be, then plan() takes it down right away.
    //
    bool streamer::
    reload (Game::GfxImage* image)
    {
      lock_guard g (mutex_);

      record* r (find (image));
      if (r == nullptr)
        return false;

      r->sharpest = sharpest_tier (*image);
      r->coarsest = max (r->coarsest, r->sharpest);
      return true;
    }

    // Mark the image as drawn in this frame. Runs on the render thread
    // for every bind, so this has to be cheap.
    //
    // Most binds are of images that were already bound in the same
    // frame, so we filter those out first with a small table of the
    // images that this thread has seen in this frame. A collision only
    // means that we mark an image twice, which is wasteful but
    // harmless.
    //
    void streamer::
    mark (const Game::GfxCmdBufContext& c, const Game::GfxImage* image) noexcept
    {
      struct seen
      {
        const Game::GfxImage* image;
        uint32_t              frame;
      };

      thread_local array<seen, 1024>      filter {};
      thread_local Game::GfxCmdBufState*  last_state (nullptr);

      // See if this is a sampler state we haven't seen before. There
      // are only a few of them and a thread normally uses one for a
      // long time, so remembering the last one should be enough to keep
      // this off the common path.
      //
      if (c.state != last_state && c.state != nullptr)
      {
        last_state = c.state;

        for (auto& e: states_)
        {
          Game::GfxCmdBufState* p (e.load (memory_order_acquire));
          if (p == c.state)
            break;

          if (p == nullptr)
          {
            if (e.compare_exchange_strong (p, c.state, memory_order_acq_rel))
              break;

            if (p == c.state)
              break;
          }
        }
      }

      // Skip the images that this thread has already marked in this
      // frame (see above).
      //
      const uint32_t f (frame_.load (memory_order_relaxed));

      const uintptr_t a (reinterpret_cast<uintptr_t> (image));
      seen& e (filter[((a >> 3) * 0x9E3779B97F4A7C15ull) >> 54]);
      if (e.image == image && e.frame == f)
        return;

      e.image = image;
      e.frame = f;

      // Add the image to this frame's buffer. If it's full, then count
      // the mark as dropped so that r_streamInfo can tell us if this
      // ever happens in practice.
      //
      mark_buffer& b (marks_[f & 1]);
      const uint32_t i (b.size.fetch_add (1, memory_order_relaxed));
      if (i < mark_capacity)
        b.images[i].store (image, memory_order_relaxed);
      else
        dropped_marks_.fetch_add (1, memory_order_relaxed);
    }

    // Return the image to bind in place of this one. Runs on the render
    // thread, for every bind.
    //
    // With r_streamShowUnloaded set, this is IW4's black image for
    // every streamed image that isn't at its sharpest tier yet, so that
    // we can see what the streaming is still working on. We have to
    // look the image up for that, which means taking the lock on every
    // bind. That's probably too slow to leave on, but for debugging it
    // seems good enough, and so we only do it when asked to.
    //
    const Game::GfxImage* streamer::
    substitute (const Game::GfxImage* image)
    {
      if (r_stream_show_unloaded_ == nullptr || !r_stream_show_unloaded_->current.enabled)
        return image;

      const Game::GfxImage* black (*reinterpret_cast<Game::GfxImage* const*> (black_image));
      if (black == nullptr)
        return image;

      lock_guard g (mutex_);

      const record* r (find (image));
      return r != nullptr && r->resident != r->sharpest ? black : image;
    }

    // Follow an image from the zone's copy to the database's. Runs on
    // the database thread right after DB_AddXAsset() has made the copy.
    //
    // This one took a while to figure out. The zone loads an image into
    // a copy that Load_Texture() creates the texture for, and
    // DB_AddXAsset() then copies the image into the database pool. The
    // materials end up with the pool's copy, so that's the one IW4
    // draws. If we didn't follow the image there, then we would keep
    // replacing the texture of a copy that nothing draws, while the one
    // that is drawn would stay at the coarsest tier and, worse, keep
    // using that texture after we release it. In practice this is
    // mostly the weapons since most of the map's images are delayed and
    // so load straight into the pool.
    //
    // We could perhaps hook DB_AddXAsset() for images and stream the
    // pool's copy from the start, but by then the texture is already
    // created and we would be loading the image twice. Following it is
    // cheaper.
    //
    // Note that if AssetHandler kept the image out of the database,
    // then the two are the same and all we do is clear the staged flag.
    //
    void streamer::
    relocate (const Game::GfxImage* loaded, Game::GfxImage* added)
    {
      lock_guard g (mutex_);

      auto i (index_.find (loaded));
      if (i == index_.end ())
        return;

      const uint32_t n (i->second);
      record& r (records_[n]);

      // If the pool's copy doesn't have our texture, then the database
      // kept the image that it already had. This is what DB_AddXAsset()
      // does when a zone of the same priority has already added an
      // image with this name, and it then frees the new entry. Wouldn't
      // that leak our texture? It turns out it doesn't. Before freeing
      // the entry, IW4 calls the image's release handler on it (the
      // table at 0x140420180), which for images is just a jump to
      // Image_Release(). So our texture is already gone by the time we
      // get here, and all that's left to do is forget the record.
      //
      // Note that our release hook does run for that copy but doesn't
      // find it, since the record is still under the zone's copy. Which
      // is fine, we don't release anything in forget() and so nothing
      // is released twice.
      //
      if (added == nullptr || added->texture.map != r.texture)
      {
        forget (n);
        return;
      }

      if (added != loaded)
      {
        // If the pool's copy is already streamed (the same image loaded
        // by another zone, say), then its record is stale.
        //
        if (record* o = find (added))
          forget (static_cast<uint32_t> (o - records_.data ()));

        index_.erase (loaded);
        index_.emplace (added, n);
        r.image = added;
      }

      r.staged = false;
    }

    // Do the streaming work of a frame. Runs on the render thread right
    // before Present().
    //
    // This is the only place where we change the textures of images
    // that are already loaded. Why here? All the frame's draws have
    // been issued by then, so nothing binds in the meantime and we can
    // clear the sampler states that cached a changed image (see
    // unbind()). Doing it from the worker would perhaps show up sooner,
    // but then we would be racing the renderer and we would much rather
    // not.
    //
    void streamer::
    frame (IDirect3DDevice9* device)
    {
      const int now (Game::Sys_Milliseconds ());

      lock_guard g (mutex_);

      // Collect the marks even if there is nothing else to do, so that
      // the buffers keep getting cleared.
      //
      collect_marks (now);
      retire_textures (false);

      if (suspended_ || records_.empty ())
        return;

      // Install what the worker read, as long as this frame's upload
      // limit allows. What doesn't fit waits for the next frame with
      // its memory still reserved. The limit is there so that a burst
      // of reads doesn't turn into one long frame, which we would
      // rather spread over a few short ones. Before the dvars are
      // registered we go with the default.
      //
      uploaded_     = 0;
      upload_limit_ = r_stream_upload_limit_ != nullptr
        ? static_cast<uint64_t> (r_stream_upload_limit_->current.integer) << 10
        : 8ull << 20;

      for (;;)
      {
        if (!upload_left ())
          break;

        completion c;
        {
          lock_guard l (io_mutex_);
          if (completions_.empty ())
            break;

          c = move (completions_.front ());
          completions_.pop_front ();
        }

        apply (device, move (c));
      }

      plan (device, now);
    }

    // Stop changing textures until resume(). Called when the device is
    // about to be reset (with D3D9Ex disabled) and before vid_restart.
    //
    // Note that we release the textures we replaced right away. Before
    // vid_restart the device is about to be destroyed and anything we
    // still hold would prevent that. We also forget the automatic
    // budget since the device may well report a different amount
    // afterwards (a new mode, a different adapter perhaps). What the
    // worker reads in the meantime is dropped in apply() unless the
    // image is still there.
    //
    void streamer::
    suspend ()
    {
      lock_guard g (mutex_);

      suspended_   = true;
      auto_budget_ = 0;
      retire_textures (true);
    }

    // Resume streaming once the device is back.
    //
    void streamer::
    resume ()
    {
      lock_guard g (mutex_);
      suspended_ = false;
    }

    // Stop the worker before IW4 shuts its file system down. Otherwise
    // it could still be reading from a file handle that no longer
    // exists.
    //
    void streamer::
    shutdown ()
    {
      if (worker_.joinable ())
      {
        worker_.request_stop ();
        worker_.join ();
      }
    }

    // Register our dvars. Note that r_streamShowUnloaded isn't
    // archived. It's a debugging aid and we probably don't want anyone
    // to restart IW4 and wonder why half the world is black.
    //
    void streamer::
    register_dvars ()
    {
      r_stream_textures_ = Game::Dvar_RegisterBool (
        "r_streamTextures",
        true,
        Game::DVAR_ARCHIVE,
        "Stream the images that are loaded from files, loading their sharper mipmaps as they are drawn");

      r_stream_budget_ = Game::Dvar_RegisterInt (
        "r_streamBudget",
        0,
        0,
        65536,
        Game::DVAR_ARCHIVE,
        "Video memory for streamed images in MB, 0 to pick it from what the device reports");

      r_stream_upload_limit_ = Game::Dvar_RegisterInt (
        "r_streamUploadLimit",
        8192,
        256,
        262144,
        Game::DVAR_ARCHIVE,
        "Streamed image data to upload per frame in KB");

      r_stream_hold_time_ = Game::Dvar_RegisterInt (
        "r_streamHoldTime",
        3000,
        0,
        600000,
        Game::DVAR_ARCHIVE,
        "Milliseconds that a streamed image counts as in use after it was last drawn");

      r_stream_show_unloaded_ = Game::Dvar_RegisterBool (
        "r_streamShowUnloaded",
        false,
        Game::DVAR_NONE,
        "Draw streamed images that are not at their sharpest tier yet as black, for debugging");
    }

    // Print what the streaming is up to (r_streamInfo).
    //
    void streamer::
    print_info ()
    {
      lock_guard g (mutex_);

      size_t live (0), sharpest (0), coarsest (0), pending (0), stuck (0), staged (0);
      for (const record& r: records_)
      {
        if (!r.live ())
          continue;

        ++live;
        if (r.staged)                        ++staged;
        if (r.resident == r.sharpest)        ++sharpest;
        if (r.resident == r.coarsest)        ++coarsest;
        if (r.pending != no_tier)            ++pending;
        if (r.failures >= max_failures)      ++stuck;
      }

      const uint64_t b (r_stream_budget_ != nullptr && r_stream_budget_->current.integer != 0
                        ? static_cast<uint64_t> (r_stream_budget_->current.integer) << 20
                        : auto_budget_);

      Logger::Print (Game::CON_CHANNEL_GFX,
                     "Texture streaming is {}{}\n"
                     "  images:    {} streamed, {} at their sharpest tier, {} at their coarsest, {} being read, {} staged\n"
                     "  memory:    {} MB resident, {} MB reserved, {} MB budget{}\n"
                     "  changes:   {} up, {} down\n"
                     "  failures:  {} reads, {} textures, {} images given up on\n"
                     "  dropped:   {} binds\n",
                     enabled () ? "enabled" : "disabled",
                     suspended_ ? " (suspended)" : "",
                     live, sharpest, coarsest, pending, staged,
                     resident_ >> 20, reserved_ >> 20, b >> 20,
                     r_stream_budget_ != nullptr && r_stream_budget_->current.integer != 0 ? "" : " (auto)",
                     upgrades_, downgrades_,
                     read_failures_, create_failures_, stuck,
                     dropped_marks_.load (memory_order_relaxed));
    }

    // Add a record for an image and return its index, reusing a free
    // record if there is one. Note that we bump the generation here as
    // well, so that a read still in flight for the record's previous
    // image doesn't apply to this one. forget() already bumps it, so
    // this is redundant, but it's cheap and it keeps add() correct on
    // its own.
    //
    uint32_t streamer::
    add (Game::GfxImage* image)
    {
      uint32_t i;
      if (!free_.empty ())
      {
        i = free_.back ();
        free_.pop_back ();
      }
      else
      {
        i = static_cast<uint32_t> (records_.size ());
        records_.emplace_back ();
      }

      record& r (records_[i]);
      const uint32_t g (r.generation + 1);
      r = record ();
      r.image      = image;
      r.generation = g;

      index_.emplace (image, i);
      return i;
    }

    // Forget a record, giving its memory back to the budget. A read in
    // flight for it is dropped once it arrives (the generation no
    // longer matches) but still counts as in flight until then.
    //
    // Note that we don't release the texture here. While the image has
    // it, IW4 releases it along with the image (see attached() for when
    // it doesn't).
    //
    void streamer::
    forget (uint32_t i)
    {
      record& r (records_[i]);
      assert (r.live ());

      resident_ -= card_size (r.info, r.resident);
      reserved_ -= r.reserved;

      index_.erase (r.image);

      const uint32_t g (r.generation + 1);
      r = record ();
      r.generation = g;

      free_.push_back (i);
    }

    // Return the record of an image if we stream it and nullptr
    // otherwise.
    //
    record* streamer::
    find (const Game::GfxImage* image)
    {
      auto i (index_.find (image));
      return i != index_.end () ? &records_[i->second] : nullptr;
    }

    // Return true if the image still has the texture we gave it.
    //
    // It always should since we hook everything that we know of that
    // releases and reloads these images. But we probably don't know of
    // everything, and if something else changed it, then the texture is
    // no longer ours to replace. The safest thing to do then seems to
    // be to stop streaming the image.
    //
    bool streamer::
    attached (const record& r) const noexcept
    {
      return r.image->texture.map == r.texture;
    }

    // Make x, the texture for the specified tier, the image's, retiring
    // the one it had (see retire_textures()).
    //
    void streamer::
    replace (record& r, IDirect3DTexture9* x, uint32_t tier)
    {
      retired_.push_back (retired {r.texture, frame_.load (memory_order_relaxed)});

      install (*r.image, x, r.info, tier);
      unbind (r.image);

      resident_ += card_size (r.info, tier);
      resident_ -= card_size (r.info, r.resident);

      r.texture  = x;
      r.resident = static_cast<uint8_t> (tier);
    }

    // Clear an image from the sampler states' caches.
    //
    // R_SetSampler() skips the device if the sampler already has the
    // image that it is asked to bind, and so it would keep the texture
    // we just replaced bound until the sampler gets another image. IW4
    // does the same when it unbinds a render target, which is
    // reassuring. Note that the device keeps its own reference to the
    // old texture until then, so this is about drawing with the new
    // texture and not about the old one going away.
    //
    void streamer::
    unbind (const Game::GfxImage* image) noexcept
    {
      const Game::GfxTexture* t (&image->texture);

      for (const auto& e: states_)
      {
        Game::GfxCmdBufState* p (e.load (memory_order_acquire));
        if (p == nullptr)
          break;

        for (Game::GfxTexture*& x: p->samplerTexture)
          if (x == t)
            x = nullptr;
      }
    }

    // Take an image down to a coarser tier from its current texture.
    // Return false if D3D won't have it, in which case the image stays
    // as it is.
    //
    bool streamer::
    shrink (IDirect3DDevice9* device, record& r, uint8_t tier)
    {
      assert (tier > r.resident);

      HRESULT hr;
      IDirect3DTexture9* x (shrink_texture (device, r.texture, r.info, r.resident, tier, hr));
      if (x == nullptr)
      {
        ++create_failures_;
        return false;
      }

      uploaded_ += card_size (r.info, tier);
      replace (r, x, tier);
      ++downgrades_;
      return true;
    }

    // Install a tier that the worker read, if the image still wants it.
    //
    void streamer::
    apply (IDirect3DDevice9* device, completion&& c)
    {
      // The request counts as in flight until here, even if we end up
      // dropping what the worker read.
      //
      assert (in_flight_ != 0 && in_flight_bytes_ >= c.size);

      in_flight_       -= 1;
      in_flight_bytes_ -= c.size;

      // If we forgot the record while the read was in flight (and
      // perhaps reused it since), then there is nothing to install.
      //
      record& r (records_[c.index]);

      if (r.generation != c.generation)
        return;

      // Whatever came of the read, its reservation is gone.
      //
      reserved_ -= r.reserved;
      r.reserved = 0;
      r.pending  = no_tier;

      // Give up on the image after a few failures and say so once.
      // Saying it on every failure would probably just flood the
      // console.
      //
      if (c.data.empty ())
      {
        ++read_failures_;

        if (++r.failures == max_failures)
          Logger::Warning (Game::CON_CHANNEL_GFX,
                           "Couldn't read '{}' for streaming, keeping it at {}x{}\n",
                           image_path (*r.image),
                           r.image->width, r.image->height);
        return;
      }

      if (!attached (r))
      {
        forget (c.index);
        return;
      }

      // The image may no longer want this tier (its picmip may have
      // changed while the read was in flight, see reload()), in which
      // case we have nothing to install. Seems wasteful, but it's rare
      // enough not to bother.
      //
      if (c.tier >= r.resident || c.tier < r.sharpest)
        return;

      HRESULT hr;
      IDirect3DTexture9* x (create_texture (device, r.info, c.tier, c.data.data (), hr));
      if (x == nullptr)
      {
        ++create_failures_;

        if (++r.failures == max_failures)
          Logger::Warning (Game::CON_CHANNEL_GFX,
                           "Couldn't create a texture for '{}' ({:#x}), keeping it at {}x{}\n",
                           image_path (*r.image), static_cast<uint32_t> (hr),
                           r.image->width, r.image->height);
        return;
      }

      uploaded_ += card_size (r.info, c.tier);
      replace (r, x, c.tier);
      ++upgrades_;
    }

    // Decide what to read and what to take down in this frame.
    //
    // First we take down the images that are sharper than IW4 now lets
    // them be. Then we go through the images in use, most recently
    // drawn first, and ask for the sharpest tier that the budget
    // allows, making room by taking idle images down to their coarsest
    // tier, least recently drawn first. Finally, if there is still
    // room, we do the same for the idle images but without making room
    // for them.
    //
    // Why bother with the idle images at all? If the budget allows,
    // then a map that fits ends up the way IW4 would have loaded it and
    // nothing looks blurry when it first comes into view. But we don't
    // let them have all of it: the idle images only get what's left
    // after we keep a tenth of the budget for the images in use.
    // Otherwise a map that just fits would end up with every image as
    // sharp as it gets and the first image that comes into view would
    // have to take another one down first. A tenth seems reasonable,
    // though it's more of a guess than something we measured.
    //
    // With streaming off, every image counts as in use and there is no
    // budget, which takes them all to their sharpest tier.
    //
    void streamer::
    plan (IDirect3DDevice9* device, int now)
    {
      const bool     on      (enabled ());
      const uint64_t limit   (on ? budget (device) : ~0ull);
      const int      hold    (r_stream_hold_time_ != nullptr ? r_stream_hold_time_->current.integer : 3000);

      // Sort the images out. Note that we leave the staged ones alone
      // (see relocate()), along with the ones that have a read in
      // flight.
      //
      vector<uint32_t> wanted;
      vector<uint32_t> idle;
      vector<uint32_t> evictable;

      for (uint32_t i (0); i != records_.size (); ++i)
      {
        record& r (records_[i]);

        if (!r.live () || r.staged || r.pending != no_tier)
          continue;

        if (!attached (r))
        {
          forget (i);
          continue;
        }

        // If the image is sharper than its picmip now allows (see
        // reload()), then take it down right away since that's what
        // r_applyPicmip asked for. Presumably whoever lowered the
        // picmip wants the memory back now and not whenever the budget
        // gets tight.
        //
        if (r.resident < r.sharpest)
        {
          if (upload_left ())
            shrink (device, r, r.sharpest);
          continue;
        }

        const bool in_use (!on || (r.used && now - r.last_used <= hold));

        if (r.resident > r.sharpest && r.failures < max_failures)
          (in_use ? wanted : idle).push_back (i);

        if (!in_use && r.resident < r.coarsest)
          evictable.push_back (i);
      }

      // Most recently drawn first, with the images that were never
      // drawn last. We evict in the opposite order. Plain least
      // recently used is probably not the smartest policy (an image
      // behind the player may well come back into view first), but it
      // seems to work well enough and it's easy to reason about.
      //
      auto recent ([this] (uint32_t x, uint32_t y)
      {
        const record& a (records_[x]);
        const record& b (records_[y]);

        if (a.used != b.used)
          return a.used;

        return a.last_used > b.last_used;
      });

      ranges::sort (wanted, recent);
      ranges::sort (idle, recent);
      ranges::sort (evictable, [&recent] (uint32_t x, uint32_t y) {return recent (y, x);});

      // If we are over the budget (it shrank, say), then take idle
      // images down until we are not.
      //
      size_t next (0);

      if (resident_ + reserved_ > limit)
        evict (device, resident_ + reserved_ - limit, evictable, next);

      // Return the sharpest tier from t on that fits into the limit l
      // and no_tier if nothing does, making room for the sharpest tier
      // if asked to.
      //
      // Note that we don't make room for the tiers in between. Should
      // we? The idea is to take what fits now and upgrade the rest of
      // the way in a later frame, rather than evict for a tier that we
      // would probably replace soon anyway. It does mean an image may
      // get sharper in two steps. Hopefully that's not noticeable.
      //
      auto fit ([this, device, &evictable, &next] (const record& r,
                                                   uint8_t t,
                                                   uint64_t l,
                                                   bool make_room)
      {
        const uint64_t current (card_size (r.info, r.resident));

        for (; t < r.resident; ++t)
        {
          const uint64_t need  (card_size (r.info, t) - current);
          const uint64_t taken (resident_ + reserved_);

          if (taken + need <= l)
            return t;

          if (make_room && t == r.sharpest)
          {
            evict (device, taken + need - l, evictable, next);

            if (resident_ + reserved_ + need <= l)
              return t;
          }
        }

        return no_tier;
      });

      // Ask for the images in use first and then for the idle ones, for
      // as long as the worker can take more.
      //
      for (uint32_t i: wanted)
      {
        if (in_flight_ == max_requests || in_flight_bytes_ >= max_request_bytes)
          return;

        record& r (records_[i]);
        const uint8_t t (fit (r, r.sharpest, limit, true));
        if (t != no_tier)
          issue (r, t);
      }

      // Keep a tenth of the budget for the images in use (see above).
      //
      const uint64_t spare (limit == ~0ull ? limit : limit - limit / 10);

      for (uint32_t i: idle)
      {
        if (in_flight_ == max_requests || in_flight_bytes_ >= max_request_bytes)
          return;

        record& r (records_[i]);
        const uint8_t t (fit (r, r.sharpest, spare, false));
        if (t != no_tier)
          issue (r, t);
      }
    }

    // Ask the worker to read a tier of an image, reserving the memory
    // that the tier will add. We reserve up front so that plan()
    // doesn't promise the same memory twice while the read is in
    // flight.
    //
    bool streamer::
    issue (record& r, uint8_t tier)
    {
      assert (tier < r.resident && r.pending == no_tier);

      request q {static_cast<uint32_t> (&r - records_.data ()),
                 r.generation,
                 tier,
                 image_path (*r.image),
                 r.info.file_size,
                 r.header,
                 r.info.tier_size[tier]};

      r.pending  = tier;
      r.reserved = card_size (r.info, tier) - card_size (r.info, r.resident);

      reserved_        += r.reserved;
      in_flight_       += 1;
      in_flight_bytes_ += q.size;

      {
        lock_guard l (io_mutex_);
        requests_.push_back (move (q));
      }

      io_condition_.notify_one ();
      return true;
    }

    // Take idle images down to their coarsest tier, least recently
    // drawn first, until we have freed the specified number of bytes.
    // Return the bytes freed, which is less than that if we ran out of
    // candidates (next tells where we stopped). The same happens once
    // this frame's uploads are used up since a copy counts as one.
    //
    // Why all the way to the coarsest tier and not just one step down?
    // It frees the most memory per image and the image is idle anyway.
    // If it comes back into view, then it will be asked for again.
    //
    uint64_t streamer::
    evict (IDirect3DDevice9* device,
           uint64_t bytes,
           vector<uint32_t>& candidates,
           size_t& next)
    {
      uint64_t r (0);

      while (r < bytes && next != candidates.size () && upload_left ())
      {
        record& x (records_[candidates[next++]]);

        // The candidate may have changed since plan() picked it (we may
        // have taken it down already, say).
        //
        if (!x.live () || x.pending != no_tier || x.resident >= x.coarsest)
          continue;

        const uint64_t before (card_size (x.info, x.resident));

        if (shrink (device, x, x.coarsest))
          r += before - card_size (x.info, x.coarsest);
      }

      return r;
    }

    // Mark the images that the previous frame bound as in use and start
    // the next frame's buffer.
    //
    // Note that a bind that races with this may get lost (see
    // mark_buffer). That seems harmless since an image that is still in
    // view gets bound again in the next frame.
    //
    void streamer::
    collect_marks (int now)
    {
      const uint32_t f (frame_.fetch_add (1, memory_order_relaxed));

      mark_buffer& b (marks_[f & 1]);
      const uint32_t n (min (b.size.load (memory_order_relaxed), static_cast<uint32_t> (mark_capacity)));

      for (uint32_t i (0); i != n; ++i)
      {
        const Game::GfxImage* p (b.images[i].exchange (nullptr, memory_order_relaxed));
        if (p == nullptr)
          continue;

        if (record* r = find (p))
        {
          r->used      = true;
          r->last_used = now;
        }
      }

      b.size.store (0, memory_order_relaxed);
    }

    // Release the textures that we replaced at least retire_frames ago,
    // all of them if all is true.
    //
    // Why not release them right away? The device keeps its own
    // reference to whatever is bound, so it's not for the device's
    // sake. But the image's texture is a plain member that other
    // threads may read without any locking, and holding on to the old
    // texture for a couple more frames is cheap. Two frames is probably
    // more than enough.
    //
    void streamer::
    retire_textures (bool all)
    {
      const uint32_t f (frame_.load (memory_order_relaxed));

      while (!retired_.empty () &&
             (all || f - retired_.front ().frame >= retire_frames))
      {
        retired_.front ().texture->Release ();
        retired_.pop_front ();
      }
    }

    // Return the video memory budget for the streamed images.
    //
    // Unless r_streamBudget gives us one, we take half of what the
    // device says is available the first time we are asked (and again
    // after a device reset), which leaves the rest to everything else
    // that IW4 puts into video memory. Half is a guess, but it seems to
    // leave enough room in practice. We go with at least 256MB.
    //
    // Could we do better than a guess? Perhaps by asking DXGI for the
    // actual budget, but that would drag in another API for what is a
    // rough number anyway. Note also that the device rounds this to the
    // nearest MB and that some report less than they have (Direct3D 9
    // tops out at 4GB, for one).
    //
    uint64_t streamer::
    budget (IDirect3DDevice9* device)
    {
      if (r_stream_budget_ != nullptr && r_stream_budget_->current.integer != 0)
        return static_cast<uint64_t> (r_stream_budget_->current.integer) << 20;

      if (auto_budget_ == 0)
      {
        auto_budget_ = max (static_cast<uint64_t> (device->GetAvailableTextureMem ()) / 2,
                            256ull << 20);

        if (!announced_)
        {
          announced_ = true;
          Logger::Print (Game::CON_CHANNEL_GFX,
                         "Streaming textures with a budget of {} MB\n",
                         auto_budget_ >> 20);
        }
      }

      return auto_budget_;
    }

    // Start the worker unless it's already running. We start it with
    // the first image we stream rather than at startup since before
    // that there is nothing for it to do.
    //
    void streamer::
    start_worker ()
    {
      call_once (worker_started_, [this] ()
      {
        worker_ = jthread ([this] (stop_token t) {work (t);});
      });
    }

    // Read the requested tiers, one at a time, for as long as we aren't
    // asked to stop. Runs on the worker thread.
    //
    // Note that a failed read still produces a completion (with empty
    // data) so that apply() gives back what the request reserved.
    //
    void streamer::
    work (stop_token t)
    {
      for (;;)
      {
        request q;
        {
          unique_lock l (io_mutex_);
          if (!io_condition_.wait (l, t, [this] {return !requests_.empty ();}))
            return;

          q = move (requests_.front ());
          requests_.pop_front ();
        }

        optional<vector<uint8_t>> d (read (q));

        lock_guard l (io_mutex_);
        completions_.push_back (completion {q.index,
                                            q.generation,
                                            q.tier,
                                            q.size,
                                            d ? move (*d) : vector<uint8_t> ()});
      }
    }

    // Read a tier from the file. Return nullopt if the file can't be
    // read and also if it's not the one we loaded the image from
    // (anymore).
    //
    // We read through the database thread's file handle. IW4 only uses
    // that handle to read images (see the readers that Load_Texture()
    // and DB_LoadDelayedImages() pass to
    // Image_LoadFromFileWithReader()) and the database thread has
    // exactly one (see FS_HandleForFile()). Both of those run with
    // FileSystem::FSMutex held, so we hold it for as long as we have
    // the file open. This also keeps the file system from restarting
    // under us (see FileSystem::FsRestartSync()).
    //
    // Couldn't we have a handle for the worker instead? Probably, but
    // IW4 hands out its handles per thread from fixed slots and we
    // would rather not go adding slots to its file system just for
    // this. Sharing the database thread's handle means the worker waits
    // while a map loads, which is when we don't want it reading anyway.
    //
    // Note that the file system may have restarted between the request
    // and the read with another iwd now providing the file. So we make
    // sure the file has the same size and header as the one we loaded
    // and, if not, fail the read and leave the image as it is.
    //
    optional<vector<uint8_t>> streamer::
    read (const request& q)
    {
      lock_guard g (FileSystem::FSMutex);

      Game::fileHandle_t h (0);
      const int n (Game::FS_FOpenFileReadForThread (q.path.c_str (), &h, Game::FS_THREAD_DATABASE));
      if (n < 0 || h == 0)
        return nullopt;

      optional<vector<uint8_t>> r;

      Game::GfxImageFileHeader header;
      if (static_cast<uint32_t> (n) == q.file_size &&
          Game::FS_Read (&header, static_cast<int> (header_size), h) == static_cast<int> (header_size) &&
          memcmp (&header, &q.header, header_size) == 0)
      {
        vector<uint8_t> d (q.size);
        if (Game::FS_Read (d.data (), static_cast<int> (d.size ()), h) == static_cast<int> (d.size ()))
          r = move (d);
      }

      Game::FS_FCloseFile (h);
      return r;
    }

    // Image_LoadFromFileWithReader(). Runs on the database thread for
    // the images of a zone (the delayed ones included) and, as far as
    // we can tell, on the render thread when recovering from a lost
    // device.
    //
    bool
    image_load_hook (Game::GfxImage* image, Game::Reader_t reader)
    {
      return s.load (image, reader) || image_load_original (image, reader);
    }

    // Image_Release(). Runs on the thread that unloads the image's
    // zone.
    //
    void
    image_release_hook (Game::GfxImage* image)
    {
      s.release (image);
      image_release_original (image);
    }

    // Image_Reload(). Runs on the main thread when r_applyPicmip
    // applies a new picmip.
    //
    void
    image_reload_hook (Game::GfxImage* image)
    {
      if (!s.reload (image))
        image_reload_original (image);
    }

    // R_SetSampler(). Runs on the render thread for every image that a
    // material binds.
    //
    void
    r_set_sampler_hook (Game::GfxCmdBufContext c,
                        unsigned int sampler,
                        unsigned char state,
                        const Game::GfxImage* image)
    {
      if (image != nullptr)
      {
        s.mark (c, image);
        image = s.substitute (image);
      }

      r_set_sampler_original (c, sampler, state, image);
    }
  }

  streaming::
  streaming ()
  {
    // A dedicated server doesn't render anything, so there is nothing
    // to stream.
    //
    if (Dedicated::IsEnabled ())
      return;

    // The IW4 members that we use, so that we find out at compile time
    // if we got any of them wrong rather than with a crash in the
    // middle of a map.
    //
    AssertOffset (Game::GfxImage, texture,          0x00);
    AssertOffset (Game::GfxImage, mapType,          0x08);
    AssertOffset (Game::GfxImage, semantic,         0x09);
    AssertOffset (Game::GfxImage, category,         0x0A);
    AssertOffset (Game::GfxImage, picmip,           0x0C);
    AssertOffset (Game::GfxImage, noPicmip,         0x0E);
    AssertOffset (Game::GfxImage, cardMemory,       0x10);
    AssertOffset (Game::GfxImage, width,            0x18);
    AssertOffset (Game::GfxImage, height,           0x1A);
    AssertOffset (Game::GfxImage, depth,            0x1C);
    AssertOffset (Game::GfxImage, name,             0x20);
    AssertOffset (Game::GfxCmdBufState, samplerTexture, 0x50);
    AssertOffset (Game::GfxImageFileHeader, flags,             0x04);
    AssertOffset (Game::GfxImageFileHeader, format,            0x08);
    AssertOffset (Game::GfxImageFileHeader, dimensions,        0x0A);
    AssertOffset (Game::GfxImageFileHeader, fileSizeForPicmip, 0x10);
    AssertSize   (Game::GfxImageFileHeader, 32);

    // The sizes are those of the instructions that each detour
    // overwrites at the start of the function.
    //
    image_load_original    = Utils::Hook::Detour (image_load_from_file_with_reader, 5, &image_load_hook);
    image_release_original = Utils::Hook::Detour (image_release,                    6, &image_release_hook);
    image_reload_original  = Utils::Hook::Detour (image_reload,                     6, &image_reload_hook);
    r_set_sampler_original = Utils::Hook::Detour (r_set_sampler,                    5, &r_set_sampler_hook);

    // Follow the images into the database pool (see relocate()).
    //
    AssetHandler::OnAdd ([] (Game::XAssetType type, Game::XAssetHeader loaded, Game::XAssetHeader added)
    {
      if (type == Game::ASSET_TYPE_IMAGE)
        s.relocate (loaded.image, added.image);
    });

    Renderer::OnBackendFrame ([] (IDirect3DDevice9* d) {s.frame (d);});
    Renderer::OnDeviceRecoveryBegin ([] () {s.suspend ();});
    Renderer::OnDeviceRecoveryEnd ([] () {s.resume ();});
    Scheduler::OnGameShutdown ([] () {s.shutdown ();});

    Events::OnDvarInit ([] () {s.register_dvars ();});
    Command::Add ("r_streamInfo", [] () {s.print_info ();});
  }
}
