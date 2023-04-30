#pragma once

#include <ctime>
#include <thread>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <climits>
#include <future>
#include <unordered_map>
#include <vector>
#include <memory>

#include <definition.h>

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

		//不允许复制和转移

		Timer(const Timer&) = delete;
		Timer(Timer&&) = delete;

		~Timer()
		{
			this->stop();
		}

		//不允许复制和转移
		Timer& operator=(const Timer&) = delete;
		Timer& operator=(Timer&&) = delete;

		//interval: ms
		template<typename Func, typename... Args>
		void start(int interval, Func&& func, Args&&... args)
		{
			if (!expired)
				return;

			std::function<void()> task = std::bind(std::forward<Func>(func),
				std::forward<Args>(args)...
			);

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
			cv.wait(locker, [this] {return static_cast<bool>(expired); });

			if (expired)
				tryToExpire = false;
		}

	private:
		std::condition_variable cv;
		std::mutex mut;

		std::atomic<bool> expired; //timer stop status
		std::atomic<bool> tryToExpire;//timer is in stop process.
	};

	class Timers final
	{
	public:
		Timers() = default;
		~Timers()
		{
			for (auto i = m_tasks.begin(); i != m_tasks.end(); i++)
			{
				i->second->stop();
			}
		}

		template<typename Func, typename... Args>
		bool addTask(const std::string& taskName, int interval, Func&& func, Args&&... args)
		{
			if (m_tasks.find(taskName) != m_tasks.end())
			{
				return false;
			}

			m_taskNames.push_back(taskName);
			m_tasks[taskName] = std::make_shared<Timer>();
			m_tasks[taskName]->start(interval, std::forward<Func>(func), std::forward<Args>(args)...);

			return true;
		}

		bool addTask(const std::string& taskName, int interval, std::function<void()> function)
		{
			if (m_tasks.find(taskName) != m_tasks.end())
			{
				return false;
			}

			m_taskNames.push_back(taskName);
			m_tasks[taskName] = std::make_shared<Timer>();
			m_tasks[taskName]->start(interval, function);

			return true;
		}

		void removeTask(int postion)
		{
			if (postion >= m_taskNames.size())
				throw THROW_ERROR("The postion is invalid.");
			m_tasks[m_taskNames[postion]]->stop();
			m_tasks.erase(m_taskNames[postion]);
			m_taskNames.erase(m_taskNames.begin() + postion);
		}

		const std::vector<std::string>& getTaskList() const
		{
			return m_taskNames;
		}

	private:
		std::vector<std::string> m_taskNames;
		std::unordered_map<std::string, std::shared_ptr<Timer>> m_tasks;
	};
}
