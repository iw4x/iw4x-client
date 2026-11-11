#pragma once

namespace Components
{
  class WineD3D : public Component
  {
  public:
    WineD3D();

  private:
    static std::size_t RaiseMappingBudget(unsigned char* module);
  };
}
