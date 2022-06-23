#include <STDInclude.hpp>

namespace Components
{
	std::unordered_map<std::string, Utils::Slot<UIScript::Callback>> UIScript::UIScripts;
	std::unordered_map<int, Utils::Slot<UIScript::CallbackRaw>> UIScript::UIOwnerDraws;

	template<> int UIScript::Token::get()
	{
		if (this->isValid())
		{
			return atoi(this->token);
		}

		return 0;
	}

	template<> const char* UIScript::Token::get()
	{
		if (this->isValid())
		{
			return this->token;
		}

		return "";
	}

	template<> std::string UIScript::Token::get()
	{
		return this->get<const char*>();
	}

	bool UIScript::Token::isValid()
	{
		return (this->token && this->token[0]);
	}

	void UIScript::Token::parse(const char** args)
	{
		if (args)
		{
			this->token = Game::Com_Parse(args);
		}
	}

	void UIScript::Add(const std::string& name, Utils::Slot<UIScript::Callback> callback)
	{
		UIScript::UIScripts[name] = callback;
	}

	void UIScript::AddOwnerDraw(int ownerdraw, Utils::Slot<UIScript::CallbackRaw> callback)
	{
		UIScript::UIOwnerDraws[ownerdraw] = callback;
	}

	bool UIScript::RunMenuScript(const char* name, const char** args)
	{
		if (UIScript::UIScripts.contains(name))
		{
			UIScript::UIScripts[name](UIScript::Token(args));
			return true;
		}

		return false;
	}

	void UIScript::OwnerDrawHandleKeyStub(int ownerDraw, int flags, float *special, int key)
	{
		if (key == 200 || key == 201) //mouse buttons
		{
			for (auto i = UIScript::UIOwnerDraws.begin(); i != UIScript::UIOwnerDraws.end(); ++i)
			{
				if (i->first == ownerDraw)
				{
					i->second();
				}
			}
		}

		Utils::Hook::Call<void(int, int, float*, int)>(0x4F58A0)(ownerDraw, flags, special, key);
	}

	__declspec(naked) void UIScript::RunMenuScriptStub()
	{
		__asm
		{
			mov eax, esp
			add eax, 8h
			mov edx, eax           // UIScript name
			mov eax, [esp + 0C10h] // UIScript args

			push eax
			push edx
			call UIScript::RunMenuScript
			add esp, 8h

			test al, al
			jz continue

			// if returned
			pop edi
			pop esi
			add esp, 0C00h
			retn

		continue:
			mov eax, 45ED00h
			jmp eax
		}
	}

	UIScript::UIScript()
	{
		if (Dedicated::IsEnabled()) return;

		// Install handler
		Utils::Hook::RedirectJump(0x45EC59, UIScript::RunMenuScriptStub);

		// Install ownerdraw handler
		Utils::Hook(0x63D233, UIScript::OwnerDrawHandleKeyStub, HOOK_CALL).install()->quick();
	}

	UIScript::~UIScript()
	{
		UIScript::UIScripts.clear();
		UIScript::UIOwnerDraws.clear();
	}
}
