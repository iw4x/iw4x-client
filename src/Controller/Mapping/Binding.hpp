#pragma once

#include "../Types.hpp"

#include "../Support/FunctionRef.hpp"

#include "Key.hpp"
#include "Logical.hpp"

namespace Controller
{
  namespace mapping
  {
    class binding_table
    {
    public:
      void
      bind (engine_key, std::string command);

      void
      bind (engine_key, action);

      void
      unbind (engine_key) noexcept;

      void
      clear () noexcept;

      const std::string*
      command_for (engine_key) const noexcept;

      void
      for_each (function_ref<void (engine_key, const std::string&)>) const;

      size_t
      size () const noexcept;

    private:
      static constexpr size_t count {engine_key_count};

      std::array<std::string, count> commands_;
    };

    void
    apply_button_layout (binding_table&, std::string_view name);
  }
}
