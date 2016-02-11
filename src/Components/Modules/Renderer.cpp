#include "STDInclude.hpp"

namespace Components
{
	Utils::Hook Renderer::DrawFrameHook;
	wink::signal<wink::slot<Renderer::Callback>> Renderer::FrameSignal;

	void __declspec(naked) Renderer::FrameHook()
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
	}

	void Renderer::OnFrame(Renderer::Callback* callback)
	{
		Renderer::FrameSignal.connect(callback);
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
		// Frame hook
		Renderer::DrawFrameHook.Initialize(0x5ACB99, Renderer::FrameHook, HOOK_CALL)->Install();
	}

	Renderer::~Renderer()
	{
		Renderer::DrawFrameHook.Uninstall();

		// As I don't want to include my fork as submodule, we have to wait till my pull request gets accepted in order to do this.
		//Renderer::FrameSignal.clear();
	}
}
