#include <Vulkan/vk_spirv.hpp>

#include <bit>       // bit_cast()
#include <utility>   // move()
#include <optional>
#include <cassert>
#include <stdexcept> // invalid_argument

using namespace std;

namespace Vulkan
{
  vk_spirv::
  vk_spirv (vector<uint32_t> ws)
    : words_ (move (ws))
  {
    if (words_.size () < spirv::header_size || words_[0] != spirv::magic)
      throw invalid_argument ("not a SPIR-V module");

    // Each instruction starts with a word that has the word count in the
    // high half and the opcode in the low half. Make sure the counts add up
    // so that from now on we can walk the instructions without checking.
    //
    for (size_t i (spirv::header_size); i != words_.size (); )
    {
      size_t n (words_[i] >> 16);

      if (n == 0 || n > words_.size () - i)
        throw invalid_argument ("truncated SPIR-V instruction");

      i += n;
    }
  }

  vector<vk_spirv::instruction> vk_spirv::
  instructions () const
  {
    vector<instruction> r;

    for (size_t i (spirv::header_size); i != words_.size (); )
    {
      size_t n (words_[i] >> 16);

      r.push_back (instruction {words_[i] & 0xffff,
                                span (words_.data () + i + 1, n - 1),
                                i});
      i += n;
    }

    return r;
  }

  bool vk_spirv::
  resize_block (uint32_t set, uint32_t binding, uint32_t length)
  {
    // The block that vkd3d-shader generates for a constant set looks along
    // these lines:
    //
    // OpDecorate %cb DescriptorSet 0
    // OpDecorate %cb Binding 3
    // ...
    // %n = OpConstant %uint 32
    // %a = OpTypeArray %v4float %n
    // %s = OpTypeStruct %a
    // %p = OpTypePointer Uniform %s
    // %cb = OpVariable %p Uniform
    //
    // We find the variable by its set and binding (no set decoration means
    // set 0) and follow it to the length constant.
    //
    vector<instruction> is (instructions ());

    optional<uint32_t> var;
    {
      vector<pair<uint32_t, uint32_t>> bindings;
      vector<pair<uint32_t, uint32_t>> sets;

      for (const instruction& i: is)
      {
        if (i.opcode != spirv::op_decorate || i.operands.size () != 3)
          continue;

        if (i.operands[1] == spirv::decoration_binding)
          bindings.emplace_back (i.operands[0], i.operands[2]);
        else if (i.operands[1] == spirv::decoration_descriptor_set)
          sets.emplace_back (i.operands[0], i.operands[2]);
      }

      for (const pair<uint32_t, uint32_t>& b: bindings)
      {
        if (b.second != binding)
          continue;

        uint32_t s (0);
        for (const pair<uint32_t, uint32_t>& x: sets)
        {
          if (x.first == b.first)
            s = x.second;
        }

        if (s == set)
        {
          var = b.first;
          break;
        }
      }
    }

    if (!var)
      return false;

    auto find = [&is] (uint32_t opcode, size_t result, uint32_t id)
      -> const instruction*
    {
      for (const instruction& i: is)
      {
        if (i.opcode == opcode            &&
            i.operands.size () > result   &&
            i.operands[result] == id)
          return &i;
      }

      return nullptr;
    };

    const instruction* v (find (spirv::op_variable, 1, *var));
    if (v == nullptr || v->operands[2] != spirv::storage_uniform)
      return false;

    const instruction* p (find (spirv::op_type_pointer, 0, v->operands[0]));
    if (p == nullptr || p->operands.size () != 3)
      return false;

    const instruction* s (find (spirv::op_type_struct, 0, p->operands[2]));
    if (s == nullptr || s->operands.size () < 2)
      return false;

    const instruction* a (find (spirv::op_type_array, 0, s->operands[1]));
    if (a == nullptr || a->operands.size () != 3)
      return false;

    const instruction* c (find (spirv::op_constant, 1, a->operands[2]));
    if (c == nullptr || c->operands.size () != 3)
      return false;

    if (c->operands[2] == length)
      return true;

    // Something else may use the constant, so we declare a new one right
    // before the array type and point the array at it.
    //
    uint32_t id (words_[spirv::bound_word]++);
    uint32_t type (c->operands[0]);
    size_t ap (a->position);

    words_[ap + 3] = id;

    words_.insert (words_.begin () + ap,
                   {(4u << 16) | spirv::op_constant, type, id, length});
    return true;
  }

