#include <Vulkan/vk_d3dbc.hpp>

#include <bit>       // countr_zero()
#include <limits>    // numeric_limits
#include <concepts>  // invocable
#include <algorithm> // min(), max(), ranges::find()

using namespace std;

// If you change anything here, use dexilifier
// (https://github.com/iw4x-community/dexilifier/) as a reference.
//
namespace Vulkan
{
  // Direct3D 9 shader bytecode (d3dbc) utilities.
  //
  // We translate IW4's shaders to SPIR-V with vkd3d-shader. Before handing
  // the bytecode over, we scan it for what the pipeline needs to know (the
  // constant set sizes, the samplers, etc) and rewrite the parts that
  // vkd3d-shader doesn't seem to translate as Direct3D 9 defines them.
  //
  // The bytecode is a stream of 32-bit tokens. It starts with the version
  // token and ends with the end token. In between, each instruction is an
  // instruction token followed by its parameter tokens:
  //
  // instruction:  bits 0-15 opcode, 24-27 parameter count, 28 predicated
  // parameter:    bits 0-10 register number, 11-12 and 28-30 register type,
  //               13 relative addressing, 31 always set
  //
  // A destination parameter keeps the write mask in bits 16-19. A source
  // parameter keeps the swizzle in bits 16-23 (two bits per component) and
  // the modifier (negation, absolute value, etc) in bits 24-27. If a source
  // is relatively addressed, then the next token is the address register.
  //
  // Comments are blocks of arbitrary data with the comment opcode and the
  // block size in bits 16-30. The compiler stores things like the constant
  // table there.
  //
  // Note that the parameter count only exists from shader model 2 on. So we
  // only support shader models 2 and 3, which covers IW4's shaders.
  //
  namespace
  {
    const uint32_t end_token (0x0000ffff);
    const uint32_t comment_opcode (0xfffe);

    const uint32_t dcl_opcode (31);
    const uint32_t def_opcode (81);
    const uint32_t defi_opcode (82);
    const uint32_t defb_opcode (83);

    // Register types that we care about (D3DSPR_*).
    //
    enum register_type: uint32_t
    {
      input_register = 1,
      const_register = 2,
      constint_register = 7,
      sampler_register = 10,
      const2_register = 11,
      const3_register = 12,
      const4_register = 13,
      constbool_register = 14,
      misc_register = 17 // vPos and vFace.
    };

    // D3DSMO_POSITION and D3DSMO_FACE.
    //
    const uint32_t misc_position (0);
    const uint32_t misc_face (1);

    const uint32_t usage_texcoord (5); // D3DDECLUSAGE_TEXCOORD.

    const uint32_t input_registers (10); // Pixel shader v0-v9.
    const uint32_t usage_indexes (16);   // Texcoord usage indexes 0-15.

    const uint32_t relative_bit (1u << 13);

    // Return the register type, which is split between bits 28-30 (low part)
    // and 11-12 (high part) of the parameter token.
    //
    uint32_t
    register_type (uint32_t t)
    {
      return ((t >> 28) & 0x7) | ((t >> 8) & 0x18);
    }

    uint32_t
    register_number (uint32_t t)
    {
      return t & 0x7ff;
    }

    // Return the parameter token with the register type and number replaced.
    // The other fields stay as they are.
    //
    uint32_t
    set_register (uint32_t t, uint32_t type, uint32_t number)
    {
      t &= ~((0x7u << 28) | (0x3u << 11) | 0x7ffu);
      return t | ((type & 0x7) << 28) | (((type >> 3) & 0x3) << 11) | number;
    }

    // Call f for every instruction with its opcode and parameter tokens,
    // which f may modify in place. The version token and the comments are
    // skipped.
    //
    template <invocable<uint32_t, span<uint32_t>> F>
    void
    for_each_instruction (span<uint32_t> ts, F&& f)
    {
      for (size_t i (1); i < ts.size () && ts[i] != end_token; )
      {
        uint32_t t (ts[i]);
        uint32_t op (t & 0xffff);

        if (op == comment_opcode)
        {
          i += 1 + ((t >> 16) & 0x7fff);
          continue;
        }

        size_t n ((t >> 24) & 0xf);
        f (op, ts.subspan (i + 1, n));
        i += 1 + n;
      }
    }
  }

