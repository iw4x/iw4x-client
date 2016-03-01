#include "STDInclude.hpp"

namespace Components
{
	Utils::Hook Renderer::DrawFrameHook;
	wink::signal<wink::slot<Renderer::Callback>> Renderer::FrameSignal;
	wink::signal<wink::slot<Renderer::Callback>> Renderer::FrameOnceSignal;

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
		Renderer::FrameOnceSignal();
		Renderer::FrameOnceSignal.clear();
	}

	void Renderer::Once(Renderer::Callback* callback)
	{
		Renderer::FrameOnceSignal.connect(callback);
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
		Renderer::FrameOnceSignal.clear();
		Renderer::FrameSignal.clear();
	}
}
