#pragma once
#include <mutex>
#include "frameRef.h"

namespace LandaJune
{
	namespace FrameProviders
	{
		class FrameRefPool
		{
			using autolock = std::lock_guard<std::mutex>;

			public:
				FrameRefPool() = default;

				FrameRefPool(const FrameRefPool &) = delete;
				FrameRefPool(FrameRefPool &&) = delete;
				~FrameRefPool() { clear(); }

				static std::shared_ptr<FrameRefPool> frameRefPool();

				const FrameRefPool & operator = (const FrameRefPool &) = delete;
				FrameRefPool & operator = (FrameRefPool &&) = delete;

				void reset(const uint64_t qlen, const FrameRef::GLOBAL_FRAME_DATA& globalFrameRefData)
				{
					clear();
					init(qlen, globalFrameRefData);
				}

				void clear()
				{
					autolock lock(_mutex);
					_framesFree.clear();
					_frameUsed.clear();
				}

				void init(const uint64_t size, const FrameRef::GLOBAL_FRAME_DATA& globalFrameRefData)
				{
					autolock lock(_mutex);
					for (uint64_t i = 0; i < size; ++i)
					{
						_framesFree.push_back(std::make_unique<FrameRef>(globalFrameRefData));
					}
					_size = size;
				}
				
				uint64_t size()
				{
					autolock lock(_mutex);
					return _size;
				}

				std::unique_ptr<FrameRef> pullFirstFree()
				{
					autolock lock(_mutex);
					if (_framesFree.empty())
					{
						return std::unique_ptr<FrameRef>();
					}
					auto frame = std::move(_framesFree.front());
					_framesFree.pop_front();
					return frame;
				}

				void release(std::unique_ptr<FrameRef> && frame)
				{
					autolock lock(_mutex);
					frame->reset();
					_framesFree.push_back(std::move(frame));
				}


				std::unique_ptr<FrameRef> pullFirstLoaded()
				{
					autolock lock(_mutex);
					if (_frameUsed.empty())
					{
						return std::unique_ptr<FrameRef>();
					}
					auto frame = std::move(_frameUsed.front());
					_frameUsed.pop_front(); return frame;
				}

				void pushNextLoaded(std::unique_ptr<FrameRef> && frame)
				{
					autolock lock(_mutex);
					_frameUsed.push_back(std::move(frame));
				}

			private:
				std::mutex _mutex;
				std::list<std::unique_ptr<FrameRef>> _framesFree;
				std::list<std::unique_ptr<FrameRef>> _frameUsed;
				uint64_t _size = 0UL;

				inline static std::shared_ptr<FrameRefPool> __fpool;
		};

		inline std::shared_ptr<FrameRefPool> FrameRefPool::frameRefPool()
		{
			if (!__fpool)
			{
				__fpool = std::make_shared<FrameRefPool>();
			}
			return __fpool;
		}

	}
}

