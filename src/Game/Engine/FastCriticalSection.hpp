#pragma once

namespace Game::Engine
{
	class FastCriticalSectionScopeRead
	{
	public:
		FastCriticalSectionScopeRead(FastCriticalSection* cs);
		~FastCriticalSectionScopeRead();

	private:
		FastCriticalSection* cs_;
	};

	class FastCriticalSectionScopeWrite
	{
	public:
		FastCriticalSectionScopeWrite(FastCriticalSection* cs);
		~FastCriticalSectionScopeWrite();

	private:
		FastCriticalSection* cs_;
	};
}
