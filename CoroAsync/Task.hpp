# pragma once

# include <coroutine>
# include <future>
# include <chrono>
# include <cstdint>

/// @brief CoroAsync
namespace cra
{
	/// @brief シングルスレッドで非同期処理をするためのクラス
	/// @remark コルーチンの戻り値に指定して使用します。
	/// @tparam Type 非同期処理するコルーチンの戻り値の型
	template <class Type = void>
	class Task
	{
	public:
		/// @brief 非同期処理の結果の型
		using ResultType = Type;

		/// @brief デフォルトコンストラクタ。コルーチンを結びつけずに初期化します。
		[[nodiscard]]
		Task() noexcept;

		/// @brief コピーコンストラクタ。コピー不可です。
		Task(const Task&) = delete;

		/// @brief ムーブコンストラクタ。
		[[nodiscard]]
		Task(Task&& other) noexcept;

		/// @brief デストラクタ。コルーチンを破棄します。
		~Task();

		/// @brief コピー代入演算子。コピー不可です。
		Task& operator =(const Task&) = delete;

		/// @brief ムーブ代入演算子。
		Task& operator =(Task&& other) noexcept;

		/// @brief 有効なタスクを持っているか確認します。
		/// @return 有効なタスクを持っていれば true 、そうでなければ false
		[[nodiscard]]
		bool isValid() const noexcept;

		/// @brief タスクが完了しているか確認します。
		/// @return タスクが完了していれば true 、そうでなければ false
		[[nodiscard]]
		bool isReady() const noexcept;

		/// @brief タスクの完了を待機し、非同期処理の結果を返します。
		/// @remark `get()` と `await_resume()` は1オブジェクトにつき合わせて1回しか呼び出せません。
		/// @return 非同期処理の結果
		[[nodiscard]]
		Type get();

		/// @brief タスクの完了を待機します。
		/// @remark 非同期処理はいずれかのタスクを待機している間にしか行われません。
		void wait();

		/// @brief 相対時間で期限を指定して、タスクの完了を待機します。
		/// @param relTime 相対時間の期限
		/// @return タスクが完了していれば `std::future_status::ready` 、そうでなければ `std::future_status::timeout`
		template <class Rep, class Period>
		std::future_status wait_for(const std::chrono::duration<Rep, Period>& relTime);

		/// @brief 絶対時間で期限を指定して、タスクの完了を待機します。
		/// @param absTime 絶対時間の期限
		/// @return タスクが完了していれば `std::future_status::ready` 、そうでなければ `std::future_status::timeout`
		template <class Clock, class Duration>
		std::future_status wait_until(const std::chrono::time_point<Clock, Duration>& absTime);

		/// @brief コルーチンを破棄します。
		void destroy() noexcept;

		/// @brief false を返します。
		/// @remark co_await 式のオペランドに渡したときに呼び出されます。
		/// @return true
		bool await_ready() const noexcept;

		/// @brief タスクをタスク待ちキューに追加します。
		/// @remark co_await 式のオペランドに渡したときに呼び出されます。
		void await_suspend(std::coroutine_handle<>) const noexcept;

		/// @brief `get()` を呼び出します。
		/// @remark co_await 式のオペランドに渡したときに呼び出されます。
		/// @return 非同期処理の結果
		Type await_resume();

		class promise_type;

	private:
		using Handle = std::coroutine_handle<promise_type>;

		Handle coro;
		std::future<Type> result;

		[[nodiscard]]
		Task(Handle h, std::future<Type> f) noexcept;
	};
}

# include "detail/Task.ipp"
