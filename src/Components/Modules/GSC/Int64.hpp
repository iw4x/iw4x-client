#pragma once

namespace Components::GSC
{
	class Int64 : public Component
	{
	public:
		Int64();

	private:
		using int64_OP = std::function<std::int64_t(std::int64_t, std::int64_t)>;
		using int64_Comp = std::function<bool(std::int64_t, std::int64_t)>;

		static std::unordered_map<std::string, int64_OP> Operations;
		static std::unordered_map<std::string, int64_Comp> Comparisons;

		static std::int64_t GetInt64Arg(unsigned int index, bool optional);
		static void AddFunctions();
	};
}
