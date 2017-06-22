#include "STDInclude.hpp"

namespace Components
{
	Utils::Signal<Renderer::BackendCallback> Renderer::BackendFrameSignal;
	Utils::Signal<Renderer::BackendCallback> Renderer::SingleBackendFrameSignal;

	Utils::Signal<Scheduler::Callback> Renderer::EndRecoverDeviceSignal;
	Utils::Signal<Scheduler::Callback> Renderer::BeginRecoverDeviceSignal;

	__declspec(naked) void Renderer::FrameStub()
	{
		__asm
		{
			pushad
			call Scheduler::FrameHandler
			popad

			push 5AC950h
			retn
		}
	}

	__declspec(naked) void Renderer::BackendFrameStub()
	{
		__asm
		{
			pushad
			call Renderer::BackendFrameHandler
			popad

			mov eax, ds:66E1BF0h
			push 536A85h
			retn
		}
	}

	void Renderer::BackendFrameHandler()
	{
		IDirect3DDevice9* device = *Game::dx_ptr;

		if (device)
		{
			device->AddRef();

			Renderer::BackendFrameSignal(device);

			Utils::Signal<Renderer::BackendCallback> copy(Renderer::SingleBackendFrameSignal);
			Renderer::SingleBackendFrameSignal.clear();
			copy(device);

			device->Release();
		}
	}

	void Renderer::OnNextBackendFrame(Utils::Slot<Renderer::BackendCallback> callback)
	{
		Renderer::SingleBackendFrameSignal.connect(callback);
	}

	void Renderer::OnBackendFrame(Utils::Slot<Renderer::BackendCallback> callback)
	{
		Renderer::BackendFrameSignal.connect(callback);
	}

	void Renderer::OnDeviceRecoveryEnd(Utils::Slot<Scheduler::Callback> callback)
	{
		Renderer::EndRecoverDeviceSignal.connect(callback);
	}

	void Renderer::OnDeviceRecoveryBegin(Utils::Slot<Scheduler::Callback> callback)
	{
		Renderer::BeginRecoverDeviceSignal.connect(callback);
	}

	int Renderer::Width()
	{
		return reinterpret_cast<LPPOINT>(0x66E1C68)->x;
	}

	int Renderer::Height()
	{
		return reinterpret_cast<LPPOINT>(0x66E1C68)->y;
	}

	void Renderer::PreVidRestart()
	{
		Renderer::BeginRecoverDeviceSignal();
	}

	void Renderer::PostVidRestart()
	{
		Renderer::EndRecoverDeviceSignal();
	}

	__declspec(naked) void Renderer::PostVidRestartStub()
	{
		__asm
		{
			pushad
			call Renderer::PostVidRestart
			popad

			push 4F84C0h
			retn
		}
	}

	Renderer::Renderer()
	{
		if (Dedicated::IsEnabled()) return;

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
		Utils::Hook(0x5ACB99, Renderer::FrameStub, HOOK_CALL).install()->quick();

		Utils::Hook(0x536A80, Renderer::BackendFrameStub, HOOK_JUMP).install()->quick();

		// Begin device recovery (not D3D9Ex)
		Utils::Hook(0x508298, []()
		{
			Game::DB_BeginRecoverLostDevice();
			Renderer::BeginRecoverDeviceSignal();
		}, HOOK_CALL).install()->quick();

		// End device recovery (not D3D9Ex)
		Utils::Hook(0x508355, []()
		{
			Renderer::EndRecoverDeviceSignal();
			Game::DB_EndRecoverLostDevice();
		}, HOOK_CALL).install()->quick();

		// Begin vid_restart
		Utils::Hook(0x4CA2FD, Renderer::PreVidRestart, HOOK_CALL).install()->quick();

		// End vid_restart
		Utils::Hook(0x4CA3A7, Renderer::PostVidRestartStub, HOOK_CALL).install()->quick();
	}

	Renderer::~Renderer()
	{
		Renderer::BackendFrameSignal.clear();
		Renderer::SingleBackendFrameSignal.clear();

		Renderer::EndRecoverDeviceSignal.clear();
		Renderer::BeginRecoverDeviceSignal.clear();
	}
}