  optional<vk_d3dbc_info>
  vk_d3dbc_scan (const uint32_t* ts, size_t limit)
  {
    // Walk the tokens and collect what the pipeline needs to know about the
    // shader. Note that we are careful not to read past the limit since the
    // code comes from IW4's assets and we can't assume it is well formed. On
    // any inconsistency we return nullopt.
    //
    if (limit == 0)
      return nullopt;

    vk_d3dbc_info r;
    uint32_t v (ts[0]);

    // The version token's upper half is 0xFFFF for pixel shaders and 0xFFFE
    // for vertex shaders.
    //
    if ((v >> 16) == 0xffff)
      r.pixel = true;
    else if ((v >> 16) != 0xfffe)
      return nullopt;

    r.major = (v >> 8) & 0xff;
    r.minor = v & 0xff;

    if (r.major < 2 || r.major > 3)
      return nullopt;

    // Record a register use. For each constant set we track the highest
    // register number since that determines the set size that the shader
    // needs (see vk_context::copy_constants()). The const2, const3, and
    // const4 types address the float constants from 2048, 4096, and 6144,
    // which is what the fall-through accumulates.
    //
    auto use = [&r] (uint32_t t)
    {
      uint32_t n (register_number (t));

      switch (register_type (t))
      {
        case const4_register:
          n += 2048;
          // Fall through.
        case const3_register:
          n += 2048;
          // Fall through.
        case const2_register:
          n += 2048;
          // Fall through.
        case const_register:
        {
          r.floats = max (r.floats, n + 1);

          if (t & relative_bit) // c[a0.x + n]
            r.relative = true;

          break;
        }
        case constint_register:  r.ints = max (r.ints, n + 1);   break;
        case constbool_register: r.bools = max (r.bools, n + 1); break;
        case misc_register: // See vk_d3dbc_lower_misc().
        {
          if (n == misc_position)
            r.position = true;
          else if (n == misc_face)
            r.face = true;

          break;
        }
        default:
          break;
      }
    };

    for (size_t i (1); i < limit; )
    {
      uint32_t t (ts[i]);

      if (t == end_token)
      {
        r.size = (i + 1) * sizeof (uint32_t); // Including the end token.

        // With relative addressing the shader can read any float constant,
        // so we have to assume the whole set (224 registers in pixel shaders
        // and 256 in vertex shaders). The translation then resizes the
        // uniform block to match (see vk_shader_translate.cpp).
        //
        if (r.relative)
          r.floats = r.pixel ? 224 : 256;

        return r;
      }

      uint32_t op (t & 0xffff);

      if (op == comment_opcode)
      {
        i += 1 + ((t >> 16) & 0x7fff);
        continue;
      }

      size_t n ((t >> 24) & 0xf);
      if (i + 1 + n > limit) // Truncated.
        return nullopt;

      const uint32_t* ps (ts + i + 1);

      // For declarations, we only care about samplers. The context needs
      // their types to bind the right kind of texture (see
      // vk_context::collect_textures()).
      //
      if (op == dcl_opcode)
      {
        if (n == 2 && register_type (ps[1]) == sampler_register)
        {
          uint32_t s (register_number (ps[1]));

          if (s < r.sampler_types.size ())
          {
            r.samplers |= 1u << s;

            // The usage token keeps the texture type in bits 27-30 (2 is 2D,
            // 3 is cube, and 4 is volume).
            //
            switch ((ps[0] >> 27) & 0xf)
            {
              case 3:  r.sampler_types[s] = vk_sampler_type::cube;   break;
              case 4:  r.sampler_types[s] = vk_sampler_type::volume; break;
              default: r.sampler_types[s] = vk_sampler_type::plane;  break;
            }
          }
        }
      }
      // Constants defined in the shader (def, defi, and defb) count towards
      // the set size as well.
      //
      else if (op == def_opcode || op == defi_opcode || op == defb_opcode)
      {
        if (n != 0)
          use (ps[0]);
      }
      // For other instructions look at every parameter token. Note that the
      // address tokens get here as well but they refer to the address
      // register, which use() ignores.
      //
      else
      {
        for (size_t j (0); j != n; ++j)
        {
          if (ps[j] & 0x80000000)
            use (ps[j]);
        }
      }

      i += 1 + n;
    }

    return nullopt;
  }

