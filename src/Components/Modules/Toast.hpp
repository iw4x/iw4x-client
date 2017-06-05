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

		static void Show(std::string image, std::string title, std::string description, int length, Utils::Slot<void()> callback = Utils::Slot<void()>());
		static void Show(Game::Material* material, std::string title, std::string description, int length, Utils::Slot<void()> callback = Utils::Slot<void()>());
		static bool ShowNative(const WinToastLib::WinToastTemplate& toast);

		static std::string GetIcon();

	private:
		class UIToast
		{
		public:
			Game::Material* image;
			std::string title;
			std::string desc;
			int length;
			int start;
			Utils::Slot<void()> callback;
		};

		class WinToastHandler: public WinToastLib::IWinToastHandler
		{
		public:
			void toastActivated() const override {};
			void toastDismissed(WinToastLib::IWinToastHandler::WinToastDismissalReason /*state*/) const override {};
			void toastFailed() const override {};
		};

		static void Handler();
		static void Draw(UIToast* toast);

		static std::queue<UIToast> Queue;
		static std::mutex Mutex;

		static WinToastHandler* ToastHandler;
	};
}
