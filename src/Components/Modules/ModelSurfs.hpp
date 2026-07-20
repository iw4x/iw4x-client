#pragma once

namespace Components
{
	class ModelSurfs : public Component
	{
	public:
		ModelSurfs();
		~ModelSurfs();

		static Game::XModelSurfs* CloneAndScaleSurfaces(const Game::XModelSurfs* source, const std::string& name, float scale);
		static void UpdateScaledSurfaces(Game::XModelSurfs* target, const Game::XModelSurfs* source, float scale);

	private:
		static std::unordered_map<void*, IUnknown*> BufferMap;
		static std::unordered_map<std::string, Game::CModelAllocData*> AllocMap;

		static void ReleaseModelSurf(Game::XAssetHeader header);

		static void BeginRecover();
		static void EndRecover();

		static IUnknown* GetBuffer(void* buffer);
		static void SetBuffer(char streamHandle, void* buffer, IUnknown** bufferOut, int* offsetOut);

		static void CreateBuffers(Game::XModelSurfs* surfs);
		static Game::XModelSurfs* LoadXModelSurfaces(const std::string& name);
		static bool LoadSurfaces(Game::XModel* model);
		static void XModelSurfsFixup(Game::XModel* model);

		static void GetIndexBaseStub();
		static void GetIndexBufferStub();
		static void GetIndexBufferStub2();
		static void GetVertexBufferStub();
	};
}
