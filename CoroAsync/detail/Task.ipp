# pragma once

namespace cra
{
	namespace detail
	{
	# ifdef __GNUC__

		// GCC のバグ対策
		template <class Type>
		struct AwaiterWrapper
		{
			Type m_awaiter;
			decltype(auto) await_ready()
				noexcept(noexcept(m_awaiter.await_ready()))
			{
				return m_awaiter.await_ready();
			}
			decltype(auto) await_suspend(std::coroutine_handle<> h)
				noexcept(noexcept(m_awaiter.await_suspend(h)))
			{
				return m_awaiter.await_suspend(h);
			}
			decltype(auto) await_resume()
				noexcept(noexcept(m_awaiter.await_resume()))
			{
				return m_awaiter.await_resume();
			}
		};

	# endif
	}

	template <class Type>
	inline Task<Type>::Task() noexcept
		: m_coro{ nullptr }, m_result{} {}

	template <class Type>
	inline Task<Type>::Task(Handle h, std::future<Type> f) noexcept
		: m_coro{ h }, m_result{ std::move(f) }
	{
		detail::TaskQueueImpl::Push(m_coro);
	}

	template <class Type>
	inline Task<Type>::Task(Task&& other) noexcept
		: m_coro{ std::exchange(other.m_coro, nullptr) }, m_result{ std::move(other.m_result) } {}

	template <class Type>
	inline Task<Type>::~Task()
	{
		destroy();
	}

	template <class Type>
	inline Task<Type>& Task<Type>::operator =(Task&& other) noexcept
	{
		destroy();
		m_coro = std::exchange(other.m_coro, nullptr);
		m_result = std::move(other.m_result);
		return *this;
	}

	template <class Type>
	inline bool Task<Type>::isValid() const noexcept
	{
		return m_coro && m_result.valid();
	}

	template <class Type>
	inline bool Task<Type>::isReady() const noexcept
	{
		return isValid() && m_coro.done();
	}

	template <class Type>
	inline Type Task<Type>::get()
	{
		wait();
		return m_result.get();
	}

	template <class Type>
	inline void Task<Type>::wait()
	{
		wait_until(detail::TaskQueueImpl::TimePoint::max());
	}

	template <class Type>
	template <class Rep, class Period>
	inline std::future_status Task<Type>::wait_for(const std::chrono::duration<Rep, Period>& relTime)
	{
		return wait_until(detail::TaskQueueImpl::Clock::now() + relTime);
	}

	template <class Type>
	template <class Clock, class Duration>
	inline std::future_status Task<Type>::wait_until(const std::chrono::time_point<Clock, Duration>& absTime)
	{
		detail::TaskQueueImpl::RunUntil(m_coro, absTime);
		return m_coro.done() ? std::future_status::ready : std::future_status::timeout;
	}

	template <class Type>
	inline void Task<Type>::destroy() noexcept
	{
		if (m_coro)
		{
			detail::TaskQueueImpl::Remove(m_coro);
			m_coro.destroy();
			m_coro = nullptr;
		}
	}

	template <class Type>
	inline bool Task<Type>::await_ready() const noexcept
	{
		return false;
	}

	template <class Type>
	inline void Task<Type>::await_suspend(std::coroutine_handle<>) const noexcept
	{
	}

	template <class Type>
	inline Type Task<Type>::await_resume()
	{
		return get();
	}

	template <class Type>
	class Task<Type>::promise_type
	{
	public:
		Task get_return_object()
		{
			return Task{ Handle::from_promise(*this), m_result.get_future() };
		}
		std::suspend_always initial_suspend() noexcept
		{
			return std::suspend_always{};
		}
		std::suspend_always final_suspend() noexcept
		{
			return std::suspend_always{};
		}
		void return_value(Type value)
		{
			this->m_result.set_value(std::forward<Type>(value));
		}
		void unhandled_exception()
		{
			throw;
		}

		template <class Rep, class Period>
		std::suspend_always await_transform(const std::chrono::duration<Rep, Period>& relTime)
		{
			if (relTime.count() > 0) {
				detail::TaskQueueImpl::SleepFor(Handle::from_promise(*this), relTime);
			}
			return std::suspend_always{};
		}
		std::suspend_always await_transform(std::uint32_t relTime)
		{
			return await_transform(std::chrono::milliseconds{ relTime });
		}
		template <class U>
		decltype(auto) await_transform(Task<U>& task)
		{
			detail::TaskQueueImpl::Wait(Handle::from_promise(*this), task.m_coro);
		# ifdef __GNUC__

			// GCC のバグ対策
			return detail::AwaiterWrapper<Task<U>&>{ task };
		# else

			return task;

		# endif
		}
		template <class U>
		decltype(auto) await_transform(Task<U>&& task)
		{
			detail::TaskQueueImpl::Wait(Handle::from_promise(*this), task.m_coro);
		# ifdef __GNUC__

			// GCC のバグ対策
			return detail::AwaiterWrapper<Task<U>&&>{ std::move(task) };
		# else

			return std::move(task);

		# endif
		}

	private:
		std::promise<Type> m_result;
	};

	template <>
	class Task<void>::promise_type
	{
	public:
		Task<void> get_return_object()
		{
			return Task{ Handle::from_promise(*this), m_result.get_future() };
		}
		std::suspend_always initial_suspend() noexcept
		{
			return std::suspend_always{};
		}
		std::suspend_always final_suspend() noexcept
		{
			return std::suspend_always{};
		}
		void return_void()
		{
			m_result.set_value();
		}
		void unhandled_exception()
		{
			throw;
		}

		template <class Rep, class Period>
		std::suspend_always await_transform(const std::chrono::duration<Rep, Period>& relTime)
		{
			if (relTime.count() > 0) {
				detail::TaskQueueImpl::SleepFor(Handle::from_promise(*this), relTime);
			}
			return std::suspend_always{};
		}
		std::suspend_always await_transform(std::uint32_t relTime)
		{
			return await_transform(std::chrono::milliseconds{ relTime });
		}
		template <class U>
		decltype(auto) await_transform(Task<U>& task)
		{
			detail::TaskQueueImpl::Wait(Handle::from_promise(*this), task.m_coro);
		# ifdef __GNUC__

			// GCC のバグ対策
			return detail::AwaiterWrapper<Task<U>&>{ task };
		# else

			return task;

		# endif
		}
		template <class U>
		decltype(auto) await_transform(Task<U>&& task)
		{
			detail::TaskQueueImpl::Wait(Handle::from_promise(*this), task.m_coro);
		# ifdef __GNUC__

			// GCC のバグ対策
			return detail::AwaiterWrapper<Task<U>&&>{ std::move(task) };
		# else

			return std::move(task);

		# endif
		}

	private:
		std::promise<void> m_result;
	};
}
