#pragma once

namespace Components
{
	class News : public Component
	{
	public:
		News();

		void preDestroy() override;
		bool unitTest() override;

	private:
		static const char* GetNewsText();
	};
}
