#include "Audio.hpp"

#include "../Types.hpp"

#include <cmath>
#include <cstdio>
#include <algorithm>

#include <cfgmgr32.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>
#include <avrt.h>

#pragma comment(lib, "Cfgmgr32.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Avrt.lib")

namespace Controller
{
  namespace transport
  {
    namespace
    {
      constexpr size_t haptic_channel_left {2};
      constexpr size_t haptic_channel_right {3};
      constexpr size_t required_channels {4};

      constexpr REFERENCE_TIME shared_buffer_duration {100000};

      constexpr DWORD wait_timeout {50};

      const DEVPROPKEY key_container_id
      {
        {0x8c7ed206, 0x3f8a, 0x4827, {0xb3, 0xab, 0xae, 0x9e, 0x1f, 0xae, 0xfc, 0x6c}},
        2
      };

      static_assert (sizeof (DEVPROPKEY) == sizeof (PROPERTYKEY),
                     "DEVPROPKEY and PROPERTYKEY are the same key in two spellings");

      const PROPERTYKEY&
      as_property_key (const DEVPROPKEY& k) noexcept
      {
        return reinterpret_cast<const PROPERTYKEY&> (k);
      }

      const PROPERTYKEY key_device_format
      {
        {0xf19f064d, 0x082c, 0x4e27, {0xbc, 0x73, 0x68, 0x82, 0xa1, 0xbb, 0x8e, 0x4c}},
        0
      };

      template <typename T>
      class com_ptr
      {
      public:
        com_ptr () = default;

        ~com_ptr () {reset ();}

        com_ptr (const com_ptr&) = delete;
        com_ptr& operator= (const com_ptr&) = delete;

        T**
        put () noexcept {reset (); return &p_;}

        T*
        get () const noexcept {return p_;}

        T*
        operator-> () const noexcept {return p_;}

        explicit
        operator bool () const noexcept {return p_ != nullptr;}

        void
        reset () noexcept
        {
          if (p_ != nullptr)
          {
            p_->Release ();
            p_ = nullptr;
          }
        }

      private:
        T* p_ {nullptr};
      };

      class property
      {
      public:
        property () noexcept {PropVariantInit (&v_);}

        ~property () {PropVariantClear (&v_);}

        property (const property&) = delete;
        property& operator= (const property&) = delete;

        PROPVARIANT*
        put () noexcept {return &v_;}

        const PROPVARIANT&
        get () const noexcept {return v_;}

      private:
        PROPVARIANT v_ {};
      };

      std::string
      narrow (const wchar_t* w)
      {
        if (w == nullptr)
          return {};

        const int n (WideCharToMultiByte (CP_UTF8, 0, w, -1,
                                          nullptr, 0, nullptr, nullptr));

        if (n <= 1)
          return {};

        std::string s (static_cast<size_t> (n - 1), '\0');
        WideCharToMultiByte (CP_UTF8, 0, w, -1, s.data (), n, nullptr, nullptr);
        return s;
      }

      std::optional<GUID>
      container_of (const std::wstring& interface_path)
      {
        GUID container {};
        ULONG size (sizeof (container));
        DEVPROPTYPE type (0);

        const CONFIGRET r (
          CM_Get_Device_Interface_PropertyW (interface_path.c_str (),
                                             &key_container_id,
                                             &type,
                                             reinterpret_cast<PBYTE> (&container),
                                             &size,
                                             0));

        if (r != CR_SUCCESS || type != DEVPROP_TYPE_GUID)
          return std::nullopt;

        return container;
      }

      std::optional<GUID>
      container_of (IMMDevice& device)
      {
        com_ptr<IPropertyStore> props;

        if (FAILED (device.OpenPropertyStore (STGM_READ, props.put ())))
          return std::nullopt;

        property v;

        if (FAILED (props->GetValue (as_property_key (key_container_id), v.put ())) ||
            v.get ().vt != VT_CLSID ||
            v.get ().puuid == nullptr)
          return std::nullopt;

        return *v.get ().puuid;
      }

      std::string
      friendly_name (IMMDevice& device)
      {
        com_ptr<IPropertyStore> props;

        if (FAILED (device.OpenPropertyStore (STGM_READ, props.put ())))
          return {};

        property v;

        if (FAILED (props->GetValue (PKEY_Device_FriendlyName, v.put ())) ||
            v.get ().vt != VT_LPWSTR)
          return {};

        return narrow (v.get ().pwszVal);
      }

