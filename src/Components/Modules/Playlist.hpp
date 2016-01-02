namespace Components
{
	class Playlist : public Component
	{
	public:
		typedef void(*Callback)();

		Playlist();
		~Playlist();
		const char* GetName() { return "Playlist"; };

		static void LoadPlaylist();

	private:
		static std::string CurrentPlaylistBuffer;

		static DWORD StorePlaylistStub(const char** buffer);

		static void PlaylistRequest(Network::Address address, std::string data);
		static void PlaylistReponse(Network::Address address, std::string data);
	};
}
