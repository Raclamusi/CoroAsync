# pragma once

# include "Task.hpp"
# include <variant>

/// @brief CoroAsync
namespace cra
{
	/// @brief 指定したすべてのタスクが終了するのを待つタスクを作成します。
	/// @tparam ...Types タスクの結果の型
	/// @param ...tasks タスク
	/// @return 指定したタスクの結果を、 `void` であるものを除いて `std::tuple` に引数の順に詰めたものを結果とするタスク
	template <class... Types>
	[[nodiscard]]
	auto WhenAll(Task<Types>... tasks);

	/// @brief 指定したタスクのいずれかが終了するのを待つタスクを作成します。
	/// @tparam ...Types タスクの結果の型
	/// @param ...tasks タスク
	/// @return 指定したタスクの型を引数の順に設定した `std::variant` に、最初に完了したタスクを結果とするタスク
	template <class... Types>
	[[nodiscard]]
	Task<std::variant<Task<Types>&...>> WhenAny(Task<Types>&... tasks);

	/// @brief 指定したタスクのいずれかが終了するのを待つタスクを作成します。
	/// @tparam ...Types タスクの結果の型
	/// @param ...tasks タスク
	/// @return 指定したタスクの型を引数の順に設定した `std::variant` に、最初に完了したタスクを結果とするタスク
	template <class... Types>
	[[nodiscard]]
	Task<std::variant<Task<Types>...>> WhenAny(Task<Types>... tasks);
}

# include "detail/Utility.ipp"
