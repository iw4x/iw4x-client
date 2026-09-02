#include "Store.hpp"

#include "../Types.hpp"

#include "Validate.hpp"

#include <fstream>
#include <format>
#include <string>
#include <system_error>

namespace Controller
{
  namespace calibration
  {
    namespace
    {
      constexpr const char* magic {"iw4x-controller-calibration"};

      constexpr int family_max {
        static_cast<int> (Controller::family::dualsense_edge)};

      constexpr int source_max {static_cast<int> (value_source::user)};
    }

    store::
    store (const context& ctx, std::filesystem::path directory)
      : ctx_ (ctx), dir_ (std::move (directory))
    {
    }

    std::filesystem::path
    store::
    file_for (Controller::family family, std::optional<uint64_t> device_key) const
    {
      std::string name (device_key
        ? std::format ("controller-{}-{:016x}.cal",
                       to_string (family), *device_key)
        : std::format ("controller-{}.cal", to_string (family)));

      return dir_ / name;
    }

    bool
    store::
    save (const profile& p) const
    {
      std::string why;
      if (!validate (p, why))
      {
        ctx_.report (severity::warning, facility::calibration,
                     errc::calibration_invalid,
                     "refusing to save an invalid calibration profile: " + why);
        return false;
      }

      std::error_code ec;
      std::filesystem::create_directories (dir_, ec);

      const std::filesystem::path file (file_for (p.family, p.device_key));
      std::ofstream os (file, std::ios::trunc);
      if (!os)
      {
        ctx_.report (severity::warning, facility::calibration,
                     errc::transport_failure,
                     "could not open calibration profile for writing");
        return false;
      }

      os.precision (9);

      os << magic << ' ' << p.version << '\n';
      os << "family " << static_cast<int> (p.family) << '\n';
      os << "source " << static_cast<int> (p.source) << '\n';
      os << "device_key " << (p.device_key ? 1 : 0) << ' '
         << (p.device_key ? *p.device_key : uint64_t {0}) << '\n';

      for (size_t i (0); i < stick_count; ++i)
      {
        const stick_calibration& s (p.sticks[i]);
        os << "stick " << i << ' '
           << s.center_x << ' ' << s.center_y << ' '
           << s.range_x << ' ' << s.range_y << ' '
           << s.drift_threshold << '\n';
      }

      for (size_t i (0); i < trigger_count; ++i)
      {
        const trigger_calibration& t (p.triggers[i]);
        os << "trigger " << i << ' ' << t.min << ' ' << t.max << '\n';
      }

      os << "motion "
         << p.motion.gyro_bias.x << ' ' << p.motion.gyro_bias.y << ' '
         << p.motion.gyro_bias.z << ' '
         << p.motion.accel_bias.x << ' ' << p.motion.accel_bias.y << ' '
         << p.motion.accel_bias.z << ' '
         << p.motion.gyro_scale << ' ' << p.motion.accel_scale << '\n';

      os << "smoothing " << p.smoothing << '\n';

      return static_cast<bool> (os);
    }

    std::optional<profile>
    store::
    load (Controller::family family, std::optional<uint64_t> device_key) const
    {
      const std::filesystem::path file (file_for (family, device_key));

      std::ifstream is (file);
      if (!is)
        return std::nullopt;

      auto reject = [this] (const char* why) -> std::optional<profile>
      {
        ctx_.report (severity::warning, facility::calibration,
                     errc::calibration_invalid,
                     std::string ("calibration profile rejected: ") + why);
        return std::nullopt;
      };

      std::string token;
      unsigned version (0);
      if (!(is >> token >> version) || token != magic)
        return reject ("bad header");

      if (version == 0 || version > profile::current_version)
        return reject ("unsupported version");

      profile p;
      p.version = static_cast<uint16_t> (version);

      int family_i (0);
      int source_i (0);
      int has_key (0);
      uint64_t key_value (0);

      if (!(is >> token >> family_i) || token != "family" ||
          family_i < 0 || family_i > family_max)
        return reject ("bad family");

      if (!(is >> token >> source_i) || token != "source" ||
          source_i < 0 || source_i > source_max)
        return reject ("bad source");

      if (!(is >> token >> has_key >> key_value) || token != "device_key")
        return reject ("bad device key");

      p.family = static_cast<Controller::family> (family_i);
      p.source = static_cast<value_source> (source_i);
      if (has_key != 0)
        p.device_key = key_value;

      for (size_t i (0); i < stick_count; ++i)
      {
        size_t idx (0);
        stick_calibration s;
        if (!(is >> token >> idx >> s.center_x >> s.center_y >>
              s.range_x >> s.range_y >> s.drift_threshold) ||
            token != "stick" || idx != i)
          return reject ("bad stick record");

        p.sticks[i] = s;
      }

      for (size_t i (0); i < trigger_count; ++i)
      {
        size_t idx (0);
        trigger_calibration t;
        if (!(is >> token >> idx >> t.min >> t.max) ||
            token != "trigger" || idx != i)
          return reject ("bad trigger record");

        p.triggers[i] = t;
      }

      if (!(is >> token >> p.motion.gyro_bias.x >> p.motion.gyro_bias.y >>
            p.motion.gyro_bias.z >> p.motion.accel_bias.x >>
            p.motion.accel_bias.y >> p.motion.accel_bias.z >>
            p.motion.gyro_scale >> p.motion.accel_scale) ||
          token != "motion")
        return reject ("bad motion record");

      if (!(is >> token >> p.smoothing) || token != "smoothing")
        return reject ("bad smoothing record");

      if (p.family != family || p.device_key != device_key)
        return reject ("family or device key mismatch");

      std::string why;
      if (!validate (p, why))
        return reject (why.c_str ());

      return p;
    }
  }
}
