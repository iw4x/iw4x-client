#pragma once

namespace Game::Engine
{
	class LargeLocal
	{
	public:
		explicit LargeLocal(int sizeParam);
		~LargeLocal();

		LargeLocal(LargeLocal&&) = delete;
		LargeLocal(const LargeLocal&) = delete;
		LargeLocal& operator=(LargeLocal&&) = delete;
		LargeLocal& operator=(const LargeLocal&) = delete;

		[[nodiscard]] void* GetBuf() const;

	private:
		void PopBuf();

		int startPos;
		int size;
	};

	extern void LargeLocalEnd(int startPos);
	extern void LargeLocalEndRight(int startPos);

	extern void* LargeLocalGetBuf(int startPos, int size);

	extern int CanUseServerLargeLocal();
}
