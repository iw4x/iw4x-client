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
		Renderer::FrameCallbacks.clear();
	}
}
