# pragma once

# include "Task.hpp"
# include <tuple>
# include <variant>
# include <type_traits>
# include <utility>
# include <cstddef>

/// @brief CoroAsync
namespace cra
{
	namespace detail
	{
		template <class... Types>
		struct WhenAllResult;
		template <class... Types>
		using WhenAllResult_t = typename WhenAllResult<Types...>::type;
		template <>
		struct WhenAllResult<> { using type = std::tuple<>; };
		template <class... Types>
		struct WhenAllResult<void, Types...> { using type = WhenAllResult_t<Types...>; };
		template <class T, class... Types>
		struct WhenAllResult<T, Types...>
		{
			using type = decltype(std::tuple_cat(std::declval<std::tuple<T>>(), std::declval<WhenAllResult_t<Types...>>()));
		};
	}

	/// @brief 指定したすべてのタスクが終了するのを待つタスクを作成します。
	/// @tparam ...Types タスクの結果の型
	/// @param ...tasks タスク
	/// @return 指定したタスクの結果を、 `void` であるものを除いて `std::tuple` に引数の順に詰めたものを結果とするタスク
	template <class... Types>
	Task<detail::WhenAllResult_t<Types...>> WhenAll(Task<Types>... tasks);

	/// @brief 指定したタスクのいずれかが終了するのを待つタスクを作成します。
	/// @tparam ...Types タスクの結果の型
	/// @param ...tasks タスク
	/// @return 指定したタスクの型を引数の順に設定した `std::variant` に、最初に完了したタスクを結果とするタスク
	template <class... Types>
	Task<std::variant<Task<Types>&...>> WhenAny(Task<Types>&... tasks);

	/// @brief 指定したタスクのいずれかが終了するのを待つタスクを作成します。
	/// @tparam ...Types タスクの結果の型
	/// @param ...tasks タスク
	/// @return 指定したタスクの型を引数の順に設定した `std::variant` に、最初に完了したタスクを結果とするタスク
	template <class... Types>
	Task<std::variant<Task<Types>...>> WhenAny(Task<Types>... tasks);
}

# include "detail/Utility.ipp"
