# CoroAsync

CoroAsync は C++20 でコルーチンによる非同期処理をするためのライブラリです。

## 特徴

CoroAsync は、 C++20 で追加された[コルーチン](https://cpprefjp.github.io/lang/cpp20/coroutines.html)を利用して非同期処理を行うためのライブラリです。
シングルスレッドで動作するため、スレッドを使えない環境でも非同期処理ができます。

CoroAsync を利用した非同期処理は、スレッドに比べて排他制御と中断が簡単という特徴があります。

CoroAsync で作成するタスクは `co_await` で中断ポイントを指定します。
タスクはこの中断ポイントでしか割り込まれず、それ以外の箇所では必ず同期的に実行されます。
さらに、タスクは1スレッドで処理されるため、2つ以上のタスクが同じ時刻に実行されることはありません。
そのため、スレッドより排他制御が簡単に行えます。

また、タスクが未完了のまま代入演算子やデストラクタによって破棄されるとき、タスクは中断され、それ以上実行されることはありません。スレッドのように、停止要求を確認してタスクを終了させるようなことをせずとも、タスクを簡単に中断できます。

<details>
    <summary><b>スレッドによる非同期処理の例</b></summary>

```c++
#include <iostream>
#include <thread>
#include <chrono>

std::mutex cout_mutex;

void count_up(std::stop_token st, int id) {
    int i = 0;
    while (not st.stop_requested()) {  // 停止要求の監視が必要
        std::lock_guard lock{ cout_mutex };  // mutex などによる排他制御が必要
        std::cout << "task(" << id << "): " << i++ << '\n';
    }
}

int main() {
    using namespace std::literals::chrono_literals;

    std::jthread task1{ count_up, 1 };
    std::jthread task2{ count_up, 2 };

    std::this_thread::sleep_for(1ms);
}
```

</details>

<details open>
    <summary><b>CoroAsync による非同期処理の例</b></summary>

```c++
#include <iostream>
#include <chrono>
#include "CoroAsync/Task.hpp"
#include "CoroAsync/TaskQueue.hpp"

cra::Task<> count_up(int id) {
    int i = 0;
    while (true) {  // 停止要求の監視は不要
        std::cout << "task(" << id << "): " << i++ << '\n';
        co_await 0;  // mutex などによる排他制御は必要、 co_await 以外で割り込まれない
    }
}

int main() {
    using namespace std::literals::chrono_literals;

    auto task1 = count_up(1);
    auto task2 = count_up(2);

    cra::TaskQueue::RunFor(1ms);
}
```

</details>

## 使用上の注意

CoroAsync はシングルスレッドでの仕様を想定しているため、スレッドセーフな実装はしていません。
複数のスレッドで CoroAsync を使用するときは、十分にご注意ください。

## 使用方法

このリポジトリを clone するかダウンロードし、 CoroAsync/ を作業ディレクトリにコピーするかインクルードパスに含めることで使用します。
CoroAsync はヘッダオンリーなので、インクルードするだけで使用できます。

詳しい使用方法は、 [Sample.cpp](https://github.com/Raclamusi/CoroAsync/blob/main/Sample.cpp?ts=4) やヘッダファイルの Doxygen コメントをご覧ください。

## サンプル

Wandbox でサンプルを試す: https://wandbox.org/permlink/qEg1Vyl1TiD5Xix6

Siv3D for Web のサンプルを見る: https://raclamusi.github.io/CoroAsync/SampleForSiv3DForWeb.html

サンプル: https://github.com/Raclamusi/CoroAsync/blob/main/Sample.cpp?ts=4

Siv3D 向けサンプル: https://github.com/Raclamusi/CoroAsync/blob/main/SampleForSiv3D.cpp?ts=4

Siv3D for Web 向けサンプル: https://github.com/Raclamusi/CoroAsync/blob/main/SampleForSiv3DForWeb.cpp?ts=4

## 動作を確認した環境

- GCC 11.2.0
- Visual Studio 2022 version 17.4.2
- Emscripten 3.1.20
