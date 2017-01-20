#pragma once

namespace Utils
{
	template <typename T>
	class Chain
	{
	public:
		class Entry
		{
		private:
			std::shared_ptr<T> object;
			std::shared_ptr<Entry> next;

		public:
			bool hasNext()
			{
				return (this->next.use_count() > 0);
			}

			bool isValid()
			{
				return (this->Object.use_count() > 0);
			}

			void set(T object)
			{
				this->object = std::shared_ptr<T>(new T());
				*this->object.get() = object;
			}

			std::shared_ptr<T> get()
			{
				return this->object;
			}

			Entry getNext()
			{
				if (this->hasNext())
				{
					return *(this->next.get());
				}
				else
				{
					return Entry();
				}
			}

			std::shared_ptr<Entry> getNextEntry()
			{
				return this->next;
			}

			void setNextEntry(std::shared_ptr<Entry> entry)
			{
				this->next = entry;
			}

			T *operator->()
			{
				return (this->object.get());
			}

			Entry& operator++ ()
			{
				*this = this->getNext();
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
		std::mutex mutex;
		Entry object;

	public:
		void add(T object)
		{
			std::lock_guard<std::mutex> _(this->mutex);

			if (!this->Empty())
			{
				// Create new chain entry
				std::shared_ptr<Entry> currentObject = std::shared_ptr<Entry>(new Entry);
				*currentObject.get() = this->object;

				// Add it to the chain
				this->object = Entry();
				this->object.setNextEntry(currentObject);
			}

			this->object.set(object);
		}

		void remove(std::shared_ptr<T> object)
		{
			std::lock_guard<std::mutex> _(this->mutex);

			if (!this->empty())
			{
				if (this->object.get().get() == object.get())
				{
					this->object = this->object.getNext();
				}
				else if(this->object.hasNext())
				{
					for (auto entry = this->object; entry.isValid(); ++entry)
					{
						auto next = entry.getNext();

						if (next.isValid() && next.get().get() == object.get())
						{
							*entry.getNextEntry().get() = next.getNext();
						}
					}
				}
			}
		}

		void remove(Entry entry)
		{
			if (entry.isValid())
			{
				this->remove(entry.Get());
			}
		}

		bool empty()
		{
			return !this->object.isValid();
		}

		Entry begin()
		{
			return this->object;
		}

		void clear()
		{
			this->object = Entry();
		}
	};
}
