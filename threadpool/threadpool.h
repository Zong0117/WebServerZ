#pragma once
#include <iostream>
#include <memory>
#include <thread>
#include <queue>
#include <mutex>
#include <cassert>
#include <condition_variable>
#include <functional>



using Task = std::function<void()>;

class ThreadPool
{	
public:
	explicit ThreadPool(int threadNum = 8):
		_pool(std::make_shared<Pool>())
	{
		for (int i = 0; i < threadNum; i++)
		{
			std::thread([pool = _pool] 
			{
				std::unique_lock<std::mutex> locker(pool->mutex);
				while (true)
				{
					if (!pool->tasks.empty())
					{
						auto task = std::move(pool->tasks.front());
						pool->tasks.pop();
						locker.unlock();
						task();
						locker.lock();
					}
					else if (pool->isClose) break;
					else pool->cond.wait(locker);
				}
			}).detach();
		}
	}
	ThreadPool() = default;
	ThreadPool(ThreadPool&&) = default;
	~ThreadPool()
	{
		if (static_cast<bool>(_pool))
		{
			{
				std::lock_guard<std::mutex> locker(_pool->mutex);
				_pool->isClose = true;
			}
			_pool->cond.notify_all();
		}
		
	}

	template<typename T>
	void addTask(T&& task)
	{
		{
			std::lock_guard<std::mutex> locker(_pool->mutex);
			_pool->tasks.emplace(std::forward<T>(task));
		}
		_pool->cond.notify_one();
	}
	
private:
	struct Pool
	{
		std::mutex mutex;
		std::condition_variable cond;
		bool isClose;
		std::queue<std::function<void()>> tasks;
	};
	std::shared_ptr<Pool> _pool;

};
