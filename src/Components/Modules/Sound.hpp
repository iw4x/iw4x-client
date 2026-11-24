#pragma once

extern "C"
{
  void Sound_ParseStreamHeaderStub ();

  bool Sound_ParseStreamHeader (void* header, const std::uint8_t* data, std::uint32_t size, const std::uint8_t* stream);
}

namespace Components
{
  class Sound : public Component
  {
  public:
    Sound();
  };
}
