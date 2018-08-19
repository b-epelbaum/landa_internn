#pragma once
#include <exception>
#include <mutex>
#include <future>
#include <utility>
#include <queue>
#include <vector>
#include <list>

namespace LandaJune
{
	namespace Core
	{
		enum class THREAD_STATE { IDLE = 0, BUSY };
		enum THREAD_PRIORITY { NORMAL = 0, HIGH };

		class Thread
		{
			friend class ThreadPool;
			using autolock = std::lock_guard<std::mutex>;
			static void default_error_handler(std::exception &) {}

		public:
			Thread(std::string name, const int index, const THREAD_PRIORITY tP = NORMAL) 
				: _priority(tP), _name(std::move(name)), _max_task_queue_size(128u), _index(index) {
			}
			Thread(const Thread &) = delete;
			Thread(Thread &&) = delete;
			~Thread();
			
			const Thread & operator = (const Thread &) = delete;
			Thread & operator = (Thread &&) = delete;

			bool start();
			void stop();
			bool join();

			void setErrorHandler(const std::function<void(std::exception &)> handler)
			{
				autolock l(_mutex);
				_error_handler = handler;
			}

			/**
			 * \brief Sets a background ( periodic task ), e.g. frame producing/consuming
			 * \param func - function to execute
			 * \param args - function arguments
			 */
			template <typename Func, typename... Args>
			void setBackgroundTask(Func func, Args... args)
			{
				autolock l(_mutex);
				auto boundFunc = std::bind(func, args...);
				std::packaged_task<void()> task(std::move(boundFunc));
				_task_bg = std::move(task);
			}

			/**
			 * \brief Sets a single callable function for performing it in main function loop
			 * \param func - function to invoke
			 * \param args - function arguments
			 * \return std::future for std::packaged_task, which will be performed in main function loop
			 */
			template <typename Func, typename... Args>
			auto callTask(Func func, Args... args)
			{
				autolock l(_mutex);
				if (_task.size() > _max_task_queue_size)
				{
					throw std::runtime_error("Thread::call current task still active.");
				}
				
				auto boundFunc = std::bind(func, args...);
				std::packaged_task<void()> task(std::move(boundFunc));
				_task.push(std::move(task));
				return _task.back().get_future();
			}

		protected:

			auto getErrorHandler()
			{
				autolock l(_mutex);
				return std::ref(_error_handler);
			}

			auto getBackgroundTask()
			{
				autolock l(_mutex);
				return std::ref(_task_bg);
			}

			auto getTask()
			{
				autolock l(_mutex);
				if (_task.empty()) {
				    std::packaged_task<void()> task;
					return std::move(task);
				}
				auto task = std::move(_task.front());
				_task.pop(); return std::move(task);
			}

			size_t getQueueLength(void)
			{
				autolock l(_mutex);
				return _task.size();
			}

			void setName(std::string state)
			{
				autolock l(_mutex);
				_name = std::move(state);
			}

			std::string getName()
			{
				autolock l(_mutex);
				return _name;
			}

			void setIndex(const int idx)
			{
				autolock l(_mutex);
				_index = idx;
			}

			int getIndex()
			{
				autolock l(_mutex);
				return _index;
			}

			void setState(const THREAD_STATE state)
			{
				autolock l(_mutex);
				_state = state;
			}

			THREAD_STATE getState()
			{
				autolock l(_mutex);
				return _state;
			}

			size_t getMaxTaskQueueSize(void) {
				autolock l(_mutex);
				return _max_task_queue_size;
			}
			void setMaxTaskQueueSize(size_t max_task_queue_size) {
				autolock l(_mutex);
				_max_task_queue_size = max_task_queue_size;
			}

			static void threadFunction(Thread *pThis);

		private:

			void setThreadPriority(std::thread::native_handle_type handle) const;

		protected:
			std::mutex _mutex;
			std::unique_ptr<std::thread> _thread;
			std::function<void(std::exception &)> _error_handler = default_error_handler;
			
			std::packaged_task<void()> _task_bg;
			std::queue<std::packaged_task<void()>> _task;
			size_t _max_task_queue_size;
			
			THREAD_STATE _state = THREAD_STATE::IDLE;
			THREAD_PRIORITY _priority = NORMAL;
			std::string _name;
			int _index = 0;
		};
	}
}

