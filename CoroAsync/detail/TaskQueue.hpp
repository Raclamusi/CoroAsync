# pragma once

# include <coroutine>
# include <deque>
# include <set>
# include <map>
# include <chrono>
# include <thread>
# include <utility>
# include <initializer_list>
# include <algorithm>

namespace cra
{
	namespace detail
	{
		class TaskQueue
		{
		public:
			using Clock = std::chrono::steady_clock;
			using TimePoint = Clock::time_point;
			using Handle = std::coroutine_handle<>;

			// タスクキューに追加
			static void Push(Handle h)
			{
				tasks.emplace_back(h, false);
			}

			// 実行中のタスクのスリープ時間を設定
			template <class Rep, class Period>
			static void SleepFor(const std::chrono::duration<Rep, Period>& relTime)
			{
				sleepTime = Clock::now() + relTime;
			}

			static void Wait(Handle h)
			{
				taskToWait = h;
			}

			// タスクキューから削除
			// Task のデストラクタやムーブ代入演算子から呼ばれるので noexcept に実装
			static void Remove(Handle h) noexcept
			{
				// 完了している場合、削除の必要なし
				if (h.done())
				{
					return;
				}
				for (auto&& e : tasks)
				{
					if (e.first == h) e.second = true;
				}
				for (auto&& e : sleepingTasks)
				{
					if (e.second == h) e.second = nullptr;
				}
				for (auto&& e : taskWaitingTasks)
				{
					if (e.second == h) e.second = nullptr;
				}
			}

			// coro で示されるタスクが完了する、もしくは時刻が absTime になるまでタスクキューを回す
			template <class Clock, class Duration>
			static void RunUntil(Handle coro, const std::chrono::time_point<Clock, Duration>& absTime)
			{
				CheckForResumeFromSleep(absTime);
				while (tasks.size() && not coro.done())
				{
					auto [handle, removed] = tasks.front();
					tasks.pop_front();
					if (removed)
					{
						auto [first, last] = taskWaitingTasks.equal_range(handle);
						taskWaitingTasks.erase(first, last);
						continue;
					}
					handle.resume();
					if (handle.done())
					{
						auto [first, last] = taskWaitingTasks.equal_range(handle);
						for (auto it = first; it != last; ++it)
						{
							if (it->second)
							{
								tasks.emplace_back(it->second, false);
							}
						}
						taskWaitingTasks.erase(first, last);
					}
					else
					{
						if (sleepTime != TimePoint{})
						{
							sleepingTasks.emplace(sleepTime, handle);
						}
						else if (taskToWait)
						{
							taskWaitingTasks.emplace(taskToWait, handle);
						}
						else
						{
							tasks.emplace_back(handle, false);
						}
					}
					sleepTime = TimePoint{};
					taskToWait = nullptr;
					CheckForResumeFromSleep(absTime);
					if (Clock::now() >= absTime)
					{
						break;
					}
				}
			}

		private:
			inline static std::deque<std::pair<Handle, bool>> tasks;
			inline static std::multimap<TimePoint, Handle> sleepingTasks;
			inline static TimePoint sleepTime;
			inline static std::multimap<Handle, Handle> taskWaitingTasks;
			inline static Handle taskToWait;

			// スリープの終了のチェック
			static void CheckForResumeFromSleep(const TimePoint& limit)
			{
				auto now = Clock::now();
				while (sleepingTasks.size() && not sleepingTasks.begin()->second)
				{
					sleepingTasks.erase(sleepingTasks.begin());
				}
				if (tasks.empty() && sleepingTasks.size() && limit > sleepingTasks.begin()->first)
				{
					std::this_thread::sleep_until(sleepingTasks.begin()->first);
					now = sleepingTasks.begin()->first;
				}
				while (sleepingTasks.size() && now >= sleepingTasks.begin()->first)
				{
					if (sleepingTasks.begin()->second)
					{
						tasks.emplace_back(sleepingTasks.begin()->second, false);
					}
					sleepingTasks.erase(sleepingTasks.begin());
				}
			}
		};
	}
}
