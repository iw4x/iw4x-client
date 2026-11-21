#include "Sound.hpp"

namespace Components
{
  namespace
  {
    constexpr std::int8_t StreamFormatMp3 = 5;

    // The length of the stream's file, which the stream thread keeps in the
    // stream after it opens the file.
    //
    constexpr std::size_t StreamFileLength = 0x84;

    const auto SND_ParseStreamHeader =
      reinterpret_cast<bool (*) (void*, const std::uint8_t*, std::uint32_t)> (0x1402BAF40);

    // The MPEG sample rates by version (2.5, reserved, 2, 1) and index.
    //
    constexpr int SampleRates[4][3] {
      {11025, 12000, 8000}, {0, 0, 0}, {22050, 24000, 16000}, {44100, 48000, 32000}};

    std::uint32_t
    ReadBigEndian32 (const std::uint8_t* p)
    {
      return std::uint32_t (p[0]) << 24 | std::uint32_t (p[1]) << 16 | std::uint32_t (p[2]) << 8 | p[3];
    }

    std::uint32_t
    ReadBigEndian16 (const std::uint8_t* p)
    {
      return std::uint32_t (p[0]) << 8 | p[1];
    }

    std::uint32_t
    Id3Size (const std::uint8_t* p)
    {
      return ((p[6] & 0x7F) << 21 | (p[7] & 0x7F) << 14 | (p[8] & 0x7F) << 7 | (p[9] & 0x7F)) + 10;
    }

    // Return true if the game would divide by zero over the stream. It takes
    // a stream that isn't a RIFF file for an MP3 with its own header, or with
    // an ID3v2.3 tag that carries the sample count, and divides by the length
    // it computes from that count in milliseconds. A plain MP3 has neither.
    //
    bool
    WouldDivideByZero (const std::uint8_t* data, std::uint32_t size)
    {
      constexpr std::uint32_t riff (0x46464952);
      constexpr std::uint32_t id3v23 (0x03334449);

      if (size < 36)
        return true;

      std::uint32_t magic;
      std::memcpy (&magic, data, sizeof (magic));

      if (magic == riff)
        return false;

      std::uint32_t offset (0);
      auto samples (static_cast<std::int32_t> (ReadBigEndian32 (data + 24)) -
                    static_cast<std::int32_t> (ReadBigEndian16 (data + 32)) -
                    static_cast<std::int32_t> (ReadBigEndian16 (data + 30)));

      if (magic == id3v23)
      {
        offset = Id3Size (data);

        const auto tag (ReadBigEndian32 (data + 20));

        if (tag < 0x80000000)
          samples = static_cast<std::int32_t> (tag);
        else if (tag != 0x80000001)
          samples = 0;
      }

      if (offset + 3 >= size)
        return true;

      const auto frame (ReadBigEndian32 (data + offset) & 0xFFFFFF);
      const auto version ((frame >> 19) & 3);
      const auto index ((frame >> 10) & 3);

      // Note that the game reads -1 past the end of its rates.
      //
      const std::int64_t rate (index < 3 && version != 1 ? SampleRates[version][index] : -1);

      return 1000LL * samples / rate == 0;
    }

    struct Mp3Frame
    {
      std::uint32_t offset;
      int rate;
      int channels;
      int samplesPerFrame;
      int bitrate;
      int length;
      bool mpeg1;
    };

    bool
    ReadFrame (const std::uint8_t* data, std::uint32_t size, std::uint32_t offset, Mp3Frame& f)
    {
      // The bitrates in kbit/s by MPEG 2 or 1, layer, and index.
      //
      constexpr int bitrates[2][3][15] {
        {{0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256},
         {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160},
         {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160}},
        {{0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448},
         {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384},
         {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320}}};

      if (offset + 4 > size)
        return false;

      const auto h (ReadBigEndian32 (data + offset));

      if ((h & 0xFFE00000) != 0xFFE00000)
        return false;

      const auto version ((h >> 19) & 3);
      const auto layerBits ((h >> 17) & 3);
      const auto bitrateIndex ((h >> 12) & 0xF);
      const auto rateIndex ((h >> 10) & 3);

      if (version == 1 || layerBits == 0 || bitrateIndex == 0 || bitrateIndex == 15 || rateIndex == 3)
        return false;

      const int layer (4 - static_cast<int> (layerBits));
      const int padding ((h >> 9) & 1);

      f.offset = offset;
      f.mpeg1 = version == 3;
      f.rate = SampleRates[version][rateIndex];
      f.channels = ((h >> 6) & 3) == 3 ? 1 : 2;
      f.bitrate = bitrates[f.mpeg1 ? 1 : 0][layer - 1][bitrateIndex] * 1000;

      if (layer == 1)
      {
        f.samplesPerFrame = 384;
        f.length = (12 * f.bitrate / f.rate + padding) * 4;
      }
      else
      {
        f.samplesPerFrame = layer == 3 && !f.mpeg1 ? 576 : 1152;
        f.length = f.samplesPerFrame / 8 * f.bitrate / f.rate + padding;
      }

      return f.length > 4;
    }

