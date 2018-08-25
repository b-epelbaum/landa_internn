#include "FrameRefPool.h"
#include "util.h"
#include "TaskThreadPool.h"
#include "interfaces/IAlgorithmHandler.h"

#include "functions.h"

using namespace LandaJune;
using namespace Helpers;
using namespace Algorithms;

#define FRAMECONSUME_SCOPED_LOG PRINT_INFO3 << "[frameConsume func] : "
#define FRAMECONSUME_SCOPED_ERROR PRINT_ERROR << "[frameConsume func] : "
#define FRAMECONSUME_SCOPED_WARNING PRINT_WARNING << "[frameConsume func] : "

void Functions::frameConsume(std::shared_ptr<IAlgorithmHandler> algorithmHandler)
{
	// get frame reference object pool
	auto framesPool = Core::FrameRefPool::frameRefPool();
					
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

	// we should supply a new Algorith Handler for every frame
	frameRunAlgorithms(frameRefObj.get(), std::move(algorithmHandler->clone()));
	std::this_thread::sleep_for(std::chrono::milliseconds (100));
	framesPool->release(std::move(frameRefObj));
}