      bool
      find_endpoint (const std::wstring& hid_path,
                     com_ptr<IMMDevice>& out,
                     std::string& name,
                     std::vector<std::string>& seen)
      {
        const std::optional<GUID> wanted (container_of (hid_path));

        if (!wanted)
          return false;

        com_ptr<IMMDeviceEnumerator> devices;

        if (FAILED (CoCreateInstance (__uuidof (MMDeviceEnumerator),
                                      nullptr,
                                      CLSCTX_ALL,
                                      __uuidof (IMMDeviceEnumerator),
                                      reinterpret_cast<void**> (devices.put ()))))
          return false;

        com_ptr<IMMDeviceCollection> endpoints;

        if (FAILED (devices->EnumAudioEndpoints (eRender,
                                                 DEVICE_STATE_ACTIVE,
                                                 endpoints.put ())))
          return false;

        UINT count (0);

        if (FAILED (endpoints->GetCount (&count)))
          return false;

        for (UINT i (0); i != count; ++i)
        {
          com_ptr<IMMDevice> candidate;

          if (FAILED (endpoints->Item (i, candidate.put ())))
            continue;

          const std::optional<GUID> theirs (container_of (*candidate.get ()));

          if (theirs && IsEqualGUID (*theirs, *wanted))
          {
            name = friendly_name (*candidate.get ());

            return SUCCEEDED (endpoints->Item (i, out.put ()));
          }

          seen.push_back (friendly_name (*candidate.get ()));
        }

        return false;
      }

      bool
      declared_format (IMMDevice& device, WAVEFORMATEXTENSIBLE& out)
      {
        com_ptr<IPropertyStore> props;

        if (FAILED (device.OpenPropertyStore (STGM_READ, props.put ())))
          return false;

        property v;

        if (FAILED (props->GetValue (key_device_format, v.put ())) ||
            v.get ().vt != VT_BLOB ||
            v.get ().blob.pBlobData == nullptr ||
            v.get ().blob.cbSize < sizeof (WAVEFORMATEX))
          return false;

        const auto* f (
          reinterpret_cast<const WAVEFORMATEX*> (v.get ().blob.pBlobData));

        const size_t want (f->wFormatTag == WAVE_FORMAT_EXTENSIBLE
                           ? sizeof (WAVEFORMATEXTENSIBLE)
                           : sizeof (WAVEFORMATEX));

        if (v.get ().blob.cbSize < want)
          return false;

        std::memset (&out, 0, sizeof (out));
        std::memcpy (&out, v.get ().blob.pBlobData, want);
        return true;
      }

      bool
      writable (const WAVEFORMATEX& f, bool& is_float) noexcept
      {
        if (f.nChannels < required_channels)
          return false;

        if (f.wFormatTag == WAVE_FORMAT_IEEE_FLOAT && f.wBitsPerSample == 32)
        {
          is_float = true;
          return true;
        }

        if (f.wFormatTag == WAVE_FORMAT_PCM && f.wBitsPerSample == 16)
        {
          is_float = false;
          return true;
        }

        if (f.wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
            f.cbSize >= sizeof (WAVEFORMATEXTENSIBLE) - sizeof (WAVEFORMATEX))
        {
          const auto& e (reinterpret_cast<const WAVEFORMATEXTENSIBLE&> (f));

          if (IsEqualGUID (e.SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) &&
              f.wBitsPerSample == 32)
          {
            is_float = true;
            return true;
          }

          if (IsEqualGUID (e.SubFormat, KSDATAFORMAT_SUBTYPE_PCM) &&
              f.wBitsPerSample == 16)
          {
            is_float = false;
            return true;
          }
        }

        return false;
      }

      int16_t
      quantize (float v) noexcept
      {
        return static_cast<int16_t> (
          std::lround (std::clamp (v, -1.0f, 1.0f) * 32767.0f));
      }

      std::string
      describe (const WAVEFORMATEX& f, bool exclusive)
      {
        return std::to_string (f.nChannels) + " channels, " +
               std::to_string (f.nSamplesPerSec) + " Hz, " +
               std::to_string (f.wBitsPerSample) + "-bit, " +
               (exclusive ? "exclusive" : "shared");
      }

      class stream
      {
      public:
        stream () = default;

        ~stream ()
        {
          if (ready != nullptr)
            CloseHandle (ready);
        }

