#pragma once

namespace Components
{
  class Presence : public Component
  {
  public:
    Presence();

  private:
    static void Report(const std::string& server);
  };
}
