namespace Components
{
	class News : public Component
	{
	public:
		News();
		~News();
		const char* GetName() { return "News"; };
		bool UnitTest();

	private:
		static std::thread* Thread;
	};
}