  bool vk_spirv::
  lower_position (uint32_t location)
  {
    optional<input> in (find_input (location));
    if (!in)
      return false;

    // Direct3D 9 has the pixel centers at integer coordinates and Vulkan at
    // half-integer ones, so the input value is:
    //
    // %center = OpLoad %v4float %frag_coord
    // %corner = OpFSub %v4float %center (0.5, 0.5, 0, 0)
    //
    uint32_t variable (allocate ());
    uint32_t half (allocate ());
    uint32_t zero (allocate ());
    uint32_t offset (allocate ());
    uint32_t center (allocate ());
    uint32_t corner (allocate ());

    uint32_t f (in->component);
    uint32_t v (in->vector);

    lower (*in,
           spirv::builtin_frag_coord,
           in->pointer,
           variable,
           {(4u << 16) | spirv::op_load, v, center, variable,
            (5u << 16) | spirv::op_fsub, v, corner, center, offset},
           corner,
           {(4u << 16) | spirv::op_constant, f, half,
            bit_cast<uint32_t> (0.5f),
            (4u << 16) | spirv::op_constant, f, zero,
            bit_cast<uint32_t> (0.0f),
            (7u << 16) | spirv::op_constant_composite, v, offset,
            half, half, zero, zero});
    return true;
  }

  bool vk_spirv::
  lower_face (uint32_t location)
  {
    optional<input> in (find_input (location));
    if (!in)
      return false;

    // Direct3D 9 vFace is a float, positive for front faces and negative for
    // back faces. So the input value is:
    //
    // %front = OpLoad %bool %front_facing
    // %sign  = OpSelect %float %front 1.0 -1.0
    // %value = OpCompositeConstruct %v4float %sign %sign %sign %sign
    //
    // A module may only declare one bool type, so we reuse the existing one
    // if any. It has to come before our declarations, which go before the
    // input variable (see lower()), so we move it if it comes after.
    //
    vector<uint32_t> ds;

    uint32_t boolean (0);
    size_t position (0);
    for (const instruction& i: instructions ())
    {
      if (i.opcode == spirv::op_type_bool)
      {
        boolean = i.operands[0];
        position = i.position;
      }
    }

    if (boolean == 0)
      boolean = allocate ();

    if (position == 0 || position > in->variable)
    {
      if (position != 0)
      {
        // The body follows the type and so moves up with the erase.
        //
        assert (in->body > position);

        words_.erase (words_.begin () + position,
                      words_.begin () + position + 2);
        in->body -= 2;
      }

      ds.insert (ds.end (), {(2u << 16) | spirv::op_type_bool, boolean});
    }

    uint32_t pointer (find_pointer (spirv::storage_input, boolean, ds));

    uint32_t variable (allocate ());
    uint32_t one (allocate ());
    uint32_t minus_one (allocate ());
    uint32_t front (allocate ());
    uint32_t sign (allocate ());
    uint32_t value (allocate ());

    uint32_t f (in->component);
    uint32_t v (in->vector);

    ds.insert (ds.end (),
               {(4u << 16) | spirv::op_constant, f, one,
                bit_cast<uint32_t> (1.0f),
                (4u << 16) | spirv::op_constant, f, minus_one,
                bit_cast<uint32_t> (-1.0f)});

    lower (*in,
           spirv::builtin_front_facing,
           pointer,
           variable,
           {(4u << 16) | spirv::op_load, boolean, front, variable,
            (6u << 16) | spirv::op_select, f, sign, front, one, minus_one,
            (7u << 16) | spirv::op_composite_construct, v, value,
            sign, sign, sign, sign},
           value,
           move (ds));
    return true;
  }

