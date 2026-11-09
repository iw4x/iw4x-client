#pragma once

extern "C"
{
  void ModelSurfs_GetIndexBufferStub();
  void ModelSurfs_GetIndexBufferStub2();
  void ModelSurfs_GetIndexBaseStub();
  void ModelSurfs_GetVertexBufferStub();

  void ModelSurfs_SetBuffer(char streamHandle, void* buffer, IUnknown** bufferOut, int* offsetOut);
  IUnknown* ModelSurfs_GetBuffer(void* buffer);
}

namespace Components
{
  class ModelSurfs : public Component
  {
  public:
    ModelSurfs();
    ~ModelSurfs();

  private:
    friend void (::ModelSurfs_SetBuffer)(char streamHandle, void* buffer, IUnknown** bufferOut, int* offsetOut);
    friend IUnknown* (::ModelSurfs_GetBuffer)(void* buffer);

    static std::unordered_map<void*, IUnknown*> BufferMap;
    static std::unordered_map<Game::XModelSurfs*, Game::CModelAllocData*> AllocMap;

    static void ReleaseModelSurf(Game::XAssetHeader header);

    static void BeginRecover();
    static void EndRecover();

    static IUnknown* GetBuffer(void* buffer);
    static void SetBuffer(char streamHandle, void* buffer, IUnknown** bufferOut, int* offsetOut);

    static void CreateBuffers(Game::XModelSurfs* surfs);
    static Game::XModelSurfs* LoadXModelSurfaces(Game::XModelSurfs* asset);
    static bool LoadSurfaces(Game::XModel* model);
    static void XModelSurfsFixup(Game::XModel* model);
  };
}
