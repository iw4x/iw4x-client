#define Q_IsColorString( p )  ( ( p ) && *( p ) == '^' && *( ( p ) + 1 ) && isdigit( *( ( p ) + 1 ) ) ) // ^[0-9]


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
