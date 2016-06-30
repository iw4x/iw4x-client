namespace Components
{
	class Toast : public Component
	{
	public:
		Toast();
		~Toast();
		const char* GetName() { return "Toast"; };

		static void Show(const char* image, const char* title, const char* description, int length);

	private:
		class UIToast
		{
		public:
			std::string Image;
			std::string Title;
			std::string Desc;
			int Length;
			int Start;
		};

		static void Handler();
		static void Draw(UIToast* toast);

		static std::queue<UIToast> Queue;
		static std::mutex Mutex;
	};
}
