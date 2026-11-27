#pragma once

#include "Object.hpp"
#include "Resources.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace Components
{
  namespace d3d11
  {
    class device;

    // The shader pack: every IW4 shader translated to Shader Model 5 ahead
    // of time, keyed by the D3D9 bytecode.
    //
    // D3D11 can't run the game's D3D9 shaders and translating them at run
    // time would mean shipping a shader compiler (and the one Wine has
    // isn't Microsoft's). So instead dexilifier decompiles each shader
    // from the game's zones into HLSL and fxc compiles it into DXBC, all
    // offline (see DeXILifierCli's --pack option), and we look them up here
    // when the game creates its D3D9 shaders.
    //
    // The pack is embedded into iw4x.dll as the IW4X_D3D11_SHADERS resource,
    // compressed with zlib (which takes it from about 60MB down to 8MB),
    // and we inflate it into memory on first use. Uncompressed, it looks
    // like this:
    //
    //   char     magic[8];       // "DXBCPACK"
    //   uint32_t version;        // 1
    //   uint32_t count;
    //   struct
    //   {
    //     uint64_t key;          // FNV-1a of the D3D9 bytecode.
    //     uint32_t offset;       // From the start of the file.
    //     uint32_t size;
    //   } entries[count];        // Sorted by key.
    //   ...                      // The DXBC, each aligned to 4 bytes.
    //
    // Note that src/iw4x-d3d11.pack.z is generated: to update it, rebuild
    // the pack and compress it with zlib (Python's zlib.compress() at level
    // 9, say).
    //
    // The key covers the D3D9 bytecode up to and including its end token,
    // comments too, which is everything CreateVertexShader() and
    // CreatePixelShader() get (see hash_bytecode()).
    //
    class shader_pack
    {
    public:
      shader_pack () = default;

      shader_pack (const shader_pack&) = delete;
      shader_pack& operator= (const shader_pack&) = delete;

      // Inflate the compressed pack. Return false (and log why) if it's
      // malformed.
      //
      bool
      open (std::span<const std::byte> compressed);

      // Return the DXBC for the key. Return an empty span if the pack has
      // no such shader.
      //
      std::span<const std::byte>
      find (uint64_t key) const noexcept;

      size_t
      size () const noexcept
      {
        return count_;
      }

    private:
      struct entry
      {
        uint64_t key;
        uint32_t offset;
        uint32_t size;
      };

      std::string  data_;
      const entry* entries_ = nullptr;
      size_t       count_   = 0;
    };

    // Return the shader pack embedded into the client, opening it on first
    // use. If it fails to open, then the pack is empty and every shader is
    // missing.
    //
    const shader_pack&
    shaders ();

    // Return the key of the D3D9 bytecode and, in tokens, its length up to
    // and including the end token. Return a zero length if it has no end
    // token within the limit.
    //
    std::pair<uint64_t, size_t>
    hash_bytecode (const DWORD*, size_t limit = 1 << 20) noexcept;

    // A vertex shader input as the D3D9 bytecode declares it.
    //
    struct shader_input
    {
      UINT  reg;
      BYTE  usage;
      BYTE  index;
    };

    class vertex_shader: public unknown<IDirect3DVertexShader9>
    {
    public:
      vertex_shader (device&, const DWORD* function, size_t tokens, uint64_t key);

      STDMETHOD (GetDevice) (IDirect3DDevice9**) override;
      STDMETHOD (GetFunction) (void*, UINT*) override;

      uint64_t key () const noexcept {return key_;}

      // The shader, null if the pack doesn't have it (in which case we skip
      // the draws that use it).
      //
      ComPtr<ID3D11VertexShader> shader;
      std::span<const std::byte> dxbc;
      std::vector<shader_input>  inputs;

    private:
      device&            device_;
      std::vector<DWORD> function_;
      uint64_t           key_;
    };

    class pixel_shader: public unknown<IDirect3DPixelShader9>
    {
    public:
      pixel_shader (device&, const DWORD* function, size_t tokens, uint64_t key);

      STDMETHOD (GetDevice) (IDirect3DDevice9**) override;
      STDMETHOD (GetFunction) (void*, UINT*) override;

      uint64_t key () const noexcept {return key_;}

      ComPtr<ID3D11PixelShader> shader;

    private:
      device&            device_;
      std::vector<DWORD> function_;
      uint64_t           key_;
    };

    class vertex_declaration: public unknown<IDirect3DVertexDeclaration9>
    {
    public:
      vertex_declaration (device&, const D3DVERTEXELEMENT9* elements);

      STDMETHOD (GetDevice) (IDirect3DDevice9**) override;
      STDMETHOD (GetDeclaration) (D3DVERTEXELEMENT9*, UINT*) override;

      // The elements without the terminating D3DDECL_END().
      //
      std::vector<D3DVERTEXELEMENT9> elements;

    private:
      device& device_;
    };

    // The D3D9 type that the shader's Direct3D9State.inputTypes reports for
    // a vertex shader input that the declaration doesn't feed. DecodeInput()
    // then returns (0, 0, 0, 1), which is what D3D9 reads for it.
    //
    constexpr UINT absent_input (17);

    // The input slot that feeds the absent inputs. We bind a small buffer
    // of zeros there with a stride of zero.
    //
    constexpr UINT absent_slot (D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - 1);

    // The input layout for a declaration and a vertex shader, which D3D11
    // needs together while D3D9 has them separate.
    //
    struct input_layout
    {
      ComPtr<ID3D11InputLayout> layout;
      std::array<UINT, 16>      types; // D3DDECLTYPE per v register.
      bool                      absent = false;
    };

    class layout_cache
    {
    public:
      // Return the layout for the pair, creating it if needed. Return
      // nullptr if we can't create one (logged once per pair).
      //
      const input_layout*
      find (ID3D11Device*, const vertex_declaration&, const vertex_shader&);

    private:
      std::map<std::pair<uint64_t, uint64_t>, input_layout> layouts_;
    };
  }
}
