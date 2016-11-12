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
				this->Clear();
			}

			void Clear()
			{
				for (auto i = this->RefMemory.begin(); i != this->RefMemory.end(); ++i)
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

			void Free(void* data)
			{
				auto i = this->RefMemory.find(data);
				if (i != this->RefMemory.end())
				{
					i->second(i->first);
					this->RefMemory.erase(i);
				}

				auto j = std::find(this->Pool.begin(), this->Pool.end(), data);
				if (j != this->Pool.end())
				{
					Memory::Free(data);
					this->Pool.erase(j);
				}
			}

			void Free(const void* data)
			{
				this->Free(const_cast<void*>(data));
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
			template <typename T> T* Allocate()
			{
				return this->AllocateArray<T>(1);
			}
			template <typename T> T* AllocateArray(size_t count = 1)
			{
				return static_cast<T*>(this->Allocate(count * sizeof(T)));
			}

			bool Empty()
			{
				return (this->Pool.empty() && this->RefMemory.empty());
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

		static void* AllocateAlign(size_t length, size_t alignment);
		static void* Allocate(size_t length);
		template <typename T> static T* Allocate()
		{
			return AllocateArray<T>(1);
		}
		template <typename T> static T* AllocateArray(size_t count = 1)
		{
			return static_cast<T*>(Allocate(count * sizeof(T)));
		}

		static char* DuplicateString(std::string string);

		static void Free(void* data);
		static void Free(const void* data);

		static void FreeAlign(void* data);
		static void FreeAlign(const void* data);

		static bool IsSet(void* mem, char chr, size_t length);
	};
}
