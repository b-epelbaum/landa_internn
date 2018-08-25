#pragma once

#include "GenericSafeQueue.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>
#include <limits>

#include "applog.h"
#include <Windows.h>

#define TPOOL_SCOPED_LOG PRINT_INFO4 << "[ThreadPool] : "
#define TPOOL_SCOPED_ERROR PRINT_ERROR << "[ThreadPool] : "

namespace LandaJune
{
	namespace Threading
	{
		class TaskThreadPool
		{
			class IThreadJob
			{
				public:
					IThreadJob() = default;
					virtual ~IThreadJob() = default;
					IThreadJob(const IThreadJob& other) = delete;
					IThreadJob& operator=(const IThreadJob& other) = delete;
					IThreadJob(IThreadJob&& other) = default;
					IThreadJob& operator=(IThreadJob&& other) = default;

					/**
					* Run the task.
					*/
					virtual void execute() = 0;
			};

			template <typename Func>
			class ThreadJob : public IThreadJob
			{
				public:
				explicit ThreadJob(Func&& func)
						:_func{ std::move(func) }
					{
					}

					~ThreadJob() override = default;
					ThreadJob(const ThreadJob& other) = delete;
					ThreadJob& operator=(const ThreadJob& other) = delete;
					ThreadJob(ThreadJob&& other) = default;
					ThreadJob& operator=(ThreadJob&& other) = default;

					/**
					* Run the task.
					*/
					void execute() override
					{
						_func();
					}

				private:
					Func _func;
			};

			public:
			
				/**
				* A wrapper around a std::future that adds the behavior of futures returned from std::async.
				* Specifically, this object will block and wait for execution to finish before going out of scope.
				*/
				template <typename T>
				class JobFuture
				{
				public:
					explicit JobFuture(std::future<T>&& future)
						: _future{ std::move(future) }
					{
					}

					JobFuture(const JobFuture& other) = delete;
					JobFuture& operator=(const JobFuture& other) = delete;
					JobFuture(JobFuture&& other) = default;
					JobFuture& operator=(JobFuture&& other) = default;
					bool isValid()  { return _future.valid(); }
					void wait() {
						if (_future.valid())
						{
							_future.wait();
						}
					}
					~JobFuture()
					{
						if (_future.valid())
						{
							_future.get();
						}
					}

					auto get()
					{
						if (_future.valid())
						{
							return _future.get();
						}
						return T();
					}


				private:
					std::future<T> _future;
				};


			public:
				
				/**
				* Constructor.
				*/
				explicit TaskThreadPool(std::string alias = "Anonymous", const std::uint32_t numThreads = 0 )
					:_done{ false }
					, _alias(std::move(alias))
				{
					TPOOL_SCOPED_LOG << "starting " << _alias.c_str() << " thread pool...";
					auto numActualThreads = numThreads;
					
					#undef max
					if (numThreads == 0)
						numActualThreads = std::max( std::thread::hardware_concurrency(), 2u ) - 1u;
					
					try
					{
						for (auto i = 0u; i < numActualThreads; ++i)
						{
							_threads.emplace_back(&TaskThreadPool::worker, this, i);
						}
					}
					catch (...)
					{
						destroy();
						throw;
					}
				}

				/**
				* Non-copyable.
				*/
				TaskThreadPool(const TaskThreadPool& other) = delete;

				/**
				* Non-assignable.
				*/
				TaskThreadPool& operator=(const TaskThreadPool& other) = delete;

				/**
				* Destructor.
				*/
				~TaskThreadPool(void)
				{
					destroy();
				}

				/**
				* Submit a job to be run by the thread pool.
				*/
				template <typename Func, typename... Args>
				auto submit(Func&& func, Args&&... args)
				{
					auto boundTask = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
					
					using ResultType = std::result_of_t<decltype(boundTask)()>;
					
					using PackagedTask = std::packaged_task<ResultType()>;
					using TaskType = ThreadJob<PackagedTask>;

					PackagedTask task{ std::move(boundTask) };
					JobFuture<ResultType> result{ task.get_future() };
					_jobQueue.push(std::make_unique<TaskType>(std::move(task)));
					return result;
				}

		private:
			/**
			* Constantly running function each thread uses to acquire work items from the queue.
			*/
			void worker(int idx)
			{
				PRINT_INFO7 << "[Thread: " << this->_alias.c_str() << "; IDX: " << idx << "(" << GetCurrentThreadId() << ")] started on CPU #" << GetCurrentProcessorNumber();
				while (!_done)
				{
					std::unique_ptr<IThreadJob> pTask{ nullptr };
					if (_jobQueue.waitPop(pTask))
					{
						pTask->execute();
					}else
					{
						static const std::chrono::milliseconds timeout(0);
						std::this_thread::sleep_for(timeout);
					}
				}
			}

			/**
			* Invalidates the queue and joins all running threads.
			*/
			void destroy()
			{
				_done = true;
				_jobQueue.invalidate();
				for (auto& thread : _threads)
				{
					if (thread.joinable())
					{
						thread.join();
					}
				}
			}

		private:
			std::atomic_bool _done;
			GenericSafeQueue<std::unique_ptr<IThreadJob>> _jobQueue;
			std::vector<std::thread> _threads;
			std::string _alias;
		};

		namespace TaskThreadPools
		{
			
			inline TaskThreadPool& diskDumperThreadPool()
			{
				static TaskThreadPool __diskThreadPool("Disk Dumper Thread Pool", 2);
				return __diskThreadPool;
			}

			/**
			* This pool is created with std::thread::hardware_concurrency() - 1 threads.
			*/

			inline TaskThreadPool& algorithmsThreadPool()
			{
				static TaskThreadPool __algoThreadPool("Algorithm Thread Pool");
				return __algoThreadPool;
			}

			/**
			* Submit a job to the thread pool.
			*/
			template <typename Func, typename... Args>
			auto postJob(TaskThreadPool& pool, Func&& func, Args&&... args)
			{
				return pool.submit(std::forward<Func>(func), std::forward<Args>(args)...);
			}
		}

		template <class T>
		using FUTURE_VECTOR = std::vector<TaskThreadPool::JobFuture<T>>;
		
		#define WAIT_ALL(list) std::for_each(list.begin(), list.end(), [](auto &f) { f.wait(); });
	}
}


