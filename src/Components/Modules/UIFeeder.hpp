namespace Components
{
	class UIFeeder : public Component
	{
	public:
		typedef unsigned int(__cdecl * GetItemCount_t)();
		typedef const char* (__cdecl * GetItemText_t)(unsigned int index, int column);
		typedef void(__cdecl * Select_t)(unsigned int index);

		struct Callbacks
		{
			GetItemCount_t GetItemCount;
			GetItemText_t GetItemText;
			Select_t Select;
		};

		UIFeeder();
		~UIFeeder();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* GetName() { return "UIFeeder"; };
#endif

		static void Add(float feeder, GetItemCount_t itemCountCb, GetItemText_t itemTextCb, Select_t selectCb);

	private:
		struct Container
		{
			float Feeder;
			int Index;
			int Column;
		};

		static Container Current;

		static void GetItemCountStub();
		static unsigned int GetItemCount();

		static void GetItemTextStub();
		static const char* GetItemText();

		static void SetItemSelectionStub();
		static bool SetItemSelection();

		static bool CheckFeeder();

		static void MouseEnterStub();
		static void MouseSelectStub();
		static void HandleKeyStub();
		static void PlaySoundStub();

		static std::map<float, Callbacks> Feeders;
	};
}
