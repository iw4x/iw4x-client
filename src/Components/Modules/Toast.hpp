#pragma once

namespace Components
{
	class Toast : public Component
	{
	public:
		Toast();
		~Toast();
		void preDestroy() override;

		typedef WinToastLib::WinToastTemplate Template;

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "Toast"; };
#endif

		static void Show(std::string image, std::string title, std::string description, int length);
		static bool ShowNative(const WinToastLib::WinToastTemplate& toast);

		static std::string GetIcon();

	private:
		class UIToast
		{
		public:
			std::string image;
			std::string title;
			std::string desc;
			int length;
			int start;
		};

		static void Handler();
		static void Draw(UIToast* toast);

		static std::queue<UIToast> Queue;
		static std::mutex Mutex;

		static WinToastLib::WinToastHandler* ToastHandler;
	};
}
