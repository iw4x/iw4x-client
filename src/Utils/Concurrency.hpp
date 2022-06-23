#pragma once

#include <mutex>

namespace Utils::Concurrency
{
	template <typename T, typename MutexType = std::mutex>
	class Container
	{
	public:
		template <typename R = void, typename F>
		R access(F&& accessor) const
		{
			std::lock_guard<MutexType> _{mutex_};
			return accessor(object_);
		}

		template <typename R = void, typename F>
		R access(F&& accessor)
		{
			std::lock_guard<MutexType> _{mutex_};
			return accessor(object_);
		}

		template <typename R = void, typename F>
		R accessWithLock(F&& accessor) const
		{
			std::unique_lock<MutexType> lock{mutex_};
			return accessor(object_, lock);
		}

		template <typename R = void, typename F>
		R accessWithLock(F&& accessor)
		{
			std::unique_lock<MutexType> lock{mutex_};
			return accessor(object_, lock);
		}

		T& getRaw() {return object_;}
		const T& getRaw() const {return object_;}

	private:
		mutable MutexType mutex_{};
		T object_{};
	};
}
