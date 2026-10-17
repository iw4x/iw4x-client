#pragma once

namespace Utils
{
  class InfoString
  {
  public:
    InfoString() = default;
    explicit InfoString(const std::string& buffer);

    void set(const std::string& key, const std::string& value);
    void remove(const std::string& key);

    [[nodiscard]] std::string get(const std::string& key) const;
    [[nodiscard]] std::string build() const;

#ifdef _DEBUG
    void dump();
#endif

    [[nodiscard]] nlohmann::json to_json() const;

  private:
    // We compare keys ignoring ASCII case, like the engine's
    // Info_ValueForKey() does, but keep their original spelling so that
    // build() produces the same keys that were set.
    //
    struct KeyHash
    {
      std::size_t operator()(const std::string& key) const;
    };

    struct KeyEqual
    {
      bool operator()(const std::string& lhs, const std::string& rhs) const;
    };

    std::unordered_map<std::string, std::string, KeyHash, KeyEqual> keyValuePairs_;

    void parse(std::string buffer);
  };
}
