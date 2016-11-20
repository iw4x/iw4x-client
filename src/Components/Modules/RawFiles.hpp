namespace Components
{
	class RawFiles : public Component
	{
	public:
		RawFiles();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "RawFiles"; };
#endif

		static void* RawFiles::LoadModdableRawfileFunc(const char* filename);
	};
}