  optional<span<const uint32_t>>
  vk_d3dbc_code (const uint32_t* ts)
  {
    // IDirect3DDevice9::CreateVertexShader() and CreatePixelShader() don't
    // pass the size, so we scan for the end token without a limit.
    //
    optional<vk_d3dbc_info> i (
      vk_d3dbc_scan (ts, numeric_limits<size_t>::max () / sizeof (*ts)));

    if (!i)
      return nullopt;

    return span<const uint32_t> (ts, i->size / sizeof (*ts));
  }

  optional<vk_d3dbc_lowered>
  vk_d3dbc_lower_misc (span<const uint32_t> code)
  {
    // vkd3d-shader doesn't seem to support the misc registers (vPos and
    // vFace) in d3dbc pixel shaders. So we turn them into ordinary inputs.
    // Each gets a free input register declared as a texcoord with a free
    // usage index, which vkd3d-shader translates just fine. The caller then
    // patches the SPIR-V so that these inputs read the fragment coordinate
    // and the front-facing flag (see vk_spirv::lower_position() and
    // lower_face()).
    //
    // Note that the input register and the usage index have to be unused by
    // the shader, so we return nullopt if there are none left.
    //
    vk_d3dbc_lowered r {vector<uint32_t> (code.begin (), code.end ()),
                        nullopt,
                        nullopt};

    // Return the instruction's register parameters. A declaration's usage
    // token comes before the register while a definition's values come
    // after it.
    //
    auto registers = [] (uint32_t op, span<uint32_t> ps)
    {
      if (op == dcl_opcode)
        return ps.subspan (ps.size () > 0 ? 1 : 0);

      if (op == def_opcode || op == defi_opcode || op == defb_opcode)
        return ps.first (ps.empty () ? 0 : 1);

      return ps;
    };

    // First collect the input registers and texcoord usage indexes that the
    // shader uses, together with whether it reads vPos and vFace.
    //
    uint32_t inputs (0);
    uint32_t indexes (0);
    bool position (false);
    bool face (false);

    for_each_instruction (
      r.code,
      [&registers, &inputs, &indexes, &position, &face] (uint32_t op,
                                                         span<uint32_t> ps)
      {
        if (op == dcl_opcode && ps.size () == 2 &&
            register_type (ps[1]) == input_register &&
            (ps[0] & 0x1f) == usage_texcoord)
          indexes |= 1u << ((ps[0] >> 16) & 0xf);

        for (uint32_t t: registers (op, ps))
        {
          if ((t & 0x80000000) == 0)
            continue;

          uint32_t n (register_number (t));

          switch (register_type (t))
          {
            case input_register:
            {
              if (n < input_registers)
                inputs |= 1u << n;

              break;
            }
            case misc_register:
            {
              position = position || n == misc_position;
              face = face || n == misc_face;
              break;
            }
            default:
              break;
          }
        }
      });

    // Take the highest free index, nullopt if all of them are taken. Shaders
    // seem to allocate their inputs from the bottom, so the top is the most
    // likely to be free.
    //
    auto pick = [] (uint32_t& used, uint32_t count) -> optional<uint32_t>
    {
      for (uint32_t i (count); i != 0; --i)
      {
        if ((used & (1u << (i - 1))) == 0)
        {
          used |= 1u << (i - 1);
          return i - 1;
        }
      }

      return nullopt;
    };

    optional<uint32_t> usage[2];

    if (position)
    {
      r.position = pick (inputs, input_registers);
      usage[misc_position] = pick (indexes, usage_indexes);

      if (!r.position || !usage[misc_position])
        return nullopt;
    }

    if (face)
    {
      r.face = pick (inputs, input_registers);
      usage[misc_face] = pick (indexes, usage_indexes);

      if (!r.face || !usage[misc_face])
        return nullopt;
    }

    // Now redirect every vPos and vFace reference to the picked input
    // register. In the declaration we also replace the usage token with a
    // texcoord one and set the full write mask, which an input declaration
    // needs.
    //
    for_each_instruction (
      r.code,
      [&registers, &r, &usage] (uint32_t op, span<uint32_t> ps)
      {
        for (uint32_t& t: registers (op, ps))
        {
          if ((t & 0x80000000) == 0 || register_type (t) != misc_register)
            continue;

          uint32_t n (register_number (t));
          if (n != misc_position && n != misc_face)
            continue;

          uint32_t input (n == misc_position ? *r.position : *r.face);
          t = set_register (t, input_register, input);

          if (op == dcl_opcode)
          {
            ps[0] = 0x80000000 | usage_texcoord | (*usage[n] << 16);
            t |= 0xfu << 16;
          }
        }
      });

    return r;
  }

