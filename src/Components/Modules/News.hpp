namespace Components
{
	class News : public Component
	{
	public:
		News();
		~News();
		const char* GetName() { return "News"; };

	private:
		static std::thread* Thread;
	};
}
