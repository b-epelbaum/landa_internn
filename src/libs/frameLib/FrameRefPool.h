#pragma once
#include <mutex>
#include "frameRef.h"

namespace LandaJune
{
	namespace Core
	{
		class FrameRefPool
		{
			using autolock = std::lock_guard<std::mutex>;

			public:
				FrameRefPool() = default;

				FrameRefPool(const FrameRefPool &) = delete;
				FrameRefPool(FrameRefPool &&) = delete;
				~FrameRefPool() { cleanup(); }

				static std::shared_ptr<FrameRefPool> frameRefPool();

				const FrameRefPool & operator = (const FrameRefPool &) = delete;
				FrameRefPool & operator = (FrameRefPool &&) = delete;

				void init(const uint64_t size);
				void reset(const uint64_t qlen);
				void cleanup();
				uint64_t size();

				std::unique_ptr<FrameRef> pullFirstFree();
				void release(std::unique_ptr<FrameRef> && frame);
				std::unique_ptr<FrameRef> pullFirstLoaded();

				void pushNextLoaded(std::unique_ptr<FrameRef> && frame);

			private:
				inline static std::shared_ptr<FrameRefPool> __fpool;
				std::mutex _mutex;
				std::list<std::unique_ptr<FrameRef>> _framesFree;
				std::list<std::unique_ptr<FrameRef>> _frameUsed;
				uint64_t _size = 0UL;
		};
	}
}

