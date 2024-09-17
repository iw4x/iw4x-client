#pragma once

namespace Components
{
	class Flags : public Component
	{
	public:
		static bool HasFlag(const std::string& flag);

	private:
		static std::vector<std::string> EnabledFlags;

		static void ParseFlags();
	};
}
