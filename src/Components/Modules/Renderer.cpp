#include "STDInclude.hpp"

namespace Components
{
	Utils::Hook Renderer::DrawFrameHook;
	std::vector<Renderer::Callback> Renderer::FrameCallbacks;

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
		for (auto callback : Renderer::FrameCallbacks)
		{
			callback();
		}
	}

	void Renderer::OnFrame(Renderer::Callback callback)
	{
		Renderer::FrameCallbacks.push_back(callback);
	}

	Renderer::Renderer()
	{
		// Frame hook
		Renderer::DrawFrameHook.Initialize(0x5ACB99, Renderer::FrameHook, HOOK_CALL)->Install();
	}

	Renderer::~Renderer()
	{
		Renderer::DrawFrameHook.Uninstall();
		Renderer::FrameCallbacks.clear();
	}
}
