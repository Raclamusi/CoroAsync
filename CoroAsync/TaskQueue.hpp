# pragma once

# include <chrono>
# include <coroutine>
# include <deque>
# include <set>
# include <map>
# include <thread>
# include <utility>
# include <algorithm>
# include <cstdint>

/// @brief CoroAsync
namespace cra
{
	/// @brief タスクキュー関連
	namespace TaskQueue
	{
		/// @brief タスクの管理に使用されるクロック型
		using Clock = std::chrono::steady_clock;

		/// @brief タスクの管理に使用されるタイムポイント型
		using TimePoint = Clock::time_point;

		/// @brief タスクの管理に使用されるハンドル型
		using Handle = std::coroutine_handle<>;

		/// @brief 相対時間で期限を指定して、タスクキューのタスクを実行します。
		/// @param relTime 相対時間の期限
		template <class Rep, class Period>
		void RunFor(const std::chrono::duration<Rep, Period>& relTime);

		/// @brief 絶対時間で期限を指定して、タスクキューのタスクを実行します。
		/// @param absTime 絶対時間の期限
		template <class Clock, class Duration>
		void RunUntil(const std::chrono::time_point<Clock, Duration>& absTime);
	}
}

# include "detail/TaskQueue.ipp"
