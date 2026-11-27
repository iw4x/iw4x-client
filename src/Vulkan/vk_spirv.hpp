#pragma once

#include <span>
#include <vector>
#include <utility> // move()
#include <cstddef>
#include <cstdint>
#include <optional>

namespace Vulkan
{
  // SPIR-V module that we patch after the vkd3d-shader translation, for the
  // things that vkd3d-shader doesn't handle (see vk_shader_translate.cpp).
  //
  // Note that the patching only understands modules along the lines of what
  // vkd3d-shader generates.
  //
  class vk_spirv
  {
  public:
    struct instruction
    {
      std::uint32_t opcode;
      std::span<const std::uint32_t> operands; // Without the first word.
      std::size_t position;                    // Word index in the module.
    };

    // Throw invalid_argument if the words are not a well-formed SPIR-V
    // module.
    //
    explicit
    vk_spirv (std::vector<std::uint32_t>);

    const std::vector<std::uint32_t>&
    words () const noexcept {return words_;}

    std::vector<std::uint32_t>
    release () noexcept {return std::move (words_);}

    // Return the instructions in the module order. Note that the operands
    // point into the module, so they are only valid until it changes.
    //
    std::vector<instruction>
    instructions () const;

    // Change the array length in the uniform block with the set and binding
    // (see vk_d3dbc_scan() for background). Return false if we can't find
    // the block.
    //
    bool
    resize_block (std::uint32_t set,
                  std::uint32_t binding,
                  std::uint32_t length);

    // Make the fragment shader input with the location read vPos as
    // Direct3D 9 defines it, computed from FragCoord (see
    // vk_d3dbc_lower_misc()). Return false if we can't find the input.
    //
    bool
    lower_position (std::uint32_t location);

    // As above but for vFace, computed from FrontFacing.
    //
    bool
    lower_face (std::uint32_t location);

  private:
    struct input
    {
      std::uint32_t id;
      std::uint32_t vector;
      std::uint32_t component;
      std::uint32_t pointer;
      std::size_t decoration;
      std::size_t variable;
      std::size_t entry;
      std::size_t body;
    };

    std::optional<input>
    find_input (std::uint32_t location) const;

    std::uint32_t
    find_pointer (std::uint32_t storage,
                  std::uint32_t type,
                  std::vector<std::uint32_t>& declarations);

    std::uint32_t
    allocate () noexcept;

    void
    lower (const input&,
           std::uint32_t builtin,
           std::uint32_t pointer,
           std::uint32_t variable,
           std::vector<std::uint32_t> code,
           std::uint32_t value,
           std::vector<std::uint32_t> declarations);

  private:
    std::vector<std::uint32_t> words_;
  };

  // The SPIR-V binary format values that we use (see the SPIR-V
  // specification).
  //
  namespace spirv
  {
    const std::uint32_t magic (0x07230203);
    const std::size_t   header_size (5); // In words.
    const std::size_t   bound_word (3);  // Header word with the id bound.

    const std::uint32_t op_entry_point (15);
    const std::uint32_t op_function (54);
    const std::uint32_t op_label (248);
    const std::uint32_t op_decorate (71);
    const std::uint32_t op_member_decorate (72);
    const std::uint32_t op_type_bool (20);
    const std::uint32_t op_type_int (21);
    const std::uint32_t op_type_float (22);
    const std::uint32_t op_type_vector (23);
    const std::uint32_t op_type_array (28);
    const std::uint32_t op_type_struct (30);
    const std::uint32_t op_type_pointer (32);
    const std::uint32_t op_constant (43);
    const std::uint32_t op_constant_composite (44);
    const std::uint32_t op_variable (59);
    const std::uint32_t op_load (61);
    const std::uint32_t op_store (62);
    const std::uint32_t op_access_chain (65);
    const std::uint32_t op_in_bounds_access_chain (66);
    const std::uint32_t op_composite_construct (80);
    const std::uint32_t op_fsub (131);
    const std::uint32_t op_select (169);

    const std::uint32_t decoration_block (2);
    const std::uint32_t decoration_location (30);
    const std::uint32_t decoration_binding (33);
    const std::uint32_t decoration_descriptor_set (34);
    const std::uint32_t decoration_builtin (11);

    const std::uint32_t builtin_frag_coord (15);
    const std::uint32_t builtin_front_facing (17);

    const std::uint32_t storage_uniform_constant (0);
    const std::uint32_t storage_input (1);
    const std::uint32_t storage_uniform (2);
    const std::uint32_t storage_output (3);
    const std::uint32_t storage_private (6);
    const std::uint32_t storage_function (7);
  }
}
