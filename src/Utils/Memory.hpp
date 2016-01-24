namespace Utils
{
	class Memory
	{
	public:
		class Allocator
		{
		public:
			typedef void(*FreeCallback)(void*);

			Allocator()
			{ 
				this->Pool.clear();
				this->RefMemory.clear();
			}
			~Allocator() 
			{
				for (auto i = this->RefMemory.begin(); i != this->RefMemory.end(); i++)
				{
					if (i->first && i->second)
					{
						i->second(i->first);
					}
				}

				this->RefMemory.clear();

				for (auto data : this->Pool)
				{
					Memory::Free(data);
				}

				this->Pool.clear();
			}

			void Reference(void* memory, FreeCallback callback)
			{
				this->RefMemory[memory] = callback;
			}

			void* Allocate(size_t length)
			{
				void* data = Memory::Allocate(length);
				this->Pool.push_back(data);
				return data;
			}
			template <typename T> T* AllocateArray(size_t count = 1)
			{
				return static_cast<T*>(this->Allocate(count * sizeof(T)));
			}

			char* DuplicateString(std::string string)
			{
				char* data = Memory::DuplicateString(string);
				this->Pool.push_back(data);
				return data;
			}

		private:
			std::vector<void*> Pool;
			std::map<void*, FreeCallback> RefMemory;
		};

		static void* Allocate(size_t length);
		template <typename T> static T* AllocateArray(size_t count = 1)
		{
			return static_cast<T*>(Allocate(count * sizeof(T)));
		}

		static char* DuplicateString(std::string string);

		static void Free(void* data);
		static void Free(const void* data);
	};
}
