#include "FrameRefPool.h"
#include "util.h"
#include "interfaces/IAlgorithmRunner.h"

#include "functions.h"
#include "common/type_usings.h"
#include "common/june_enums.h"

using namespace LandaJune;
using namespace Helpers;
using namespace Algorithms;

#define FRAMECONSUME_SCOPED_LOG PRINT_INFO3 << "[frameConsume func] : "
#define FRAMECONSUME_SCOPED_ERROR PRINT_ERROR << "[frameConsume func] : "
#define FRAMECONSUME_SCOPED_WARNING PRINT_WARNING << "[frameConsume func] : "

void Functions::frameConsume(AlgorithmRunnerPtr algorithmRunner, Core::ICore * coreObject, FrameConsumerCallback dataCallback )
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

	// we should supply a clone of Algorithm Runner for every frame, because it contains frame-related data !!! ?

	try
	{
		frameRunAlgorithms(frameRefObj.get(), std::move(algorithmRunner->clone()));
	}
	catch ( BaseException& bex)
	{
		dataCallback( coreObject, FrameConsumerDataCallbackType::CALLBACK_FRAME_HANDLED, std::make_any<int>(frameRefObj->getIndex()) );

		// we should be aware of exceptions to make sure we release the frame back to pool
		framesPool->release(std::move(frameRefObj));

		// re-throw exception for handling by thread function ( and Core )
		throw;
	}
	dataCallback( coreObject, FrameConsumerDataCallbackType::CALLBACK_FRAME_HANDLED, std::make_any<int>(frameRefObj->getIndex()) );

	framesPool->release(std::move(frameRefObj));
}

