#pragma once
#include <memory>

#include "FrameRefPool.h"
#include "jutils.h"

#include "frameRunAlgorithms.hpp"
#include "TaskThreadPool.h"

using namespace LandaJune::Core;
using namespace LandaJune::Helpers;
using namespace LandaJune::Algorithms;

namespace LandaJune
{
	namespace Functions
	{
		#define FRAMECONSUME_SCOPED_LOG PRINT_INFO3 << "[frameConsume func] : "
		#define FRAMECONSUME_SCOPED_ERROR PRINT_ERROR << "[frameConsume func] : "
		#define FRAMECONSUME_SCOPED_WARNING PRINT_WARNING << "[frameConsume func] : "

		static void frameConsume()
		{
			// get frame reference object pool
			auto framesPool = FrameRefPool::frameRefPool();
					
			auto frameRefObj = framesPool->pullFirstLoaded();
			if (!frameRefObj)
			{
				// TODO : think about configurable sleep 
				// TODO : if no frames from provide the thread eats all CPU time
				static const std::chrono::milliseconds timeout(10);
				std::this_thread::sleep_for(timeout);
				return;
			}

			// start sheet analysis by calling a root function in the same thread
			// we can release frame ref object after all calculation
			// internal analysis functions calls will be parallelized inside the root function recursively
			frameRunAlgorithms(frameRefObj.get());
			
			//auto& algoThreadPool = Threading::TaskThreadPools::algorithmsThreadPool();;
			//auto commonBatchParameters = frameRefObj->getBatchParams();

			/*
			// create regions
			QImage img;
			std::string pathToSave;
			frameCreateRegions(frameRefObj.get());

			// we don't need the future result, because saving the bitmaps is completely asynchronous action
			//saveBitmapPool().call(frameSaveImage, img.copy(), pathToSave);
			fileSavingPool.submit(frameSaveImage, std::move(img.copy()), pathToSave);

			//auto future_performAlgo = imgProcessingPool.call(frameRunAlgorithms, frameRefObj.get(), pParams);
			//auto future_createRegions = regionCopyPool.call(frameCreateRegions, frameRefObj.get(), pParams);

			*/

			//auto future_performAlgo = Threading::TaskThreadPools::postJob(algoThreadPool, frameRunAlgorithms, frameRefObj.get());
			
			//auto future_performAlgo = imgProcessingPool.submit(frameRunAlgorithms, frameRefObj.get());
			//if (future_performAlgo.isValid())
			//{
			//	future_performAlgo.wait();
			//}
			//else
			//{
			//	FRAMECONSUME_SCOPED_WARNING << "no free treads to start calculations. Skipping ....";
			//}

			//if (future_createRegions.valid()) 
			//{
			//	future_createRegions.wait();
			//}
			//else 
			//{
			//	FRAMECONSUME_SCOPED_WARNING << "no free treads to start calculations. Skipping ....";
			//}

			// frame has been handled fully
			// release the frame and return it to a pool of free frame ref objects

			framesPool->release(std::move(frameRefObj));
		};
	}
}
