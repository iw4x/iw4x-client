#pragma once

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
				this->pool.clear();
				this->refMemory.clear();
			}
			~Allocator()
			{
				this->clear();
			}

			void clear()
			{
				std::lock_guard _(this->mutex);

				for (auto i = this->refMemory.begin(); i != this->refMemory.end(); ++i)
				{
					if (i->first && i->second)
					{
						i->second(i->first);
					}
				}

				this->refMemory.clear();

				for (const auto& data : this->pool)
				{
					Free(data);
				}

				this->pool.clear();
			}

			void free(void* data)
			{
				std::lock_guard _(this->mutex);

				auto i = this->refMemory.find(data);
				if (i != this->refMemory.end())
				{
					i->second(i->first);
					this->refMemory.erase(i);
				}

				auto j = std::find(this->pool.begin(), this->pool.end(), data);
				if (j != this->pool.end())
				{
					Free(data);
					this->pool.erase(j);
				}
			}

			void free(const void* data)
			{
				this->free(const_cast<void*>(data));
			}

			void reference(void* memory, FreeCallback callback)
			{
				std::lock_guard _(this->mutex);

				this->refMemory[memory] = callback;
			}

			void* allocate(std::size_t length)
			{
				std::lock_guard _(this->mutex);

				void* data = Allocate(length);
				this->pool.push_back(data);
				return data;
			}

			template <typename T> T* allocate()
			{
				return this->allocateArray<T>(1);
			}

			template <typename T> T* allocateArray(std::size_t count = 1)
			{
				return static_cast<T*>(this->allocate(count * sizeof(T)));
			}

			bool empty() const
			{
				return (this->pool.empty() && this->refMemory.empty());
			}

			char* duplicateString(const std::string& string)
			{
				std::lock_guard _(this->mutex);

				char* data = DuplicateString(string);
				this->pool.push_back(data);
				return data;
			}

			bool isPointerMapped(void* ptr) const
			{
				return this->ptrMap.contains(ptr);
			}

			template <typename T> T* getPointer(void* oldPtr)
			{
				if (this->isPointerMapped(oldPtr))
				{
					return static_cast<T*>(this->ptrMap[oldPtr]);
				}

				return nullptr;
			}

			void mapPointer(void* oldPtr, void* newPtr)
			{
				this->ptrMap[oldPtr] = newPtr;
			}

		private:
			std::mutex mutex;
			std::vector<void*> pool;
			std::unordered_map<void*, void*> ptrMap;
			std::unordered_map<void*, FreeCallback> refMemory;
		};

		static void* AllocateAlign(std::size_t length, std::size_t alignment);
		static void* Allocate(std::size_t length);
		template <typename T> static T* Allocate()
		{
			return AllocateArray<T>(1);
		}
		template <typename T> static T* AllocateArray(std::size_t count = 1)
		{
			return static_cast<T*>(Allocate(count * sizeof(T)));
		}

		template <typename T> static T* Duplicate(T* original)
		{
			T* data = Memory::Allocate<T>();
			std::memcpy(data, original, sizeof(T));
			return data;
		}

		static char* DuplicateString(const std::string& string);

		static void Free(void* data);
		static void Free(const void* data);

		static void FreeAlign(void* data);
		static void FreeAlign(const void* data);

		static bool IsSet(void* mem, char chr, std::size_t length);

		static bool IsBadReadPtr(const void* ptr);
		static bool IsBadCodePtr(const void* ptr);

		static Allocator* GetAllocator();

	private:
		static Allocator MemAllocator;
	};
}
