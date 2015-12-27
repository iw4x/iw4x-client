namespace Components
{
	class Console : public Component
	{
	public:
		Console();
		const char* GetName() { return "Console"; };

	private:
		static void ToggleConsole();
		static char** GetAutoCompleteFileList(const char *path, const char *extension, Game::FsListBehavior_e behavior, int *numfiles, int allocTrackType);
	};
}
