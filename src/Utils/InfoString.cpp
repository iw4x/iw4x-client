#include "InfoString.hpp"

namespace Utils
{
  std::size_t InfoString::KeyHash::operator()(const std::string& key) const
  {
    return std::hash<std::string>()(String::ToLower(key));
  }

  bool InfoString::KeyEqual::operator()(const std::string& lhs, const std::string& rhs) const
  {
    return lhs.size() == rhs.size() && _strnicmp(lhs.data(), rhs.data(), lhs.size()) == 0;
  }

  InfoString::InfoString(const std::string& buffer)
  {
    this->parse(buffer);
  }

  void InfoString::set(const std::string& key, const std::string& value)
  {
    // Remove any existing key that only differs in case, so build() never
    // emits both.
    //
    this->keyValuePairs_.erase(key);
    this->keyValuePairs_[key] = value;
  }

  void InfoString::remove(const std::string& key)
  {
    this->keyValuePairs_.erase(key);
  }

  std::string InfoString::get(const std::string& key) const
  {
    if (const auto value = this->keyValuePairs_.find(key); value != this->keyValuePairs_.end())
    {
      return value->second;
    }

    return {};
  }

  void InfoString::parse(std::string buffer)
  {
    if (buffer[0] == '\\')
    {
      buffer = buffer.substr(1);
    }

    const auto keyValues = String::Split(buffer, '\\');
    for (std::size_t i = 0; !keyValues.empty() && i < (keyValues.size() - 1); i += 2)
    {
      const auto& key = keyValues[i];
      const auto& value = keyValues[i + 1];

      // Keep the first match ignoring case, the same as the engine's
      // Info_ValueForKey() does.
      //
      if (!this->keyValuePairs_.contains(key))
      {
        this->keyValuePairs_[key] = value;
      }
    }
  }

  std::string InfoString::build() const
  {
    std::string infoString;
    for (const auto& [key, value] : this->keyValuePairs_)
    {
      infoString.append("\\");
      infoString.append(key);
      infoString.append("\\");
      infoString.append(value);
    }

    return infoString;
  }

#ifdef _DEBUG
  void InfoString::dump()
  {
    for (const auto& [key, value] : this->keyValuePairs_)
    {
      OutputDebugStringA(String::VA("%s: %s\n", key.data(), value.data()));
    }
  }
#endif

  nlohmann::json InfoString::to_json() const
  {
    nlohmann::json result = nlohmann::json::object();
    for (const auto& [key, value] : this->keyValuePairs_)
    {
      result[key] = value;
    }

    return result;
  }
}
