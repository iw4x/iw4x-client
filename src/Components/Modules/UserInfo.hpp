#pragma once

namespace Components
{
	class UserInfo : public Component
	{
	public:
		UserInfo();

	private:
		using userInfoMap = std::unordered_map<std::string, std::string>;
		static std::unordered_map<int, userInfoMap> UserInfoOverrides;

		static void SV_GetUserInfo_Stub(int index, char* buffer, int bufferSize);

		static void ClearClientOverrides(int client);
		static void ClearAllOverrides();

		static void AddScriptMethods();
	};
}
