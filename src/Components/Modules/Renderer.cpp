#include "STDInclude.hpp"

namespace Components
{
	Utils::Hook Renderer::DrawFrameHook;
	wink::signal<wink::slot<Renderer::Callback>> Renderer::FrameSignal;
	wink::signal<wink::slot<Renderer::Callback>> Renderer::FrameOnceSignal;
	wink::signal<wink::slot<Renderer::BackendCallback>> Renderer::BackendFrameSignal;

	void __declspec(naked) Renderer::FrameStub()
	{
		__asm
		{
			call Renderer::FrameHandler
			jmp Renderer::DrawFrameHook.Original
		}
	}

	void Renderer::FrameHandler()
	{
		Renderer::FrameSignal();
		Renderer::FrameOnceSignal();
		Renderer::FrameOnceSignal.clear();
	}

	void __declspec(naked) Renderer::BackendFrameStub()
	{
		__asm
		{
			call Renderer::BackendFrameHandler

			mov eax, ds:66E1BF0h
			mov ecx, 536A85h
			jmp ecx
		}
	}

	void Renderer::BackendFrameHandler()
	{
		IDirect3DDevice9* device = *Game::dx_ptr;

		if (device)
		{
			device->AddRef();
			Renderer::BackendFrameSignal(device);
			device->Release();
		}
	}

	void Renderer::Once(Renderer::Callback* callback)
	{
		Renderer::FrameOnceSignal.connect(callback);
	}

	void Renderer::OnFrame(Renderer::Callback* callback)
	{
		Renderer::FrameSignal.connect(callback);
	}

	void Renderer::OnBackendFrame(Renderer::BackendCallback* callback)
	{
		Renderer::BackendFrameSignal.connect(callback);
	}

	int Renderer::Width()
	{
		return Utils::Hook::Get<int>(0x66E1C68);
	}

	int Renderer::Height()
	{
		return Utils::Hook::Get<int>(0x66E1C6C);
	}

	Renderer::Renderer()
	{
// 		Renderer::OnBackendFrame([] (IDirect3DDevice9* device)
// 		{
// 			if (Game::Sys_Milliseconds() % 2)
// 			{
// 				device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0, 0, 0);
// 			}
// 
// 			return;
// 
// 			IDirect3DSurface9* buffer = nullptr;
// 
// 			device->CreateOffscreenPlainSurface(Renderer::Width(), Renderer::Height(), D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &buffer, nullptr);
// 			device->GetFrontBufferData(0, buffer);
// 
// 			if (buffer)
// 			{
// 				D3DSURFACE_DESC desc;
// 				D3DLOCKED_RECT lockedRect;
// 
// 				buffer->GetDesc(&desc);
// 
// 				HRESULT res = buffer->LockRect(&lockedRect, NULL, D3DLOCK_READONLY);
// 
// 
// 				buffer->UnlockRect();
// 			}
// 		});

		// Frame hook
		Renderer::DrawFrameHook.Initialize(0x5ACB99, Renderer::FrameStub, HOOK_CALL)->Install();

		Utils::Hook(0x536A80, Renderer::BackendFrameStub, HOOK_JUMP).Install()->Quick();
	}

	Renderer::~Renderer()
	{
		Renderer::DrawFrameHook.Uninstall();
		Renderer::BackendFrameSignal.clear();
		Renderer::FrameOnceSignal.clear();
		Renderer::FrameSignal.clear();
	}
}
