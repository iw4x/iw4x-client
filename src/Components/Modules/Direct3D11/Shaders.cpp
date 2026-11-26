#include "Shaders.hpp"

#include "Device.hpp"
#include "Formats.hpp"

#include <algorithm> // ranges::lower_bound(), ranges::is_sorted(), ranges::find_if()
#include <cstring>   // memcmp(), memcpy()
#include <set>

using namespace std;

namespace Components
{
  namespace d3d11
  {
    // shader_pack
    //
    shader_pack::
    ~shader_pack ()
    {
      if (view_ != nullptr)
        UnmapViewOfFile (view_);

      if (mapping_ != nullptr)
        CloseHandle (mapping_);

      if (file_ != INVALID_HANDLE_VALUE)
        CloseHandle (file_);
    }

    bool shader_pack::
    open (const filesystem::path& p)
    {
      auto fail ([&p] (const char* why)
      {
        Logger::Warning (Game::CON_CHANNEL_GFX,
                         "Couldn't load the shader pack '{}': {}\n",
                         p.string (),
                         why);
        return false;
      });

      file_ = CreateFileW (p.c_str (),
                           GENERIC_READ,
                           FILE_SHARE_READ,
                           nullptr,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           nullptr);
      if (file_ == INVALID_HANDLE_VALUE)
        return fail ("no such file");

      LARGE_INTEGER n;
      if (!GetFileSizeEx (file_, &n) || n.QuadPart < 16)
        return fail ("file too small");

      bytes_ = static_cast<size_t> (n.QuadPart);

      mapping_ = CreateFileMappingW (file_, nullptr, PAGE_READONLY, 0, 0, nullptr);
      if (mapping_ == nullptr)
        return fail ("unable to map the file");

      view_ = static_cast<const std::byte*> (MapViewOfFile (mapping_, FILE_MAP_READ, 0, 0, 0));
      if (view_ == nullptr)
        return fail ("unable to map the file");

      if (memcmp (view_, "DXBCPACK", 8) != 0)
        return fail ("not a shader pack");

      uint32_t version, count;
      memcpy (&version, view_ + 8, 4);
      memcpy (&count, view_ + 12, 4);

      if (version != 1)
        return fail ("unsupported version");

      if (16 + static_cast<size_t> (count) * sizeof (entry) > bytes_)
        return fail ("truncated table");

      entries_ = reinterpret_cast<const entry*> (view_ + 16);
      count_   = count;

      const span<const entry> es (entries_, count_);

      if (!ranges::is_sorted (es, {}, &entry::key))
        return fail ("unsorted table");

      for (const entry& e: es)
      {
        if (static_cast<size_t> (e.offset) + e.size > bytes_)
          return fail ("entry out of bounds");
      }

      return true;
    }

    span<const std::byte> shader_pack::
    find (uint64_t key) const noexcept
    {
      const span<const entry> es (entries_, count_);

      auto i (ranges::lower_bound (es, key, {}, &entry::key));
      if (i == es.end () || i->key != key)
        return {};

      return span<const std::byte> (view_ + i->offset, i->size);
    }

    namespace
    {
      // Open the pack next to our own module, which is iw4x.dll whatever
      // it's named.
      //
      bool
      open_pack (shader_pack& p)
      {
        HMODULE m (nullptr);
        GetModuleHandleExW (GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCWSTR> (&open_pack),
                            &m);

        wstring s (MAX_PATH, L'\0');
        for (;;)
        {
          const DWORD n (GetModuleFileNameW (m, s.data (), static_cast<DWORD> (s.size ())));

          if (n < s.size ())
          {
            s.resize (n);
            break;
          }

          s.resize (s.size () * 2);
        }

        const filesystem::path f (filesystem::path (s).parent_path () / L"iw4x-d3d11.pack");

        if (!p.open (f))
          return false;

        Logger::Print (Game::CON_CHANNEL_GFX,
                       "Loaded {} Direct3D 11 shaders from '{}'\n",
                       p.size (),
                       f.string ());
        return true;
      }
    }

    const shader_pack&
    shaders ()
    {
      static shader_pack p;
      static const bool opened (open_pack (p));

      static_cast<void> (opened);
      return p;
    }

