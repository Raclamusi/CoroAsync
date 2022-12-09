# pragma once

# include <utility>
# include <cstddef>

namespace cra
{
	namespace detail
	{
		auto GetAll()
		{
			return std::tuple<>{};
		}
		template <class T, class... Types>
		auto GetAll(Task<T>& task, Task<Types>&... tasks);
		template <class... Types>
		auto GetAll(Task<>&, Task<Types>&... tasks)
		{
			return GetAll(tasks...);
		}
		template <class T, class... Types>
		auto GetAll(Task<T>& task, Task<Types>&... tasks)
		{
			return std::tuple_cat(std::tuple{ task.get() }, GetAll(tasks...));
		}

		template <class Ret, std::size_t I = 0, class Task>
		Ret GetAny(Task&& task)
		{
			return Ret{ std::in_place_index<I>, std::forward<Task>(task) };
		}
		template <class Ret, std::size_t I = 0, class Task, class... Tasks>
		Ret GetAny(Task&& task, Tasks&&... tasks)
		{
			if (task.isReady())
			{
				return Ret{ std::in_place_index<I>, std::forward<Task>(task) };
			}
			return GetAny<Ret, I + 1>(std::forward<Tasks>(tasks)...);
		}
	}

	template <class... Types>
	Task<detail::WhenAllResult_t<Types...>> WhenAll(Task<Types>... tasks)
	{
		while ((not tasks.isReady() || ...))
		{
			co_await 0;
		}
		co_return detail::GetAll(tasks...);
	}

	template <class... Types>
	Task<std::variant<Task<Types>&...>> WhenAny(Task<Types>&... tasks)
	{
		while ((not tasks.isReady() && ...))
		{
			co_await 0;
		}
		co_return detail::GetAny<std::variant<Task<Types>...>>(tasks...);
	}

	template <class... Types>
	Task<std::variant<Task<Types>...>> WhenAny(Task<Types>... tasks)
	{
		while ((not tasks.isReady() && ...))
		{
			co_await 0;
		}
		co_return detail::GetAny<std::variant<Task<Types>...>>(std::move(tasks)...);
	}
}
