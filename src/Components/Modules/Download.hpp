namespace Components
{
	class Download : public Component
	{
	public:
		Download();
		~Download();

#ifdef DEBUG
		const char* GetName() { return "Download"; };
#endif

	private:
		static mg_mgr Mgr;

		static void EventHandler(mg_connection *nc, int ev, void *ev_data);
		static void ListHandler(mg_connection *nc, int ev, void *ev_data);
		static void FileHandler(mg_connection *nc, int ev, void *ev_data);
		static void InfoHandler(mg_connection *nc, int ev, void *ev_data);

		static bool IsClient(mg_connection *nc);
		static Game::client_t* GetClient(mg_connection *nc);
		static void Forbid(mg_connection *nc);
	};
}
