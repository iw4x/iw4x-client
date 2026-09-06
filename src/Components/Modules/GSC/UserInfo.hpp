#pragma once

namespace Components::GSC
{
	class UserInfo : public Component
	{
	public:
		UserInfo();

		static void ClearClientOverrides(int clientNum);
		static void ClearAllOverrides();

	private:
		using userInfoMap = std::unordered_map<std::string, std::string>;
		static std::unordered_map<std::uint64_t, userInfoMap> UserInfoOverrides;

		static std::uint64_t GetOverrideKey(int clientNum);

		static void SV_GetUserInfo_Stub(int index, char* buffer, int bufferSize);

		static void AddScriptMethods();
	};
}
