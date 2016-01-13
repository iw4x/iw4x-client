namespace Components
{
	class StructuredData : public Component
	{
	public:
		StructuredData();
		~StructuredData();
		const char* GetName() { return "StructuredData"; };

	private:
		static void DumpDataDef(Game::structuredDataDef_t* dataDef);
	};
}
