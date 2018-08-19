#pragma once
#include <mutex>
#include <utility>
#include "BackgroundThread.h"

namespace LandaJune
{
	namespace Threading
	{
		class BackgroundThreadPool 
		{
			using autolock = std::lock_guard<std::mutex>;

			public:
				explicit BackgroundThreadPool(const THREAD_PRIORITY tP = NORMAL, std::string alias = "Anonymous") : _alias(std::move(alias)), _priority(tP) {}
				BackgroundThreadPool(const BackgroundThreadPool &) = delete;
				BackgroundThreadPool(BackgroundThreadPool &&) = delete;
				~BackgroundThreadPool() { stop(); }

				const BackgroundThreadPool & operator = (const BackgroundThreadPool &) = delete;
				BackgroundThreadPool & operator = (BackgroundThreadPool &&) = delete;

				void setAlias(const std::string val) { _alias = std::move(val); }
				std::string alias() const { return _alias;  }

				bool start(const uint64_t len = std::thread::hardware_concurrency());
				void stop();

				void setErrorHandler(std::function<void(std::exception &)> handler);

			/**
				 * \brief Calls a single asynchronious task on first free thread in the pool
				 * \param func - callable function
				 * \param args - function arguments
				 * \return : std::future for callable task, or invalid future in case of no free thread found
				 */
				template <typename Func, typename... Args>
				auto call(Func func, Args... args)
				{
					autolock l(_mutex);
					const auto length = _pool.size();
#if 0
					for (size_t i = 0; i < length; ++i)
					{
						const auto idx = (_next + i) % length;
						auto t = _pool.at(idx).get();
						if (t->isFree())
						{
							_next = idx;
							return t->callTask(func, args...);
						}
					}
					return std::future<void>();
#endif
                    auto it = std::min_element(_pool.begin(), _pool.end(),
                        [](auto & a, auto & b) -> bool { return a.getQueueLength() < b.getQueueLength();});
					return (*it).get()->callTask(func, args...);
				}

				protected:
					std::string _alias;
					std::mutex _mutex {};
					std::vector<std::unique_ptr<BackgroundThread>> _pool;
					size_t _next = 0;
					THREAD_PRIORITY _priority = NORMAL;
		};


		/**
		 * Declaration of thread pools
		 */

		 /**
		 * \brief processingPool provides the thread pool for frame processing
		 * \return
		 */
		ThreadPool& processingPool();

		/**
		  * \brief copyRegionsPool provides the thread pool for independent region copy copying ( obsolete )
		  * \return
		  */
		ThreadPool& copyRegionsPool();

		/**
		  * \brief saveBitmapPool provides the thread pool for saving data to disk ( in various formats )
		  * \return
		  */
		ThreadPool& saveBitmapPool();


		/**
		* \brief imageGenerationThread the only thread which deals with image acquisition ( emulation/generation/capture ), e.g. Frame Producer
		* \return
		*/
		inline BackgroundThread& frameProducerThread()
		{
			static BackgroundThread __th("Frame Producer Thread", 100);
			return __th;
		}

		/**
		* \brief imageProcessingThread the only thread which deals with image processing, including in-memory region copying and algorithmic image processing, e.g. Frame Consumer
		* \return
		*/
		inline BackgroundThread& frameConsumerThread()
		{
			static BackgroundThread __th("Frame Consumer Thread", 200);
			return __th;
		}
	}
}
