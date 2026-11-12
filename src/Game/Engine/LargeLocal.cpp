#include "LargeLocal.hpp"

namespace Game::Engine
{
  LargeLocal::LargeLocal(int sizeParam)
  {
    assert(sizeParam);
    assert(Sys_IsMainThread() || CanUseServerLargeLocal());

    // Let the game round the size and allocate the buffer.
    //
    // Note that x64 inlines LargeLocalBegin() and LargeLocalBeginRight()
    // into its LargeLocal constructor, which stores the start position and
    // the rounded size into the same two fields as ours.
    //
    LargeLocalConstruct(this, sizeParam);
  }

  LargeLocal::~LargeLocal()
  {
    if (this->size)
    {
      this->PopBuf();
    }
  }

  void LargeLocal::PopBuf()
  {
    assert(this->size);
    assert(Sys_IsMainThread() || CanUseServerLargeLocal());

    if (Sys_IsMainThread())
    {
      LargeLocalEnd(this->startPos);
    }
    else
    {
      LargeLocalEndRight(this->startPos);
    }

    this->size = 0;
  }

  void* LargeLocal::GetBuf() const
  {
    assert(this->size);
    assert(Sys_IsMainThread() || CanUseServerLargeLocal());

    return LargeLocalGetBuf(this->startPos, this->size);
  }

  void LargeLocalEnd(int startPos)
  {
    assert(Sys_IsMainThread());
    assert(*g_largeLocalBuf);

    *g_largeLocalPos = startPos;
  }

  void LargeLocalEndRight(int startPos)
  {
    assert(CanUseServerLargeLocal());
    assert(*g_largeLocalBuf);

    *g_largeLocalRightPos = startPos;
  }

  void* LargeLocalGetBuf(int startPos, int size)
  {
    assert(Sys_IsMainThread() || CanUseServerLargeLocal());
    assert(*g_largeLocalBuf);
    assert(!(size & 127));

    if (Sys_IsMainThread())
    {
      return &(*g_largeLocalBuf)[startPos];
    }

    const auto startIndex = startPos - size;
    assert(startIndex >= 0);

    return &(*g_largeLocalBuf)[startIndex];
  }

  int CanUseServerLargeLocal()
  {
    return SV_GetServerThreadOwnsGame() ? Sys_IsServerThread() : Sys_IsRenderThread();
  }
}
