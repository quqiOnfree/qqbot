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

#include "definition.h"

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
		void start(long long interval, Func&& func, Args&&... args)
		{
			if (!static_cast<bool>(expired))
				return;

			std::function<void()> task = std::bind(std::forward<Func>(func),
				std::forward<Args>(args)...
			);

			expired = false;
			std::thread([this, interval, task]()
				{
				while (!static_cast<bool>(tryToExpire))
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
			if (static_cast<bool>(expired))
				return;

			if (static_cast<bool>(tryToExpire))
				return;

			tryToExpire = true;

			std::unique_lock<std::mutex> locker(mut);
			cv.wait(locker, [this] {return static_cast<bool>(expired); });

			if (static_cast<bool>(expired))
				tryToExpire = false;

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

	private:
		std::condition_variable cv;
		std::mutex mut;

		std::atomic<bool> expired; //timer stop status
		std::atomic<bool> tryToExpire;//timer is in stop process.
	};

	class Timers
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

		/// <summary>
		/// 添加函数任务
		/// </summary>
		/// <typeparam name="Func">函数类型</typeparam>
		/// <typeparam name="...Args">函数参数类型</typeparam>
		/// <param name="taskName">任务名</param>
		/// <param name="interval">运行间隔（秒）</param>
		/// <param name="func">函数</param>
		/// <param name="...args">参数</param>
		/// <returns>如果已经有此任务就返回false，没有就返回true</returns>
		template<typename Func, typename... Args>
		bool addTask(const std::string& taskName, long long interval, Func&& func, Args&&... args)
		{
			if (m_tasks.find(taskName) != m_tasks.end())
			{
				return false;
			}

			m_taskNames.push_back(taskName);
			m_tasks[taskName] = std::make_shared<Timer>();
			m_tasks[taskName]->start(interval * 1000ll, std::forward<Func>(func), std::forward<Args>(args)...);

			return true;
		}

		/// <summary>
		/// 添加函数任务
		/// </summary>
		/// <param name="taskName">任务名</param>
		/// <param name="interval">运行间隔（秒）</param>
		/// <param name="function">函数</param>
		/// <returns>如果已经有此任务就返回false，没有就返回true</returns>
		bool addTask(const std::string& taskName, long long interval, std::function<void()> function)
		{
			if (m_tasks.find(taskName) != m_tasks.end())
			{
				return false;
			}

			m_taskNames.push_back(taskName);
			m_tasks[taskName] = std::make_shared<Timer>();
			m_tasks[taskName]->start(interval * 1000ll, function);

			return true;
		}

		/// <summary>
		/// 删除任务
		/// </summary>
		/// <param name="postion">任务在任务列表中的位置</param>
		void removeTask(long long postion)
		{
			if (postion >= m_taskNames.size())
				throw THROW_ERROR("The postion is invalid.");
			m_tasks[m_taskNames[postion]]->stop();
			m_tasks.erase(m_taskNames[postion]);
			m_taskNames.erase(m_taskNames.begin() + postion);
		}

		/// <summary>
		/// 获取任务列表
		/// </summary>
		/// <returns>任务列表</returns>
		const std::vector<std::string>& getTaskList() const
		{
			return m_taskNames;
		}

	private:
		std::vector<std::string> m_taskNames;
		std::unordered_map<std::string, std::shared_ptr<Timer>> m_tasks;
	};
}
