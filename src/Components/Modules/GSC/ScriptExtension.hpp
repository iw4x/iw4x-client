#pragma once

namespace Components::GSC
{
	class ScriptExtension : public Component
	{
	public:
		ScriptExtension();

		static const char* GetCodePosForParam(int index);

	private:
		static std::unordered_map<const char*, const char*> ReplacedFunctions;
		static const char* ReplacedPos;

		static void GetReplacedPos(const char* pos);
		static void SetReplacedPos(const char* what, const char* with);
		static void VMExecuteInternalStub();

		static void AddFunctions();
	};
}