  namespace
  {
    // Arithmetic opcodes (D3DSIO_*).
    //
    enum opcode: uint32_t
    {
      mov_opcode = 1,
      add_opcode = 2,
      sub_opcode = 3,
      mad_opcode = 4,
      mul_opcode = 5,
      rsq_opcode = 7,
      dp3_opcode = 8,
      dp4_opcode = 9,
      min_opcode = 10,
      max_opcode = 11,
      slt_opcode = 12,
      sge_opcode = 13,
      exp_opcode = 14,
      log_opcode = 15,
      lit_opcode = 16,
      dst_opcode = 17,
      lrp_opcode = 18,
      frc_opcode = 19,
      m4x4_opcode = 20,
      m4x3_opcode = 21,
      m3x4_opcode = 22,
      m3x3_opcode = 23,
      m3x2_opcode = 24,
      pow_opcode = 32,
      crs_opcode = 33,
      sgn_opcode = 34,
      abs_opcode = 35,
      nrm_opcode = 36,
      mova_opcode = 46,
      expp_opcode = 78,
      logp_opcode = 79,
      cnd_opcode = 80,
      cmp_opcode = 88,
      dp2add_opcode = 90,
      dsx_opcode = 91,
      dsy_opcode = 92
    };

    const uint32_t predicated_bit (1u << 28); // Instruction token.
    const uint32_t temp_register (0);         // D3DSPR_TEMP.

    // Source modifiers (D3DSPSM_*).
    //
    const uint32_t modifier_none (0);
    const uint32_t modifier_neg (1);
    const uint32_t modifier_abs (11);
    const uint32_t modifier_absneg (12);

    // An instruction parameter, which is the token and, for a relatively
    // addressed source, the address token that follows it.
    //
    struct param
    {
      uint32_t token;
      optional<uint32_t> relative;
    };

    // Return the component that the source swizzle selects for component i
    // (0 is x and 3 is w).
    //
    uint32_t
    component (const param& p, uint32_t i) noexcept
    {
      return (p.token >> (16 + 2 * i)) & 0x3;
    }

    // Return the source with the pattern composed with its swizzle. That is,
    // the result's component i selects what the original swizzle selects
    // for pattern[i].
    //
    param
    swizzle (param p, const array<uint32_t, 4>& pattern) noexcept
    {
      uint32_t s (0);
      for (uint32_t i (0); i != 4; ++i)
        s |= component (p, pattern[i]) << (2 * i);

      p.token = (p.token & ~(0xffu << 16)) | (s << 16);
      return p;
    }

    // Return the source with component i replicated (.xxxx, .yyyy, etc).
    //
    param
    replicate (const param& p, uint32_t i) noexcept
    {
      return swizzle (p, {i, i, i, i});
    }

    // Return the negated source, nullopt if its modifier can't be combined
    // with negation (bias and sign, for example).
    //
    optional<param>
    negate (param p) noexcept
    {
      uint32_t m ((p.token >> 24) & 0xf);

      switch (m)
      {
        case modifier_none:   m = modifier_neg;    break;
        case modifier_neg:    m = modifier_none;   break;
        case modifier_abs:    m = modifier_absneg; break;
        case modifier_absneg: m = modifier_abs;    break;
        default:              return nullopt;
      }

      p.token = (p.token & ~(0xfu << 24)) | (m << 24);
      return p;
    }

    // Return the source's absolute value, nullopt if its modifier is
    // something other than negation and absolute value.
    //
    optional<param>
    absolute (param p) noexcept
    {
      switch ((p.token >> 24) & 0xf)
      {
        case modifier_none:
        case modifier_neg:
        case modifier_abs:
        case modifier_absneg: break;
        default:              return nullopt;
      }

      p.token = (p.token & ~(0xfu << 24)) | (modifier_abs << 24);
      return p;
    }

    // Return the parameter for the register that is n registers after this
    // one. We use this to address matrix rows, which take consecutive
    // constant registers.
    //
    param
    offset (param p, uint32_t n) noexcept
    {
      p.token = set_register (p.token,
                              register_type (p.token),
                              register_number (p.token) + n);
      return p;
    }

    // Destination and source parameters for temporary register r. The
    // defaults are the full write mask and the identity swizzle (.xyzw).
    //
    param
    temp_dst (uint32_t r, uint32_t mask = 0xf) noexcept
    {
      return param {0x80000000 | (mask << 16) | r, nullopt};
    }

