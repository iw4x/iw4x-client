namespace Components
{
	class Discovery : public Component
	{
	public:
		Discovery();
		~Discovery();
		const char* GetName() { return "Discovery"; };

		static void Perform();
	};
}