  optional<vk_spirv::input> vk_spirv::
  find_input (uint32_t location) const
  {
    // We only handle what vkd3d-shader generates for a d3dbc input, which
    // looks along these lines:
    //
    // OpEntryPoint Fragment %main "main" ... %in ...
    // OpDecorate %in Location 1
    // ...
    // %p = OpTypePointer Input %v4float
    // %in = OpVariable %p Input
    // ...
    // %main = OpFunction %void None %f
    // %l = OpLabel
    // ...                                  (function variables)
    // ...                                  (body)
    //
    vector<instruction> is (instructions ());

    auto find = [&is] (uint32_t opcode, size_t index, uint32_t value)
      -> const instruction*
    {
      for (const instruction& i: is)
      {
        if (i.opcode == opcode           &&
            i.operands.size () > index   &&
            i.operands[index] == value)
          return &i;
      }

      return nullptr;
    };

    const instruction* d (nullptr);
    const instruction* v (nullptr);

    for (const instruction& i: is)
    {
      const auto& o (i.operands);

      if (i.opcode != spirv::op_decorate     ||
          o.size () != 3                     ||
          o[1] != spirv::decoration_location ||
          o[2] != location)
        continue;

      v = find (spirv::op_variable, 1, o[0]);
      if (v != nullptr && v->operands[2] == spirv::storage_input)
      {
        d = &i;
        break;
      }
    }

    if (d == nullptr)
      return nullopt;

    const instruction* p (find (spirv::op_type_pointer, 0, v->operands[0]));
    if (p == nullptr || p->operands.size () != 3)
      return nullopt;

    const instruction* t (find (spirv::op_type_vector, 0, p->operands[2]));
    if (t == nullptr || t->operands.size () != 3 || t->operands[2] != 4)
      return nullopt;

    if (find (spirv::op_type_float, 0, t->operands[1]) == nullptr)
      return nullopt;

    const instruction* e (nullptr);
    for (const instruction& i: is)
    {
      if (i.opcode == spirv::op_entry_point)
      {
        if (e != nullptr)
          return nullopt;

        e = &i;
      }
    }

    if (e == nullptr || e->operands.size () < 3)
      return nullopt;

    optional<size_t> body;
    {
      bool function (false);
      bool label (false);

      for (const instruction& i: is)
      {
        if (!function)
          function = (i.opcode == spirv::op_function &&
                      i.operands.size () > 1         &&
                      i.operands[1] == e->operands[1]);
        else if (!label)
          label = (i.opcode == spirv::op_label);
        else if (i.opcode != spirv::op_variable)
        {
          body = i.position;
          break;
        }
      }
    }

    if (!body)
      return nullopt;

    return input {d->operands[0],
                  t->operands[0],
                  t->operands[1],
                  p->operands[0],
                  d->position,
                  v->position,
                  e->position,
                  *body};
  }

  uint32_t vk_spirv::
  find_pointer (uint32_t storage,
                uint32_t type,
                vector<uint32_t>& declarations)
  {
    // See if the pointer type is already declared, also among the
    // declarations that are yet to be inserted.
    //
    for (const instruction& i: instructions ())
    {
      const auto& o (i.operands);

      if (i.opcode == spirv::op_type_pointer &&
          o.size () == 3                     &&
          o[1] == storage                    &&
          o[2] == type)
        return o[0];
    }

    for (size_t i (0); i < declarations.size (); )
    {
      const uint32_t* w (declarations.data () + i);
      size_t n (w[0] >> 16);

      if ((w[0] & 0xffff) == spirv::op_type_pointer &&
          n == 4                                    &&
          w[2] == storage                           &&
          w[3] == type)
        return w[1];

      i += n != 0 ? n : 1;
    }

    uint32_t r (allocate ());
    declarations.insert (declarations.end (),
                         {(4u << 16) | spirv::op_type_pointer,
                          r, storage, type});
    return r;
  }

