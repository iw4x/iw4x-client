#define Q_IsColorString( p )  ( ( p ) && *( p ) == '^' && *( ( p ) + 1 ) && isdigit( *( ( p ) + 1 ) ) ) // ^[0-9]

namespace Components
{
	class Colors : public Component
	{
	public:
		Colors();
		const char* GetName() { return "Colors"; };

		static void Strip(const char* in, char* out, int max);

	private:
		static Dvar::Var NewColors;

		static void ClientUserinfoChanged(int length);
		static char* GetClientName(int localClientNum, int index, char *buf, size_t size);

		static void UpdateColorTable();
	};
}
