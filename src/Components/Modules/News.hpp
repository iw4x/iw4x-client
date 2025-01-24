#pragma once

namespace Components
{
	class News : public Component
	{
	public:
		News();

		void preDestroy() override;

	private:
		static const char* GetNewsText();
	};
}
