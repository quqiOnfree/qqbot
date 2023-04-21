#pragma once

#include <ctime>
#include <thread>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <climits>
#include <future>

namespace qqbot
{
	class Timer
	{
	public:
		Timer()
		{
			expired = true;
			tryToExpire = false;
		}

		~Timer()
		{
			this->stop();
		}

		//interval: ms
		template<typename Func, typename... Args>
		void start(int interval, Func&& func, Args&&... args)
		{
			std::function<void()> task = std::bind(std::forward<Func>(func),
				std::forward<Args>(args)...
			);

			if (expired == false)
				return;

			expired = false;
			std::thread([this, interval, task]()
				{
				while (!tryToExpire)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(interval));
					task();
				}
				std::lock_guard<std::mutex> locker(mut);
				expired = true;
				cv.notify_one();

				}).detach();
		}

		//停止定时器
		void stop()
		{
			if (expired)
				return;

			if (tryToExpire)
				return;

			tryToExpire = true;

			std::unique_lock<std::mutex> locker(mut);
			cv.wait(locker, [this] {return expired == true; });

			if (expired == true)
				tryToExpire = false;
		}

	private:
		std::condition_variable cv;
		std::mutex mut;

		std::atomic<bool> expired; //timer stop status
		std::atomic<bool> tryToExpire;//timer is in stop process.
	};
}
