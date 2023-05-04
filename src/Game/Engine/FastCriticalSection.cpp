#include <STDInclude.hpp>
#include "FastCriticalSection.hpp"

namespace Game::Engine
{
	FastCriticalSectionScopeRead::FastCriticalSectionScopeRead(FastCriticalSection* cs)
		: cs_(cs)
	{
		if (this->cs_)
		{
			Sys_LockRead(this->cs_);
		}
	}

	FastCriticalSectionScopeRead::~FastCriticalSectionScopeRead()
	{
		if (this->cs_)
		{
			Sys_UnlockRead(this->cs_);
		}
	}

	FastCriticalSectionScopeWrite::FastCriticalSectionScopeWrite(FastCriticalSection* cs)
		: cs_(cs)
	{
		if (this->cs_)
		{
			Sys_LockWrite(this->cs_);
		}
	}

	FastCriticalSectionScopeWrite::~FastCriticalSectionScopeWrite()
	{
		if (this->cs_)
		{
			Sys_UnlockWrite(this->cs_);
		}
	}
}
