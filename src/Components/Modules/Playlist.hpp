namespace Components
{
	class Playlist : public Component
	{
	public:
		typedef void(*Callback)();

		Playlist();
		~Playlist();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* GetName() { return "Playlist"; };
#endif

		static void LoadPlaylist();

		static std::string ReceivedPlaylistBuffer;

	private:
		static std::string CurrentPlaylistBuffer;
		static DWORD StorePlaylistStub(const char** buffer);

		static void PlaylistRequest(Network::Address address, std::string data);
		static void PlaylistReponse(Network::Address address, std::string data);
	};
}
