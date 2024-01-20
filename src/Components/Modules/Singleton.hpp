#pragma once

namespace Components
{
	class Singleton : public Component
	{
	public:
		Singleton();

		void preDestroy() override;

		static bool IsFirstInstance();

	private:
		static HANDLE Mutex;
		static bool FirstInstance;
	};
}
