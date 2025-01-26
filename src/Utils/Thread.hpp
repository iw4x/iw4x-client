#pragma once

namespace Utils::Thread
{
	bool SetName(HANDLE t, const std::string& name);
	bool SetName(DWORD id, const std::string& name);
	bool SetName(std::jthread& t, const std::string& name);
	bool SetName(const std::string& name);

	template <typename Fn, typename... Args>
	std::jthread CreateNamedThread(const std::string& name, Fn&& fn, Args&&... args)
	{
		std::jthread t (
			[name, fn = std::forward<Fn> (fn)] (std::stop_token st,
			                                    Args&&... innerArgs) mutable
		{
			fn (st, std::forward<Args> (innerArgs)...);
		},
			std::forward<Args> (args)...);

		SetName(t, name);
		return t;
	}

	std::vector<DWORD> GetThreadIds();
}
