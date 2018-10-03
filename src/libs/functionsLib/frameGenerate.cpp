#include "functions.h"
#include "FrameRefPool.h"
#include "applog.h"
#include "util.h"
#include "RealTimeStats.h"
#include "interfaces/IFrameProvider.h"
#include "common/june_enums.h"

using namespace LandaJune;
using namespace Helpers;

/**
 * frameGenerate free static function
 * This function is a main entry point of one of two "main" processing threads :
*	- frame generating thread
*	- frame consuming thread
*	
*	The function rus in thread with a timeout of 1 millisecond sleep ( in case of empty implementation - see thread implementation )
*	and tries to acquire one free frameReference object ( slot ) from the frame reference object list.
*	Once it succeeds, it fills the frame object with needed metadata and invokes the current image provider for accessing 
*	bits of a new incoming image.
*	
*	After bits acquisition, the function pushes prepared frame reference object back to the queue.
*	The prepared frame object is waiting for a first free thread for handling.
*	
 * 
 */


#define FRAMEGENERATE_SCOPED_LOG PRINT_INFO3 << "[frameGenerate func] : "
#define FRAMEGENERATE_SCOPED_ERROR PRINT_ERROR << "[frameGenerate func] : "
#define FRAMEGENERATE_SCOPED_WARNING PRINT_WARNING << "[frameGenerate func] : "

//void Functions::frameGenerate(FrameProviderPtr frameProvider, Core::ICore * coreObject, std::function<void( Core::ICore *, std::shared_ptr<Core::SharedFrameData>)> viewFunc)
void Functions::frameGenerate(FrameProviderPtr frameProvider, Core::ICore * coreObject, FrameProviderCallback dataCallback)
{
	// get a first free 
	auto framePool = Core::FrameRefPool::frameRefPool();
	auto frameRef = framePool->pullFirstFree();

	if (!frameRef) // no free frame - all threads are busy handling another frames
	{
		static auto _droppedFrameCount = 0;
		if (frameProvider->warnAboutDroppedFrames())
		{
			//FRAMEGENERATE_SCOPED_WARNING << "No free FrameRef object in pool ! [ FRAME # " << frameProvider->getCurrentFrameIndex() << " DROPPED] Total dropped : " << ++_droppedFrameCount;
			//RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_acquiredFramesFail, 1.0e-6);
		}

		static const std::chrono::milliseconds timeout(frameProvider->getFrameDropDelayTimeout());
		std::this_thread::sleep_for(timeout);
		return;
	}

	const auto& tStart = Utility::now_in_microseconds();
	// set provider name to the frame
	frameRef->setNamedParameter(NAMED_PROPERTY_PROVIDER_NAME, frameProvider->getName().toStdString());

	// assign postData callback to the frameRefObject
	using namespace std::placeholders;
	std::function<void(Core::FrameRef*)> f = std::bind(&IFrameProvider::releaseData, frameProvider, _1);
	frameRef->setPostDataFunction(f);

	// prepare data for pushing to the frameObject
	auto retVal = frameProvider->prepareData(frameRef.get());
	if (retVal != RESULT_OK)
	{
		framePool->release(std::move(frameRef));
		if (frameProvider->canContinue(retVal))
		{
			return;
		}
		THROW_EX_ERR(retVal);
	}

	// perform an actual acquisition
	retVal = frameProvider->accessData (frameRef.get());
	if (retVal != RESULT_OK)
	{
		framePool->release(std::move(frameRef));
		FRAMEGENERATE_SCOPED_ERROR << "Image provider failed getting frame. Error : " << retVal;
		if (frameProvider->canContinue(retVal))
			return;
		THROW_EX_ERR(retVal);
	}

	// call viewer function to pass image data for viewing
	dataCallback( coreObject
				, FrameProviderDataCallbackType::CALLBACK_FRAME_DATA
				, std::make_any<std::shared_ptr<Core::SharedFrameData>>(
									std::make_shared<Core::SharedFrameData>(
												frameRef.get(), frameProvider->getFrameLifeSpan()
											)
					)
	);
		
	
	// push loaded frame reference object to queue
	framePool->pushNextLoaded(std::move(frameRef));
	
	const auto& tFinish = Utility::now_in_microseconds();

	if (retVal == RESULT_OK)
	{
		RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_acquiredFramesOk, (tFinish - tStart) * 1.0e-6);
	}
	else
	{
		RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_acquiredFramesFail, (tFinish - tStart) * 1.0e-6);
	}
}