  uint32_t vk_spirv::
  allocate () noexcept
  {
    return words_[spirv::bound_word]++;
  }

  void vk_spirv::
  lower (const input& in,
         uint32_t builtin,
         uint32_t pointer,
         uint32_t variable,
         vector<uint32_t> code,
         uint32_t value,
         vector<uint32_t> declarations)
  {
    // Our goal here is to make the shader read the built-in where it read
    // the input, without changing anything else. So we turn the input
    // variable into a private variable, which the entry point fills from the
    // built-in before anything else runs. With the earlier example (see
    // find_input()), the result looks along these lines:
    //
    // OpEntryPoint Fragment %main "main" ... %builtin ...
    // OpDecorate %builtin BuiltIn FragCoord
    // ...
    // <declarations>
    // %builtin = OpVariable %pointer Input
    // %in = OpVariable %private Private
    // ...
    // %l = OpLabel
    // ...                                  (function variables)
    // <code>
    // OpStore %in %value
    // ...                                  (body)
    //
    vector<uint32_t>& ds (declarations);

    ds.insert (ds.end (), {(4u << 16) | spirv::op_variable,
                           pointer, variable, spirv::storage_input});

    // Variable.
    //
    words_[in.variable + 1] = find_pointer (spirv::storage_private,
                                            in.vector,
                                            ds);
    words_[in.variable + 3] = spirv::storage_private;

    // Access chains. The pointers that they produce change storage as well.
    //
    {
      vector<instruction> is (instructions ());

      auto pointee = [&is] (uint32_t pointer) -> uint32_t
      {
        for (const instruction& i: is)
        {
          if (i.opcode == spirv::op_type_pointer && i.operands[0] == pointer)
            return i.operands[2];
        }

        return 0;
      };

      for (const instruction& i: is)
      {
        if ((i.opcode != spirv::op_access_chain            &&
             i.opcode != spirv::op_in_bounds_access_chain) ||
            i.operands.size () < 3                         ||
            i.operands[2] != in.id)
          continue;

        uint32_t t (pointee (i.operands[0]));
        assert (t != 0);

        words_[i.position + 1] = find_pointer (spirv::storage_private, t, ds);
      }
    }

    // Decoration (same length, so rewritten in place).
    //
    words_[in.decoration + 1] = variable;
    words_[in.decoration + 2] = spirv::decoration_builtin;
    words_[in.decoration + 3] = builtin;

    // Entry point interface. It follows the name, which is a nul-terminated
    // string padded to whole words. Note that before SPIR-V 1.4 the
    // interface may not list private variables.
    //
    {
      size_t n (words_[in.entry] >> 16);
      size_t i (in.entry + 3);

      auto terminal = [] (uint32_t w)
      {
        return (w & 0x000000ff) == 0 || (w & 0x0000ff00) == 0 ||
               (w & 0x00ff0000) == 0 || (w & 0xff000000) == 0;
      };

      while (i != in.entry + n && !terminal (words_[i]))
        ++i;

      for (++i; i < in.entry + n; ++i)
      {
        if (words_[i] == in.id)
          words_[i] = variable;
      }
    }

    // Code and declarations. Note that the code goes in first since it comes
    // after the variable and inserting the declarations would shift it.
    //
    code.insert (code.end (),
                 {(3u << 16) | spirv::op_store, in.id, value});

    words_.insert (words_.begin () + in.body, code.begin (), code.end ());
    words_.insert (words_.begin () + in.variable, ds.begin (), ds.end ());
  }
}
