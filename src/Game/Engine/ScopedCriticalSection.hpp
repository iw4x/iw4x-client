#pragma once

namespace Game::Engine
{
	enum ScopedCriticalSectionType
	{
		SCOPED_CRITSECT_NORMAL = 0x0,
		SCOPED_CRITSECT_DISABLED = 0x1,
		SCOPED_CRITSECT_RELEASE = 0x2,
		SCOPED_CRITSECT_TRY = 0x3,
	};

	class ScopedCriticalSection
	{
	public:
		ScopedCriticalSection(CriticalSection s, ScopedCriticalSectionType type);
		~ScopedCriticalSection();

		void enterCritSect();
		void leaveCritSect();
		[[nodiscard]] bool tryEnterCritSect();

		[[nodiscard]] bool hasOwnership() const;
		[[nodiscard]] bool isScopedRelease() const;

	private:
		CriticalSection s_;
		bool hasOwnership_;
		bool isScopedRelease_;
	};
}