    pair<uint64_t, size_t>
    hash_bytecode (const DWORD* f, size_t limit) noexcept
    {
      // Find the end token first. The version token comes first, then
      // instructions and comments, whose lengths are in their first token
      // (Shader Model 2 and later, which is all IW4 has).
      //
      size_t end (0);
      for (size_t i (1); i < limit;)
      {
        const DWORD t (f[i]);

        if (t == D3DSIO_END)
        {
          end = i + 1;
          break;
        }

        if ((t & D3DSI_OPCODE_MASK) == D3DSIO_COMMENT)
          i += 1 + ((t & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT);
        else
          i += 1 + ((t & D3DSI_INSTLENGTH_MASK) >> D3DSI_INSTLENGTH_SHIFT);
      }

      if (end == 0)
        return {0, 0};

      uint64_t h (0xCBF29CE484222325);
      const std::byte* p (reinterpret_cast<const std::byte*> (f));
      for (size_t i (0); i != end * sizeof (DWORD); ++i)
      {
        h ^= static_cast<uint64_t> (p[i]);
        h *= 0x100000001B3;
      }

      return {h, end};
    }

    namespace
    {
      // Return the inputs that the vertex shader declares. These are the
      // dcl instructions with an input register as the destination.
      //
      vector<shader_input>
      parse_inputs (const DWORD* f, size_t tokens)
      {
        vector<shader_input> r;

        for (size_t i (1); i < tokens;)
        {
          const DWORD t (f[i]);

          if (t == D3DSIO_END)
            break;

          if ((t & D3DSI_OPCODE_MASK) == D3DSIO_COMMENT)
          {
            i += 1 + ((t & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT);
            continue;
          }

          const size_t n ((t & D3DSI_INSTLENGTH_MASK) >> D3DSI_INSTLENGTH_SHIFT);

          if ((t & D3DSI_OPCODE_MASK) == D3DSIO_DCL && n >= 2 && i + 2 < tokens)
          {
            const DWORD u (f[i + 1]);
            const DWORD d (f[i + 2]);

            const DWORD type (((d & D3DSP_REGTYPE_MASK) >> D3DSP_REGTYPE_SHIFT) |
                              ((d & D3DSP_REGTYPE_MASK2) >> D3DSP_REGTYPE_SHIFT2));

            if (type == D3DSPR_INPUT)
              r.push_back (shader_input {d & D3DSP_REGNUM_MASK,
                                         static_cast<BYTE> (u & D3DSP_DCL_USAGE_MASK),
                                         static_cast<BYTE> ((u & D3DSP_DCL_USAGEINDEX_MASK) >> D3DSP_DCL_USAGEINDEX_SHIFT)});
          }

          i += 1 + n;
        }

        return r;
      }

      // The semantic names of the D3DDECLUSAGE values, which is also how
      // dexilifier names the vertex shader inputs (from the dcl_*
      // instructions of the disassembly).
      //
      const char* const usage_names[]
      {
        "POSITION",
        "BLENDWEIGHT",
        "BLENDINDICES",
        "NORMAL",
        "PSIZE",
        "TEXCOORD",
        "TANGENT",
        "BINORMAL",
        "TESSFACTOR",
        "POSITIONT",
        "COLOR",
        "FOG",
        "DEPTH",
        "SAMPLE"
      };
    }

    // vertex_shader
    //
    vertex_shader::
    vertex_shader (device& d, const DWORD* f, size_t tokens, uint64_t key)
      : inputs (parse_inputs (f, tokens)),
        device_ (d),
        function_ (f, f + tokens),
        key_ (key)
    {
    }

    HRESULT vertex_shader::
    GetDevice (IDirect3DDevice9** r)
    {
      return get_device (device_, r);
    }

    HRESULT vertex_shader::
    GetFunction (void* data, UINT* size)
    {
      if (size == nullptr)
        return D3DERR_INVALIDCALL;

      const UINT n (static_cast<UINT> (function_.size () * sizeof (DWORD)));

      if (data != nullptr)
      {
        if (*size < n)
          return D3DERR_INVALIDCALL;

        memcpy (data, function_.data (), n);
      }

      *size = n;
      return D3D_OK;
    }

    // pixel_shader
    //
    pixel_shader::
    pixel_shader (device& d, const DWORD* f, size_t tokens, uint64_t key)
      : device_ (d), function_ (f, f + tokens), key_ (key)
    {
    }

    HRESULT pixel_shader::
    GetDevice (IDirect3DDevice9** r)
    {
      return get_device (device_, r);
    }

    HRESULT pixel_shader::
    GetFunction (void* data, UINT* size)
    {
      if (size == nullptr)
        return D3DERR_INVALIDCALL;

      const UINT n (static_cast<UINT> (function_.size () * sizeof (DWORD)));

      if (data != nullptr)
      {
        if (*size < n)
          return D3DERR_INVALIDCALL;

        memcpy (data, function_.data (), n);
      }

      *size = n;
      return D3D_OK;
    }

    // vertex_declaration
    //
    vertex_declaration::
    vertex_declaration (device& d, const D3DVERTEXELEMENT9* e)
      : device_ (d)
    {
      for (; e->Stream != 0xFF; ++e)
        elements.push_back (*e);
    }

    HRESULT vertex_declaration::
    GetDevice (IDirect3DDevice9** r)
    {
      return get_device (device_, r);
    }

    HRESULT vertex_declaration::
    GetDeclaration (D3DVERTEXELEMENT9* e, UINT* n)
    {
      if (n == nullptr)
        return D3DERR_INVALIDCALL;

      const UINT count (static_cast<UINT> (elements.size () + 1));

      if (e != nullptr)
      {
        ranges::copy (elements, e);
        e[count - 1] = D3DDECL_END ();
      }

      *n = count;
      return D3D_OK;
    }

    // layout_cache
    //
    const input_layout* layout_cache::
    find (ID3D11Device* d, const vertex_declaration& decl, const vertex_shader& vs)
    {
      const pair<uint64_t, uint64_t> k (decl.id (), vs.id ());

      if (auto i (layouts_.find (k)); i != layouts_.end ())
        return i->second.layout != nullptr ? &i->second : nullptr;

      input_layout& r (layouts_[k]);
      r.types.fill (absent_input);

      // D3D9 matches the declaration to the shader by usage and usage
      // index, so we build the D3D11 elements from the shader's inputs in
      // the same way. An input that the declaration doesn't feed reads
      // zeros from the absent slot and its type tells the shader to
      // substitute what D3D9 would have read.
      //
      vector<D3D11_INPUT_ELEMENT_DESC> es;
      es.reserve (vs.inputs.size ());

      for (const shader_input& in: vs.inputs)
      {
        if (in.usage >= size (usage_names) || in.reg >= r.types.size ())
          continue;

        D3D11_INPUT_ELEMENT_DESC e {};
        e.SemanticName   = usage_names[in.usage];
        e.SemanticIndex  = in.index;
        e.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

        auto i (ranges::find_if (decl.elements, [&in] (const D3DVERTEXELEMENT9& x)
        {
          return x.Usage == in.usage && x.UsageIndex == in.index;
        }));

        const DXGI_FORMAT f (i != decl.elements.end ()
                             ? vertex_format (static_cast<D3DDECLTYPE> (i->Type))
                             : DXGI_FORMAT_UNKNOWN);

        if (f != DXGI_FORMAT_UNKNOWN)
        {
          e.Format            = f;
          e.InputSlot         = i->Stream;
          e.AlignedByteOffset = i->Offset;
          r.types[in.reg]     = i->Type;
        }
        else
        {
          e.Format            = DXGI_FORMAT_R32G32B32A32_UINT;
          e.InputSlot         = absent_slot;
          e.AlignedByteOffset = 0;
          r.absent            = true;
        }

        es.push_back (e);
      }

      if (vs.dxbc.empty () ||
          FAILED (d->CreateInputLayout (es.data (),
                                        static_cast<UINT> (es.size ()),
                                        vs.dxbc.data (),
                                        vs.dxbc.size (),
                                        &r.layout)))
      {
        Logger::Warning (Game::CON_CHANNEL_GFX,
                         "Couldn't create the input layout for vertex shader {:016x}\n",
                         vs.key ());
        return nullptr;
      }

      return &r;
    }
  }
}