        stream (const stream&) = delete;
        stream& operator= (const stream&) = delete;

        com_ptr<IAudioClient> client;
        com_ptr<IAudioRenderClient> render;

        HANDLE ready {nullptr};
        UINT32 buffer_frames {0};
        uint32_t rate {0};
        size_t channels {0};
        bool exclusive {false};
        bool is_float {false};

        std::string described;
      };

      HRESULT
      initialize (IMMDevice& device, stream& s)
      {
        REFERENCE_TIME default_period (0);
        REFERENCE_TIME minimum_period (0);
        s.client->GetDevicePeriod (&default_period, &minimum_period);

        WAVEFORMATEXTENSIBLE declared {};

        if (declared_format (device, declared) &&
            writable (declared.Format, s.is_float))
        {
          HRESULT hr (s.client->Initialize (AUDCLNT_SHAREMODE_EXCLUSIVE,
                                            AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                            default_period,
                                            default_period,
                                            &declared.Format,
                                            nullptr));

          if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED)
          {
            UINT32 frames (0);

            if (SUCCEEDED (s.client->GetBufferSize (&frames)) && frames != 0)
            {
              const REFERENCE_TIME aligned (
                static_cast<REFERENCE_TIME> (
                  10000.0 * 1000 * frames / declared.Format.nSamplesPerSec + 0.5));

              s.client.reset ();

              hr = device.Activate (__uuidof (IAudioClient),
                                    CLSCTX_ALL,
                                    nullptr,
                                    reinterpret_cast<void**> (s.client.put ()));

              if (SUCCEEDED (hr))
                hr = s.client->Initialize (AUDCLNT_SHAREMODE_EXCLUSIVE,
                                           AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                           aligned,
                                           aligned,
                                           &declared.Format,
                                           nullptr);
            }
          }

          if (SUCCEEDED (hr))
          {
            s.exclusive = true;
            s.rate = declared.Format.nSamplesPerSec;
            s.channels = declared.Format.nChannels;
            s.described = describe (declared.Format, true);
            return hr;
          }
        }

        WAVEFORMATEX* mix (nullptr);

        if (FAILED (s.client->GetMixFormat (&mix)) || mix == nullptr)
          return AUDCLNT_E_UNSUPPORTED_FORMAT;

        HRESULT hr (AUDCLNT_E_UNSUPPORTED_FORMAT);

        if (writable (*mix, s.is_float))
        {
          hr = s.client->Initialize (AUDCLNT_SHAREMODE_SHARED,
                                     AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                     shared_buffer_duration,
                                     0,
                                     mix,
                                     nullptr);

          if (SUCCEEDED (hr))
          {
            s.exclusive = false;
            s.rate = mix->nSamplesPerSec;
            s.channels = mix->nChannels;
            s.described = describe (*mix, false);
          }
        }

        CoTaskMemFree (mix);
        return hr;
      }

      bool
      place (stream& s, UINT32 count, std::span<const haptic::frame> frames) noexcept
      {
        BYTE* data (nullptr);

        if (FAILED (s.render->GetBuffer (count, &data)) || data == nullptr)
          return false;

        const size_t samples (static_cast<size_t> (count) * s.channels);

        if (s.is_float)
        {
          auto* out (reinterpret_cast<float*> (data));
          std::fill_n (out, samples, 0.0f);

          for (UINT32 i (0); i != count; ++i)
          {
            out[i * s.channels + haptic_channel_left] = frames[i].left;
            out[i * s.channels + haptic_channel_right] = frames[i].right;
          }
        }
        else
        {
          auto* out (reinterpret_cast<int16_t*> (data));
          std::fill_n (out, samples, int16_t {0});

          for (UINT32 i (0); i != count; ++i)
          {
            out[i * s.channels + haptic_channel_left] = quantize (frames[i].left);
            out[i * s.channels + haptic_channel_right] = quantize (frames[i].right);
          }
        }

        return SUCCEEDED (s.render->ReleaseBuffer (count, 0));
      }

      bool
      prepare (stream& s)
      {
        s.ready = CreateEventW (nullptr, FALSE, FALSE, nullptr);

        if (s.ready == nullptr)
          return false;

        return SUCCEEDED (s.client->SetEventHandle (s.ready)) &&
               SUCCEEDED (s.client->GetBufferSize (&s.buffer_frames)) &&
               SUCCEEDED (s.client->GetService (
                 __uuidof (IAudioRenderClient),
                 reinterpret_cast<void**> (s.render.put ())));
      }

