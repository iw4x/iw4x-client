#pragma once

namespace Utils::Thread
{
	bool setName(HANDLE t, const std::string& name);
	bool setName(DWORD id, const std::string& name);
	bool setName(std::thread& t, const std::string& name);
	bool setName(const std::string& name);

	template <typename ...Args>
	std::thread createNamedThread(const std::string& name, Args&&... args)
	{
		auto t = std::thread(std::forward<Args>(args)...);
		setName(t, name);
		return t;
	}

	std::vector<DWORD> getThreadIds();
	void forEachThread(const std::function<void(HANDLE)>& callback);

	void suspendOtherThreads();
	void resumeOtherThreads();
}
