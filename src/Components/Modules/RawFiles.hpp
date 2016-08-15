namespace Components
{
	class RawFiles : public Component
	{
	public:
		RawFiles();

#ifdef DEBUG
		const char* GetName() { return "RawFiles"; };
#endif

		static void* RawFiles::LoadModdableRawfileFunc(const char* filename);
	};
}