    // Find the first frame past any ID3 tag. We only take a frame where the
    // next one agrees with it, since a frame sync can turn up in other data.
    //
    bool
    FindFirstFrame (const std::uint8_t* data, std::uint32_t size, Mp3Frame& f)
    {
      std::uint32_t start (0);

      if (size >= 10 && data[0] == 'I' && data[1] == 'D' && data[2] == '3')
        start = Id3Size (data) + (data[5] & 0x10 ? 10 : 0);

      for (auto offset (start); offset < size && offset < start + 0x10000; ++offset)
      {
        if (!ReadFrame (data, size, offset, f))
          continue;

        const auto next (offset + static_cast<std::uint32_t> (f.length));

        if (next + 4 > size)
          return true;

        Mp3Frame n;
        if (ReadFrame (data, size, next, n) && n.rate == f.rate && n.mpeg1 == f.mpeg1)
          return true;
      }

      return false;
    }

    // Count the samples from a Xing or VBRI header if the encoder wrote one,
    // and otherwise from the length at the first frame's bitrate.
    //
    std::int64_t
    CountSamples (const std::uint8_t* data, std::uint32_t size, const Mp3Frame& f, std::int64_t length)
    {
      const int side (f.channels == 1 ? (f.mpeg1 ? 17 : 9) : (f.mpeg1 ? 32 : 17));
      const auto xing (f.offset + 4 + side);

      if (xing + 12 <= size &&
          (!std::memcmp (data + xing, "Xing", 4) || !std::memcmp (data + xing, "Info", 4)) &&
          (ReadBigEndian32 (data + xing + 4) & 1))
        return static_cast<std::int64_t> (ReadBigEndian32 (data + xing + 8)) * f.samplesPerFrame;

      const auto vbri (f.offset + 36);

      if (vbri + 18 <= size && !std::memcmp (data + vbri, "VBRI", 4))
        return static_cast<std::int64_t> (ReadBigEndian32 (data + vbri + 14)) * f.samplesPerFrame;

      auto bytes (length - f.offset);
      if (bytes <= 0)
        bytes = static_cast<std::int64_t> (size) - f.offset;

      return bytes * 8 * f.rate / f.bitrate;
    }

    // Fill in the header the way the game does for an MP3 with its own
    // header, having read the magic (readPosition 8). We compute the
    // average bytes as the game does.
    //
    bool
    FillMp3Header (Game::StreamHeader* h, const std::uint8_t* data, std::uint32_t size, std::int64_t length)
    {
      Mp3Frame f;
      if (!FindFirstFrame (data, size, f))
        return false;

      const auto samples (CountSamples (data, size, f, length));

      if (samples <= 0 || samples > std::numeric_limits<std::int32_t>::max () || 1000 * samples / f.rate == 0)
        return false;

      *h = {};
      h->data = data;
      h->size = size;
      h->readPosition = 8;
      h->dataOffset = f.offset;
      h->rate = f.rate;
      h->dataSize = static_cast<std::int32_t> (size - f.offset);
      h->samples = static_cast<std::int32_t> (samples);
      h->averageBytes = static_cast<std::int32_t> (2 * samples / (1000 * samples / f.rate) / 1000);
      h->channels = static_cast<std::uint16_t> (f.channels);
      h->bits = 16;
      h->blockAlign = static_cast<std::uint16_t> (2 * f.channels);
      h->format = StreamFormatMp3;

      return true;
    }
  }

  Sound::
  Sound ()
  {
    // Note that on x86 we work around a few bugs in the sound and voice
    // initialization, none of which x64 has:
    //
    // The x86 game initializes the sound driver (Miles over DirectSound)
    // before it creates its window, which leaves DirectSound bound to the
    // wrong window, or to none, so we initialize it after the renderer.
    // x64 plays sound through XAudio2, which isn't bound to a window, and
    // binds the DirectSound of voice chat to the desktop window.
    //
    // We also patch a double release of the DirectSound interface in Miles,
    // which x64 doesn't use.
    //
    // Finally we fix the creation of the voice playback buffers, which x86
    // starts without checking that DirectSound is initialized, with a
    // clobbered argument, and releases uninitialized on failure, and whose
    // failure poisons the buffer array with null pointers. x64 checks that
    // DirectSound is initialized, creates the buffer inline, releases it only
    // if it was created, and skips the null buffers wherever it uses them.

    // Read the header of a plain MP3 stream ourselves.
    //
    // Note that x86 streams through Miles, which reads plain MP3s, while x64
    // reads the header with SND_ParseStreamHeader(), which only knows the
    // game's own MP3s and divides by zero on a plain one. IW4x's sounds are
    // plain MP3s, which are also what an x86 server may have us download.
    //
    // We replace the call in the stream thread's open, which passes the
    // header, the stream's first data and its size. See Sound.asm for how we
    // pass the stream itself, which holds the length of its file.
    //
    Utils::Hook (0x1402C2D99, Sound_ParseStreamHeaderStub, HOOK_CALL).install ()->quick ();
  }
}

bool
Sound_ParseStreamHeader (void* header, const std::uint8_t* data, std::uint32_t size, const std::uint8_t* stream)
{
  using namespace Components;

  if (!WouldDivideByZero (data, size))
    return SND_ParseStreamHeader (header, data, size);

  std::int32_t length;
  std::memcpy (&length, stream + StreamFileLength, sizeof (length));

  if (FillMp3Header (static_cast<Game::StreamHeader*> (header), data, size, length))
    return true;

  static bool reported (false);
  if (!std::exchange (reported, true))
    Logger::Warning (Game::CON_CHANNEL_SOUND, "Skipping a streamed sound with no MP3 frame we can read\n");

  return false;
}
