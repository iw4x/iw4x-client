#define Q_IsColorString( p )  ( ( p ) && *( p ) == '^' && *( ( p ) + 1 ) && isdigit( *( ( p ) + 1 ) ) ) // ^[0-9]

namespace Components
{
	class Colors : public Component
	{
	public:
		Colors();
		const char* GetName() { return "Colors"; };

		static Dvar::Var NewColors;

		static void ClientUserinfoChanged(int length);
		static char* CL_GetClientName(int a1, int a2, char* buffer, size_t _length);

		static void UpdateColorTable();

		static void Strip(const char* in, char* out, int max);
	};
}
