#pragma once

namespace Components
{
	class ZoneConvert : public Component
	{
	public:
		ZoneConvert();

    static std::string
    SearchPath (std::string_view group);
  };
}
