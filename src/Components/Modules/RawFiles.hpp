namespace Components
{
	class RawFiles : public Component
	{
	public:
		RawFiles();
		const char* GetName() { return "RawFiles"; };

		static void* RawFiles::LoadModdableRawfileFunc(const char* filename);
	};
}
