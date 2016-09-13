namespace Components
{
	class ModelSurfs : public Component
	{
	public:
		ModelSurfs();
		~ModelSurfs();

	private:

		static std::map<void*, IUnknown*> BufferMap;

		static void ReleaseModelSurf(Game::XAssetHeader header);

		static void BeginRecover();
		static void EndRecover();

		static IUnknown* GetBuffer(void* buffer);
		static void SetBuffer(char streamHandle, void* buffer, IUnknown** bufferOut, int* offsetOut);

		static bool LoadXSurfaces(Game::XModel* model);
		static void XModelSurfsFixup(Game::XModel* model);

		static void GetIndexBaseStub();
		static void GetIndexBufferStub();
		static void GetIndexBufferStub2();
		static void GetVertexBufferStub();
	};
}