    param
    temp_src (uint32_t r, uint32_t swizzle = 0xe4) noexcept
    {
      return param {0x80000000 | (swizzle << 16) | r, nullopt};
    }

    // Append the instruction to the code. Note that a relatively addressed
    // parameter takes two tokens in the parameter count.
    //
    void
    emit (vector<uint32_t>& code,
          uint32_t op,
          const param& d,
          initializer_list<param> ss)
    {
      auto size = [] (const param& p) -> uint32_t
      {
        return p.relative ? 2 : 1;
      };

      uint32_t n (size (d));
      for (const param& p: ss)
        n += size (p);

      code.push_back (op | (n << 24));

      auto push = [&code] (const param& p)
      {
        code.push_back (p.token);

        if (p.relative)
          code.push_back (*p.relative);
      };

      push (d);

      for (const param& p: ss)
        push (p);
    }
  }

  optional<vector<uint32_t>>
  vk_d3dbc_expand (span<const uint32_t> code)
  {
    // Direct3D 9 has a number of "macro" instructions (the matrix multiplies
    // and lit, for example). Drivers expand them into simpler instructions.
    // There are also a few instructions with subtle semantics (log, rsq,
    // and pow operate on the source's absolute value). We
    // can't rely on vkd3d-shader translating all of them as Direct3D 9
    // defines them, so we expand them ourselves into instructions that it
    // does handle.
    //
    // The expansions need up to two temporary registers, which we take
    // above the highest one that the shader uses. If that exceeds the shader
    // model's limit, then we fail (return nullopt) since there is no correct
    // fallback.
    //
    vector<uint32_t> ts (code.begin (), code.end ());

    // Split the parameter tokens into parameters, attaching the address
    // token to a relatively addressed source.
    //
    auto split = [] (span<uint32_t> ps) -> vector<param>
    {
      vector<param> r;

      for (size_t i (0); i < ps.size (); )
      {
        param p {ps[i++], nullopt};

        if ((p.token & relative_bit) != 0 && i < ps.size ())
          p.relative = ps[i++];

        r.push_back (p);
      }

      return r;
    };

    auto expandable = [] (uint32_t op)
    {
      switch (op)
      {
        case log_opcode:  case rsq_opcode:
        case sub_opcode:  case lrp_opcode:  case pow_opcode:
        case crs_opcode:  case nrm_opcode:  case sgn_opcode:
        case lit_opcode:  case dst_opcode:  case expp_opcode:
        case logp_opcode: case m4x4_opcode: case m4x3_opcode:
        case m3x4_opcode: case m3x3_opcode: case m3x2_opcode:
        case dp2add_opcode:
          return true;
      }

      return false;
    };

    // First see whether there is anything to expand at all and, if not,
    // return the code as is. While at it, find the first temporary register
    // that the shader doesn't use. Note that log, logp, and rsq are
    // rewritten in place and need no temporaries.
    //
    bool found (false);
    bool temporary (false);
    uint32_t temps (0);

    for_each_instruction (
      ts,
      [&expandable, &found, &temporary, &temps] (uint32_t op, span<uint32_t> ps)
      {
        if (op == def_opcode || op == defi_opcode || op == defb_opcode)
          return;

        if (expandable (op))
        {
          found = true;
          temporary = temporary || (op != log_opcode  &&
                                    op != logp_opcode &&
                                    op != rsq_opcode);
        }

        for (size_t i (op == dcl_opcode ? 1 : 0); i < ps.size (); ++i)
        {
          uint32_t t (ps[i]);

          if ((t & 0x80000000) != 0 && register_type (t) == temp_register)
            temps = max (temps, register_number (t) + 1);
        }
      });

    if (!found)
      return ts;

    // Shader model 3 has 32 temporary registers and shader model 2 has 12.
    //
    const uint32_t major ((ts[0] >> 8) & 0xff);
    const uint32_t limit (major >= 3 ? 32 : 12);

    if (temporary && temps + 2 > limit)
      return nullopt;

    const uint32_t t0 (temps);
    const uint32_t t1 (temps + 1);

    // Copy the instructions, replacing the expandable ones. The two
    // temporaries are t0 and t1.
    //
    vector<uint32_t> r;
    r.reserve (ts.size () * 2);
    r.push_back (ts[0]);

    for (size_t i (1); i < ts.size (); )
    {
      uint32_t t (ts[i]);
      uint32_t op (t & 0xffff);

      size_t n (op == comment_opcode || t == end_token
                ? (t == end_token ? 0 : (t >> 16) & 0x7fff)
                : (t >> 24) & 0xf);

      span<uint32_t> ps (span<uint32_t> (ts).subspan (i + 1, n));

      auto copy = [&r, &ts, i, n] ()
      {
        r.insert (r.end (), ts.begin () + i, ts.begin () + i + 1 + n);
      };

      vector<param> as (split (ps));

      // Copy everything that we don't expand as is.
      //
      // @@ This includes predicated macro instructions, which we would have
      //    to expand into a sequence of predicated instructions. Probably
      //    rare enough not to bother.
      //
      if (t == end_token            ||
          op == comment_opcode      ||
          !expandable (op)          ||
          (t & predicated_bit) != 0 ||
          as.empty ())
      {
        copy ();
        i += 1 + n;
        continue;
      }

      const param d (as[0]);
      const vector<param> ss (as.begin () + 1, as.end ());

      auto arity = [&ss] (size_t k) {return ss.size () == k;};

      auto temp = [&r] (uint32_t o, const param& x, initializer_list<param> xs)
      {
        emit (r, o, x, xs);
      };

      auto result = [&r, &d] (uint32_t o, initializer_list<param> xs)
      {
        emit (r, o, d, xs);
      };

      // Set to false if the instruction turns out to be unexpandable (wrong
      // parameter count, unsupported modifier), in which case we copy it as
      // is.
      //
      bool done (true);

      switch (op)
      {
        case log_opcode:
        case logp_opcode:
        case rsq_opcode:
        {
          // Direct3D 9 defines log and rsq on the source's absolute value
          // (so that log (-x) and rsq (-x) are not NaN). We make this
          // explicit with the abs modifier. Partial precision logp is
          // computed as full precision log.
          //
          optional<param> a;
          if (!arity (1) || !(a = absolute (ss[0])))
          {
            done = false;
            break;
          }

          result (op == rsq_opcode ? rsq_opcode : log_opcode, {*a});
          break;
        }
        case sub_opcode:
        {
          // a - b as a + -b.
          //
          optional<param> b;
          if (!arity (2) || !(b = negate (ss[1])))
          {
            done = false;
            break;
          }

          result (add_opcode, {ss[0], *b});
          break;
        }
        case lrp_opcode:
        {
          // lrp (s, a, b) as s * (a - b) + b with a - b in t0.
          //
          optional<param> b;
          if (!arity (3) || !(b = negate (ss[2])))
          {
            done = false;
            break;
          }

          temp (add_opcode, temp_dst (t0), {ss[1], *b});
          result (mad_opcode, {ss[0], temp_src (t0), ss[2]});
          break;
        }
        case pow_opcode:
        {
          // pow (a, b) as exp (log (|a|) * b), computed in t0.x. As for log,
          // Direct3D 9 defines pow on the base's absolute value.
          //
          optional<param> a;
          if (!arity (2) || !(a = absolute (ss[0])))
          {
            done = false;
            break;
          }

          temp (log_opcode, temp_dst (t0, 0x1), {*a});
          temp (mul_opcode, temp_dst (t0, 0x1), {temp_src (t0, 0x00), ss[1]});
          result (exp_opcode, {temp_src (t0, 0x00)});
          break;
        }
        case crs_opcode:
        {
          // crs (a, b) as a.yzx * b.zxy - a.zxy * b.yzx with the second
          // product in t0. Note that crs only writes xyz, so we don't care
          // about t0.w.
          //
          if (!arity (2))
          {
            done = false;
            break;
          }

          optional<param> c (negate (temp_src (t0)));

          temp (mul_opcode,
                temp_dst (t0, 0x7),
                {swizzle (ss[0], {2, 0, 1, 3}),
                 swizzle (ss[1], {1, 2, 0, 3})});
          result (mad_opcode,
                  {swizzle (ss[0], {1, 2, 0, 3}),
                   swizzle (ss[1], {2, 0, 1, 3}),
                   *c});
          break;
        }
        case nrm_opcode:
        {
          // nrm (a) as a * rsq (dp3 (a, a)) with the reciprocal length in
          // t0.x.
          //
          if (!arity (1))
          {
            done = false;
            break;
          }

          temp (dp3_opcode, temp_dst (t0, 0x1), {ss[0], ss[0]});
          temp (rsq_opcode, temp_dst (t0, 0x1), {temp_src (t0, 0x00)});
          result (mul_opcode, {ss[0], temp_src (t0, 0x00)});
          break;
        }
        case sgn_opcode:
        {
          // sgn (a) as (-a < a) - (a < -a) with the two comparisons in t0
          // and t1, which gives 1, 0, and -1 for positive, zero, and
          // negative components. The instruction's second and third sources
          // are scratch registers for drivers that expand sgn, so we ignore
          // them.
          //
          optional<param> ns;
          if (!arity (3) || !(ns = negate (ss[0])))
          {
            done = false;
            break;
          }

          optional<param> b (negate (temp_src (t1)));

          temp (slt_opcode, temp_dst (t0), {*ns, ss[0]});
          temp (slt_opcode, temp_dst (t1), {ss[0], *ns});
          result (add_opcode, {temp_src (t0), *b});
          break;
        }
        case lit_opcode:
        {
          // lit (a) computes the lighting coefficients
          //
          // (1, max (a.x, 0), a.x > 0 ? max (a.y, 0)^a.w : 0, 1)
          //
          // where a.x is N.L, a.y is N.H, and a.w is the specular power. We
          // build the result in t0 and keep the intermediate values in t1.
          // The constants come from comparing a.x with itself (sge gives 1
          // and slt gives 0) and the power is computed with log and exp, as
          // for pow. Note that we can't write the destination directly since
          // it may also be the source.
          //
          if (!arity (1))
          {
            done = false;
            break;
          }

          const param x (replicate (ss[0], 0));
          const param y (replicate (ss[0], 1));
          const param p (replicate (ss[0], 3));

          temp (slt_opcode, temp_dst (t1), {x, x});
          temp (sge_opcode, temp_dst (t0), {x, x});
          temp (max_opcode, temp_dst (t0, 0x2), {x, temp_src (t1, 0x00)});
          temp (max_opcode, temp_dst (t1, 0x2), {y, temp_src (t1, 0x00)});
          temp (log_opcode, temp_dst (t1, 0x2), {temp_src (t1, 0x55)});
          temp (mul_opcode, temp_dst (t1, 0x2), {temp_src (t1, 0x55), p});
          temp (exp_opcode, temp_dst (t1, 0x2), {temp_src (t1, 0x55)});
          temp (slt_opcode, temp_dst (t1, 0x4), {temp_src (t1, 0x00), x});
          temp (mul_opcode,
                temp_dst (t0, 0x4),
                {temp_src (t1, 0x55), temp_src (t1, 0xaa)});
          result (mov_opcode, {temp_src (t0)});
          break;
        }
        case dst_opcode:
        {
          // dst (a, b) computes the distance vector (1, a.y * b.y, a.z, b.w).
          // As for lit, we build it in t0 with the 1 from sge (a.x, a.x).
          //
          if (!arity (2))
          {
            done = false;
            break;
          }

          const param ax (replicate (ss[0], 0));

          temp (sge_opcode, temp_dst (t0, 0x1), {ax, ax});
          temp (mul_opcode, temp_dst (t0, 0x2), {ss[0], ss[1]});
          temp (mov_opcode, temp_dst (t0, 0x4), {ss[0]});
          temp (mov_opcode, temp_dst (t0, 0x8), {ss[1]});
          result (mov_opcode, {temp_src (t0)});
          break;
        }
        case expp_opcode:
        {
          // Partial precision expp is computed as full precision exp.
          //
          if (!arity (1))
          {
            done = false;
            break;
          }

          result (exp_opcode, {ss[0]});
          break;
        }
        case m4x4_opcode:
        case m4x3_opcode:
        case m3x4_opcode:
        case m3x3_opcode:
        case m3x2_opcode:
        {
          // mNxM (a, m) as one dot product per row (dp4 for 4-component and
          // dp3 for 3-component vectors), with the rows in consecutive
          // registers from m on. As for lit, we go through t0 since the
          // destination may also be the source.
          //
          if (!arity (2))
          {
            done = false;
            break;
          }

          const uint32_t dot (op == m4x4_opcode || op == m4x3_opcode
                              ? dp4_opcode
                              : dp3_opcode);

          const uint32_t rows (op == m4x4_opcode || op == m3x4_opcode ? 4 :
                               op == m3x2_opcode                       ? 2 :
                                                                         3);

          for (uint32_t k (0); k != rows; ++k)
            temp (dot, temp_dst (t0, 1u << k), {ss[0], offset (ss[1], k)});

          result (mov_opcode, {temp_src (t0)});
          break;
        }
        case dp2add_opcode:
        {
          // dp2add (a, b, c) as a.x * b.x + a.y * b.y + c, accumulated in
          // t0.x.
          //
          if (!arity (3))
          {
            done = false;
            break;
          }

          temp (mul_opcode,
                temp_dst (t0, 0x1),
                {replicate (ss[0], 0), replicate (ss[1], 0)});
          temp (mad_opcode,
                temp_dst (t0, 0x1),
                {replicate (ss[0], 1), replicate (ss[1], 1),
                 temp_src (t0, 0x00)});
          result (add_opcode, {temp_src (t0, 0x00), ss[2]});
          break;
        }
      }

      if (!done) // Unexpected parameter count, unsupported modifier.
        copy ();

      i += 1 + n;
    }

    return r;
  }

