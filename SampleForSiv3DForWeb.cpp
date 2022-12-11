# include <Siv3D.hpp>
# include <list>
# include "CoroAsync/Task.hpp"
# include "CoroAsync/TaskQueue.hpp"

class Bar
{
public:
	Bar(const Point& pos)
		: m_value{ 0 },
		  m_rect{ Arg::center = pos, 150, 30 },
		  m_rectBar{ Arg::center = pos, 100, 10 } {}

	void update()
	{
		if (m_value < 100)
		{
			++m_value;
		}
	}

	void draw() const
	{
		m_rect
			.drawShadow(Vec2{ 2, 2 }, 8, 1)
			.draw(m_value == 100 ? Palette::Yellow : Palette::White);
		m_rectBar.draw(Palette::Gray);
		m_rectBar.stretched(0, m_value - 100, 0, 0).draw(Palette::Lightgreen);
	}

	void setValue(uint32 value)
	{
		m_value = Clamp(value, 0u, 100u);
	}

private:
	uint32 m_value;
	Rect m_rect;
	Rect m_rectBar;
};

cra::Task<> CountUpAsync()
{
	int32 i = 0;
	while (true)
	{
		Print << i++;
		co_await 100ms;
	}
}

void Main()
{
	Scene::SetBackground(ColorF{ 0.8, 0.9, 1.0 });
	Scene::SetResizeMode(ResizeMode::Actual);

	auto task = CountUpAsync();
	std::list<Bar> bars;
	Array<cra::Task<>> barTasks;
	while (System::Update())
	{
		if (barTasks.size() >= 1000)
		{
			barTasks.remove_if([](auto&& task) { return task.isReady(); });
		}

		for (auto&& bar : bars)
		{
			bar.draw();
		}

		Circle{ Cursor::Pos(), 40 }.draw(ColorF{ 1, 0, 0, 0.5 });

		if (SimpleGUI::Button(U"タスクを生成", Vec2{ 100, 50 }))
		{
			bars.push_front(Bar{ Point{ RandomClosed(0, Scene::Width()), RandomClosed(0, Scene::Height()) } });
			barTasks << [&](auto it, uint32 duration) -> cra::Task<>
			{
				for ([[maybe_unused]] auto i : step(100))
				{
					co_await duration;
					it->update();
				}
				co_await 1s;
				bars.erase(it);
			}(bars.begin(), RandomClosed(10, 50));
		}

		// cra::TaskQueue::RunFor で指定時間だけタスクを実行する。
		// 60FPSに間に合うような待機時間を設定する。
		cra::TaskQueue::RunFor(5ms);
	}
}