      void
      render_loop (stream& s,
                   const audio_endpoint::source& fill,
                   std::atomic<bool>& running,
                   const std::stop_token& stop) noexcept
      {
        std::vector<haptic::frame> frames (s.buffer_frames);

        const auto block = [&] (UINT32 count) noexcept
        {
          const std::span<haptic::frame> b (frames.data (), count);

          std::fill (b.begin (), b.end (), haptic::frame {});
          fill (b);

          return place (s, count, b);
        };

        if (!block (s.buffer_frames) || FAILED (s.client->Start ()))
          return;

        running.store (true, std::memory_order_release);

        while (!stop.stop_requested ())
        {
          if (WaitForSingleObject (s.ready, wait_timeout) != WAIT_OBJECT_0)
            break;

          UINT32 free (s.buffer_frames);

          if (!s.exclusive)
          {
            UINT32 padding (0);

            if (FAILED (s.client->GetCurrentPadding (&padding)))
              break;

            free = s.buffer_frames - padding;
          }

          if (free != 0 && !block (free))
            break;
        }

        running.store (false, std::memory_order_release);
        s.client->Stop ();
      }
    }

    audio_endpoint::
    audio_endpoint (const context& ctx, device_id device,
                    const std::wstring& hid_path, source s)
      : source_ (std::move (s)),
        thread_ ([this, ctx, device, path = hid_path] (std::stop_token t)
                 {run (std::move (t), ctx, device, std::move (path));})
    {
    }

    std::string
    audio_endpoint::
    status () const
    {
      std::lock_guard lock (status_mutex_);
      return status_;
    }

    void
    audio_endpoint::
    note (std::string s) const
    {
      std::lock_guard lock (status_mutex_);
      status_ = std::move (s);
    }

    void
    audio_endpoint::
    run (std::stop_token stop,
         context ctx,
         device_id device,
         std::wstring hid_path) noexcept
    {
      if (FAILED (CoInitializeEx (nullptr, COINIT_MULTITHREADED)))
      {
        note ("COM could not be initialized for the audio thread");
        return;
      }

      {
        com_ptr<IMMDevice> endpoint;
        std::string name;
        std::vector<std::string> seen;

        if (!find_endpoint (hid_path, endpoint, name, seen))
        {
          note ("no audio endpoint belongs to this controller");

          std::string of (seen.empty ()
                          ? "no active render endpoints were found at all"
                          : std::to_string (seen.size ()) +
                            " active render endpoint(s) were found, none of them "
                            "this controller's:");

          for (const std::string& s: seen)
            of += " '" + s + "'";

          ctx.report (severity::info, facility::transport, errc::none, device,
                      "the controller exposes no audio endpoint, so haptics fall "
                      "back to the HID rumble path; " + of);

          CoUninitialize ();
          return;
        }

        stream s;

        if (FAILED (endpoint->Activate (__uuidof (IAudioClient),
                                      CLSCTX_ALL,
                                      nullptr,
                                      reinterpret_cast<void**> (s.client.put ()))))
        {
          note ("audio endpoint '" + name + "' could not be activated");
          CoUninitialize ();
          return;
        }

        if (FAILED (initialize (*endpoint.get (), s)) || !prepare (s))
        {
          note ("audio endpoint '" + name + "' could not be opened for four-channel "
                "rendering");

          ctx.report (severity::warning, facility::transport, errc::transport_failure,
                      "the controller's audio endpoint could not be opened for the "
                      "four channels its actuators sit on, either because it offers "
                      "no such format or because another application holds it; "
                      "haptics fall back to the HID rumble path");

          CoUninitialize ();
          return;
        }

        note ("haptics on '" + name + "' (" + s.described + ")");

        ctx.report (severity::info, facility::transport, errc::none,
                    "controller haptics streaming to '" + name + "' (" +
                    s.described + ")");

        rate_.store (s.rate, std::memory_order_release);

        DWORD task_index (0);
        HANDLE task (AvSetMmThreadCharacteristicsW (L"Pro Audio", &task_index));

        render_loop (s, source_, running_, stop);

        if (task != nullptr)
          AvRevertMmThreadCharacteristics (task);

        if (!stop.stop_requested ())
          note ("haptics stream on '" + name + "' ended");
      }

      CoUninitialize ();
    }
  }
}