  namespace
  {
    // Instructions that compute each result component from the same
    // component of each (swizzled) source. For these a result component
    // that is not written doesn't depend on anything (see
    // vk_d3dbc_mask_inputs()).
    //
    const uint32_t componentwise_opcodes[]
    {
      mov_opcode, add_opcode, sub_opcode, mad_opcode, mul_opcode, min_opcode,
      max_opcode, slt_opcode, sge_opcode, lrp_opcode, frc_opcode, abs_opcode,
      mova_opcode, cnd_opcode, cmp_opcode, dsx_opcode, dsy_opcode
    };
  }

  vector<uint32_t>
  vk_d3dbc_mask_inputs (span<const uint32_t> code)
  {
    // With a partial write mask an instruction discards some result
    // components but, as far as the bytecode is concerned, still reads the
    // corresponding source components. For an input register this means
    // reading components that the previous stage may not write.
    // vkd3d-shader seems to count such reads as uses when it builds the
    // varying map, so we rewrite the swizzles of componentwise instructions
    // to only read what is actually used.
    //
    vector<uint32_t> r (code.begin (), code.end ());

    for_each_instruction (
      r,
      [] (uint32_t op, span<uint32_t> ps)
      {
        if (ps.empty () || ranges::find (componentwise_opcodes, op) ==
                           ranges::end (componentwise_opcodes))
          return;

        const uint32_t mask ((ps[0] >> 16) & 0xf);

        if (mask == 0 || mask == 0xf)
          return;

        // Point the discarded components at the first written component.
        // Any written component would do since its value is read anyway.
        //
        const uint32_t first (static_cast<uint32_t> (countr_zero (mask)));

        // Skip the destination (and its address token, if any).
        //
        for (size_t i ((ps[0] & relative_bit) != 0 ? 2 : 1);
             i < ps.size ();
             ++i)
        {
          uint32_t& t (ps[i]);
          const bool relative ((t & relative_bit) != 0);

          if (register_type (t) == input_register)
          {
            const uint32_t sw ((t >> 16) & 0xff);
            uint32_t x (0);

            for (uint32_t c (0); c != 4; ++c)
            {
              const uint32_t k ((mask & (1u << c)) != 0 ? c : first);
              x |= ((sw >> (2 * k)) & 0x3) << (2 * c);
            }

            t = (t & ~(0xffu << 16)) | (x << 16);
          }

          if (relative) // Skip the address token.
            ++i;
        }
      });

    return r;
  }

  vector<uint32_t>
  vk_d3dbc_strip_comments (span<const uint32_t> code)
  {
    // Comments hold things like the constant table and debug information.
    // We don't use any of it and drop it all before handing the code to
    // vkd3d-shader.
    //
    // @@ Does vkd3d-shader actually need this? Worth trying without.
    //
    vector<uint32_t> r;
    r.reserve (code.size ());

    if (code.empty ())
      return r;

    r.push_back (code[0]);

    for (size_t i (1); i < code.size (); )
    {
      const uint32_t t (code[i]);

      if (t == end_token) // Has no length field.
      {
        r.push_back (t);
        break;
      }

      const uint32_t op (t & 0xffff);
      const size_t n (op == comment_opcode
                      ? (t >> 16) & 0x7fff
                      : (t >> 24) & 0xf);

      const size_t e (min (i + 1 + n, code.size ())); // Truncated comment.

      if (op != comment_opcode)
        r.insert (r.end (), code.begin () + i, code.begin () + e);

      i = e;
    }

    return r;
  }
}
