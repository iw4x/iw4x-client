namespace Components
{
	class Download : public Component
	{
	public:
		Download();
		~Download();
		const char* GetName() { return "Download"; };

	private:
		static mg_mgr Mgr;

		static void EventHandler(mg_connection *nc, int ev, void *ev_data);
	};
}
