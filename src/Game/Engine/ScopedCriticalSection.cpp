#include <STDInclude.hpp>
#include "ScopedCriticalSection.hpp"

namespace Game::Engine
{
	ScopedCriticalSection::ScopedCriticalSection(const CriticalSection s, const ScopedCriticalSectionType type)
		: s_(s), isScopedRelease_(false)
	{
		if (type == SCOPED_CRITSECT_NORMAL)
		{
			Sys_EnterCriticalSection(this->s_);
			this->hasOwnership_ = true;
			return;
		}

		if (type == SCOPED_CRITSECT_TRY)
		{
			this->hasOwnership_ = Sys_TryEnterCriticalSection(this->s_);
		}
		else
		{
			if (type == SCOPED_CRITSECT_RELEASE)
			{
				Sys_LeaveCriticalSection(this->s_);
				this->isScopedRelease_ = true;
			}

			this->hasOwnership_ = false;
		}
	}

	ScopedCriticalSection::~ScopedCriticalSection()
	{
		if (!this->hasOwnership_ || this->isScopedRelease_)
		{
			if (!this->hasOwnership_ && this->isScopedRelease_)
			{
				Sys_EnterCriticalSection(this->s_);
			}
		}
		else
		{
			Sys_LeaveCriticalSection(this->s_);
		}
	}

	void ScopedCriticalSection::enterCritSect()
	{
		assert(!this->hasOwnership_);

		this->hasOwnership_ = true;
		Sys_EnterCriticalSection(this->s_);
	}

	void ScopedCriticalSection::leaveCritSect()
	{
		assert(this->hasOwnership_);

		this->hasOwnership_ = false;
		Sys_LeaveCriticalSection(this->s_);
	}

	bool ScopedCriticalSection::tryEnterCritSect()
	{
		assert(!this->hasOwnership_);

		const auto result = Sys_TryEnterCriticalSection(this->s_);
		this->hasOwnership_ = result;
		return result;
	}

	bool ScopedCriticalSection::hasOwnership() const
	{
		return this->hasOwnership_;
	}

	bool ScopedCriticalSection::isScopedRelease() const
	{
		return this->isScopedRelease_;
	}
}
