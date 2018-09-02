#pragma once
#include <exception>
#include <mutex>
#include <future>
#include <utility>

namespace LandaJune
{
	namespace Threading
	{
		enum class THREAD_STATE { IDLE = 0, BUSY };
		enum THREAD_PRIORITY { NORMAL = 0, HIGH };

		class BackgroundThread
		{
			friend class ThreadPool;
			using autolock = std::lock_guard<std::mutex>;
			static void default_error_handler(std::exception &) {}

		public:
			BackgroundThread(std::string name, const int index, const THREAD_PRIORITY tP = NORMAL) : _priority(tP), _name(std::move(name)), _index(index) {}
			BackgroundThread(const BackgroundThread &) = delete;
			BackgroundThread(BackgroundThread &&) = delete;
			~BackgroundThread();
			
			const BackgroundThread & operator = (const BackgroundThread &) = delete;
			BackgroundThread & operator = (BackgroundThread &&) = delete;

			bool start();
			void stop();
			bool join();

			void setErrorHandler(const std::function<void(std::exception &)>& handler)
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
			void setThreadFunction(Func func, Args... args)
			{
				autolock l(_mutex);
				auto boundFunc = std::bind(func, args...);
				std::packaged_task<void()> task(std::move(boundFunc));
				_threafFunc = std::move(task);
			}
	

		protected:

			auto getErrorHandler()
			{
				autolock l(_mutex);
				return std::ref(_error_handler);
			}

			auto getThreadFunction()
			{
				autolock l(_mutex);
				return std::ref(_threafFunc);
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

			static void threadFunction(BackgroundThread *pThis);



		private:

			void setThreadPriority(std::thread::native_handle_type handle) const;

		protected:
			std::mutex _mutex;
			std::unique_ptr<std::thread> _thread;
			std::function<void(std::exception &)> _error_handler = default_error_handler;
			
			std::packaged_task<void()> _threafFunc;
			
			THREAD_STATE _state = THREAD_STATE::IDLE;
			THREAD_PRIORITY _priority = NORMAL;
			std::string _name;
			int _index = 0;
		};
	}
}

