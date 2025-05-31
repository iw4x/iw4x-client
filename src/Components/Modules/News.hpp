#pragma once
#include "rapidjson/document.h"

namespace Components
{
	class News : public Component
	{
	public:
		News();

		void preDestroy() override;

	private:
		static std::optional<std::string> ExtractStringByMemberName(const rapidjson::Document& document, const std::string& memberName);
		static void ProcessPopmenus(const rapidjson::Document& document);
		static std::optional<std::pair<std::string, std::string>> ExtractPopmenuItem(const rapidjson::Value& menuItem);
		static bool ShouldShowForRevision(const rapidjson::Value& revisions);
		static const char* GetNewsText();
	};
}
