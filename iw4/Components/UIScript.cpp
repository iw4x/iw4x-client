#include "..\STDInclude.hpp"

namespace Components
{
	std::map<std::string, UIScript::Callback> UIScript::UIScripts;

	template<> int UIScript::Token::Get()
	{
		if (this->IsValid())
		{
			return atoi(this->token);
		}

		return 0;
	}

	template<> char* UIScript::Token::Get()
	{
		if (this->IsValid())
		{
			return this->token;
		}

		return "";
	}

	template<> const char* UIScript::Token::Get()
	{
		return this->Get<char*>();
	}

	template<> std::string UIScript::Token::Get()
	{
		return this->Get<const char*>();
	}

	bool UIScript::Token::IsValid()
	{
		return (token && token[0]);
	}

	void UIScript::Token::Parse(const char** args)
	{
		if (args)
		{
			this->token = Game::Com_ParseExt(args);
		}
	}

	void UIScript::Add(std::string name, UIScript::Callback callback)
	{
		UIScript::UIScripts[name] = callback;
	}

	void UIScript::Add(std::string name, UIScript::CallbackRaw callback)
	{
		UIScript::Add(name, reinterpret_cast<UIScript::Callback>(callback));
	}

	bool UIScript::RunMenuScript(const char* name, const char** args)
	{
		if (UIScript::UIScripts.find(name) != UIScript::UIScripts.end())
		{
			UIScript::UIScripts[name](UIScript::Token(args));
			return true;
		}

		return false;
	}

	void __declspec(naked) UIScript::RunMenuScriptStub()
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
		// Install handler
		Utils::Hook::Set<int>(0x45EC5B, (DWORD)UIScript::RunMenuScriptStub - 0x45EC59 - 6);
	}

	UIScript::~UIScript()
	{
		UIScript::UIScripts.clear();
	}
}
