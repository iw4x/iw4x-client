namespace Components
{
	class Maps : public Component
	{
	public:
		Maps();
		const char* GetName() { return "Maps"; };

	private:
		static void* WorldMP;
		static void* WorldSP;

		static void GetBSPName(char* buffer, size_t size, const char* format, const char* mapname);
	};
}
