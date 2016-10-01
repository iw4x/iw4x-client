namespace Utils
{
	template <typename T>
	class Chain
	{
	public:
		class Entry
		{
		private:
			std::shared_ptr<T> Object;
			std::shared_ptr<Entry> Next;

		public:
			bool HasNext()
			{
				return (this->Next.use_count() > 0);
			}

			bool IsValid()
			{
				return (this->Object.use_count() > 0);
			}
			
			void Set(T object)
			{
				this->Object = std::shared_ptr<T>(new T());
				*this->Object.get() = object;
			}

			std::shared_ptr<T> Get()
			{
				return this->Object;
			}

			Entry GetNext()
			{
				if (this->HasNext())
				{
					return *(this->Next.get());
				}
				else
				{
					return Entry();
				}
			}

			std::shared_ptr<Entry> GetNextEntry()
			{
				return this->Next;
			}

			void SetNextEntry(std::shared_ptr<Entry> entry)
			{
				this->Next = entry;
			}

			T *operator->()
			{
				return (this->Object.get());
			}

			Entry& operator++ ()
			{
				*this = this->GetNext();
				return *this;
			}

			Entry operator++ (int)
			{
				Entry result = *this;
				this->operator++();
				return result;
			}
		};

	private:
		std::mutex Mutex;
		Entry Object;

	public:
		void Add(T object)
		{
			this->Mutex.lock();
			
			if (!this->Empty())
			{
				// Create new chain entry
				std::shared_ptr<Entry> currentObject = std::shared_ptr<Entry>(new Entry);
				*currentObject.get() = this->Object;

				// Add it to the chain
				this->Object = Entry();
				this->Object.SetNextEntry(currentObject);
			}

			this->Object.Set(object);

			this->Mutex.unlock();
		}

		void Remove(std::shared_ptr<T> object)
		{
			this->Mutex.lock();
			
			if (!this->Empty())
			{
				if (this->Object.Get().get() == object.get())
				{
					this->Object = this->Object.GetNext();
				}
				else if(this->Object.HasNext())
				{
					for (auto entry = this->Object; entry.IsValid(); ++entry)
					{
						auto next = entry.GetNext();

						if (next.IsValid() && next.Get().get() == object.get())
						{
							*entry.GetNextEntry().get() = next.GetNext();
						}
					}
				}
			}

			this->Mutex.unlock();
		}

		void Remove(Entry entry)
		{
			if (entry.IsValid())
			{
				this->Remove(entry.Get());
			}
		}

		bool Empty()
		{
			return !this->Object.IsValid();
		}

		Entry Begin()
		{
			return this->Object;
		}

		void Clear()
		{
			this->Object = Entry();
		}
	};
}
