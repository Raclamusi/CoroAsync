# include <iostream>
# include <chrono>
# include <type_traits>
# include "CoroAsync/Task.hpp"
# include "CoroAsync/Utility.hpp"

using namespace std::literals::chrono_literals;

// 非同期処理するタスクの定義
//     cra::Task<Type> を戻り値の型に指定し、 co_await もしくは co_return を少なくとも1回使用する。
//     cra::Task<void> は cra::Task<> とも書ける。
cra::Task<> FuncAsync(int id)
{
	std::cout << "FuncAsync(" << id << "): begin\n";

	// co_await で中断ポイントを指定
	//     co_await relTime
	//         タスクを中断し、 relTime の時間経過を待ってからタスクキューの末尾に追加する。
	//         relTime の型は std::uint32_t もしくは std::chrono::duration<Rep, Period> であり、
	//         std::uint32_t の場合は co_await std::chrono::milliseconds{ relTime } と同じ効果を持つ。
	//         relTime の値が 0 以下の時、タスクは何も待たずにタスクキューの末尾に追加される。
	//         void に評価される。
	co_await 0;

	for (int i = 1; i <= 3; ++i)
	{
		std::cout << "FuncAsync(" << id << "): " << i << '\n';
		co_await 100ms;
	}
	std::cout << "FuncAsync(" << id << "): end\n";
}

// 返り値のあるタスクの定義
cra::Task<unsigned int> FibonacciAsync(unsigned int n)
{
	if (n <= 1)
	{
		// co_return でタスクの結果を返して終了
		co_return n;
	}

	// co_await で中断ポイントを指定
	//     co_await task
	//         タスクを中断し、指定したタスクの完了を待ってからタスクキューの末尾に追加する。
	//         task の型は cra::Task<Type> である。
	//         Type に評価される。
	auto ret = co_await FibonacciAsync(n - 1) + co_await FibonacciAsync(n - 2);

	co_return ret;
}

int main()
{
	{
		// タスクの作成
		//     戻り値は必ず受け取るようにする。
		auto task1 = FuncAsync(1);
		auto task2 = FuncAsync(2);
		auto task3 = FuncAsync(3);

		// タスクを待機
		// タイムアウトを指定して待機
		std::cout << "main: wait 150ms for task1\n";
		task1.wait_for(150ms);
		// タスクの完了まで待機
		std::cout << "main: wait for task2\n";
		task2.wait();

		// タスクの完了を確認
		std::cout << "main: task1 is " << (task1.isReady() ? "ready" : "not ready") << "\n";
		std::cout << "main: task2 is " << (task2.isReady() ? "ready" : "not ready") << "\n";
		std::cout << "main: task3 is " << (task3.isReady() ? "ready" : "not ready") << "\n";

		// タスクを中断
		//     以下の場合、タスクは中断され、それ以降は実行されない。
		//       - cra::Task::destroy を呼び出した場合
		//       - 別のタスクや空のタスクが代入された場合
		//       - デストラクタが呼び出された場合
		// t3.destroy();
		// t3 = {};
	}
	std::cout << "----------\n";
	{
		auto fib5 = FibonacciAsync(5);
		auto fib10 = FibonacciAsync(10);

		// タスクの結果を取得
		//     タスクが完了していない場合、完了まで待機する。
		std::cout << "main: fib5 = " << fib5.get() << "\n";
		std::cout << "main: fib10 = " << fib10.get() << "\n";

		// タスクを生成して結果を即取得
		std::cout << "main: fib15 = " << FibonacciAsync(15).get() << "\n";
	}
	std::cout << "----------\n";
	{
		// ラムダ式によるタスクの定義
		auto getPiAsync = []() -> cra::Task<double>
		{
			co_return 3.14159;
		};

		auto piTask = getPiAsync();
		auto pi = piTask.get();
		std::cout << "main: pi = " << pi << "\n";

		// タスクの定義と作成、待機を同時にすることも可能
		auto e = []() -> cra::Task<double> { co_return 2.71828; }().get();
		std::cout << "main: e = " << e << "\n";
	}
	std::cout << "----------\n";
	{
		auto t1 = []() -> cra::Task<std::string> { co_return "Hello"; }();
		auto t2 = []() -> cra::Task<std::string> { co_return "Good-bye"; }();

		// すべてのタスクの完了を待つタスクの作成
		//     cra::WhenAll は渡したタスクがすべて完了するのを待つタスクを返す。
		//     タスクは値で受け取るため、その場で作成するか、ムーブする必要がある。
		//     結果は std::tuple で受け取れる。このとき、結果の型が void であるものは除かれる。
		//     例えば、 WhenAll(Task<int>, Task<>, Task<std::string>) の戻り値の型は Task<std::tuple<int, std::string>> である。
		auto whenAllTask = cra::WhenAll(
			[]() -> cra::Task<int> { co_return 42; }(),
			FuncAsync(4),
			std::move(t1)
		);
		auto [a, b] = whenAllTask.get();
		std::cout << "main: WhenAll result: { " << a << ", " << b << " }\n";

		// いずれかのタスクの完了を待つタスクの作成
		//     cra::WhenAny は渡したタスクがいずれかが完了するのを待つタスクを返す。
		//     タスクを値で受け取るものと、非 const 参照で受け取るものがオーバーロードで用意されている。
		//     最初に完了したタスクに対して処理を行いつつ、最終的にはすべてのタスクの完了を待ちたい場合などに、後者を用いることができる。
		//     結果は最初に完了したタスク（参照で渡した場合は参照）が std::variant で受け取れる。
		//     std::variant::index() で何番目のタスクが最初に完了したかを知ることができる。
		auto whenAnyTask = cra::WhenAny(
			[]() -> cra::Task<> { co_await 24h; }(),
			FuncAsync(5),
			std::move(t2)
		);
		auto whenAnyResult = whenAnyTask.get();
		std::cout << "main: t" << whenAnyResult.index() << " was done first in WhenAny(t0, t1, t2)\n";
		std::visit([]<class T>(cra::Task<T>& task) {
			std::cout << "main: WhenAny result: ";
			if constexpr (std::is_void_v<T>) std::cout << "(void)";
			else std::cout << task.get();
			std::cout << "\n";
		}, whenAnyResult);
	}
}

// 出力
/*
main: wait 150ms for task1
FuncAsync(1): begin
FuncAsync(2): begin
FuncAsync(3): begin
FuncAsync(1): 1
FuncAsync(2): 1
FuncAsync(3): 1
FuncAsync(1): 2
FuncAsync(2): 2
FuncAsync(3): 2
main: wait for task2
FuncAsync(1): 3
FuncAsync(2): 3
FuncAsync(3): 3
FuncAsync(1): end
FuncAsync(2): end
main: task1 is ready
main: task2 is ready
main: task3 is not ready
----------
main: fib5 = 5
main: fib10 = 55
main: fib15 = 610
----------
main: pi = 3.14159
main: e = 2.71828
----------
FuncAsync(4): begin
FuncAsync(4): 1
FuncAsync(4): 2
FuncAsync(4): 3
FuncAsync(4): end
main: WhenAll result: { 42, Hello }
FuncAsync(5): begin
main: t2 was done first in WhenAny(t0, t1, t2)
main: WhenAny result: Good-bye
*/
