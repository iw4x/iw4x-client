#pragma once

#include "../Types.hpp"

#include "../Context.hpp"
#include "Profile.hpp"

namespace Controller
{
  namespace calibration
  {
    class store
    {
    public:
      store (const context&, std::filesystem::path directory);

      std::optional<profile>
      load (Controller::family, std::optional<uint64_t> device_key) const;

      bool
      save (const profile&) const;

      const std::filesystem::path&
      directory () const noexcept {return dir_;}

    private:
      std::filesystem::path
      file_for (Controller::family, std::optional<uint64_t> device_key) const;

      const context& ctx_;
      std::filesystem::path dir_;
    };
  }
}
