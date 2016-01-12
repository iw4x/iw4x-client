namespace Utils
{
	class Memory
	{
	public:
		static void* Allocate(size_t length);
		template <typename T> static T* AllocateArray(size_t count)
		{
			return (T*)Allocate(count * sizeof(T));
		}

		static char* DuplicateString(std::string string);

		static void Free(void* data);
		static void Free(const void* data);
	};
}
