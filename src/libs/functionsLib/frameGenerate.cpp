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


#define FRAMEGENERATE_SCOPED_LOG PRINT_INFO8 << "[frameGenerate func] : "
#define FRAMEGENERATE_SCOPED_ERROR PRINT_ERROR << "[frameGenerate func] : "
#define FRAMEGENERATE_SCOPED_WARNING PRINT_WARNING << "[frameGenerate func] : "

CORE_ERROR Functions::frameGenerate(
			BaseParametersPtr parameters,
			FrameProviderPtr frameProvider, 
			Core::ICore * coreObject, 
			CoreEventCallback coreCallback
)
{
	
	// sanity check
	if (!parameters)
		return CORE_ERROR::ERR_FUNC_INVALID_PARAM_PTR;

	if (!frameProvider)
		return CORE_ERROR::ERR_FUNC_INVALID_PROVIDER_PTR;

	if (!coreObject)
		return CORE_ERROR::ERR_FUNC_INVALID_CORE_OBJECT;

	if (!coreCallback)
		return CORE_ERROR::ERR_FUNC_INVALID_CORE_CALLBACK;

	auto framePool = Core::FrameRefPool::frameRefPool();
	std::unique_ptr<Core::FrameRef> frameRefObj;
	CORE_ERROR retVal = CORE_ERROR::RESULT_OK;

	// as a rule, return values mean the provider can coninue, or indicate a certain state
	// Thrown Exceptions mean that there is no option to continue image generation
	// Any exception leads to stopping a provider thread and provider cleanup.
	// However, the Core object shoudl examine the exception error code 
	// to differentiate between runtime errors and normal provider completion 
	// ( such as reaching a maximum image count, or finishing all source images in folder

	const auto& tStart = Utility::now_in_microseconds();
	try
	{
		if ( !frameProvider->isInited ())
		{
			frameProvider->init(parameters, coreObject, coreCallback );
			return RESULT_OK;
		}

		// get a first free 
		frameRefObj = framePool->pullFirstFree();

		if (!frameRefObj) // no free frame - all threads are busy handling another frames
		{
			// TODO : think about configurable sleep 
			// TODO : if no frames from provide the thread eats all CPU time
			// TODO : notify core about skipped frame

			if (frameProvider->shouldReportSkippedFrame())
			{
				RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_acquiredFramesSkipped, 1);
				if (coreCallback)
					coreCallback( coreObject, CoreCallbackType::CALLBACK_PROVIDER_FRAME_SKIPPED, std::make_any<int>(1) );
			}

			static const std::chrono::milliseconds timeout(frameProvider->getFrameDropDelayTimeout());
			std::this_thread::sleep_for(timeout);
			return RESULT_OK;
		}

	
		// set provider name to the frame
		frameRefObj->setNamedParameter(NAMED_PROPERTY_PROVIDER_NAME, frameProvider->getName().toStdString());

		// assign postData callback to the frameRefObject
		using namespace std::placeholders;
		std::function<void(Core::FrameRef*)> f = std::bind(&IFrameProvider::releaseData, frameProvider, _1);
		frameRefObj->setPostDataFunction(f);

		// prepare data for pushing to the frameObject
		retVal = frameProvider->prepareData(frameRefObj.get());

		
		// the only acceptale return value for prepare data is ERR_PROVIDER_UNINITIALIZED, which should be handled by init call
		// otherwise, the return value converted to exception and caught on catch block, leading to stopping the provider thread and cleanup

		// if not inited yet
		// init and return;
		if (retVal != RESULT_OK)
		{
			if (retVal == CORE_ERROR::ERR_PROVIDER_UNINITIALIZED )
			{
				framePool->release(std::move(frameRefObj));
				frameProvider->init(parameters, coreObject, coreCallback );
				return RESULT_OK;
			}
			
			if ( retVal == CORE_ERROR::ERR_OFFLINE_READER_NO_MORE_FILES || retVal == CORE_ERROR::ERR_OFFLINE_READER_REACHED_MAX_COUNT )
			{
				framePool->release(std::move(frameRefObj));

				// report to core that we finished the frame generation
				if (coreCallback)
					coreCallback( coreObject, CoreCallbackType::CALLBACK_PROVIDER_FINISHED, std::make_any<int>(0));

				// return "critical" value to callng thread to make it stop
				return CORE_ERROR::ERR_PROVIDER_EXCEPTION;
			}

			// otherwise convert error code to exception
			THROW_EX_ERR(retVal);
		}
		
		// perform an actual acquisition
		retVal = frameProvider->accessData (frameRefObj.get());
		if (retVal != RESULT_OK)
		{
			framePool->release(std::move(frameRefObj));
			FRAMEGENERATE_SCOPED_ERROR << "Image provider failed getting frame. Error : " << retVal;

			// throw exception with error code
			THROW_EX_ERR(retVal);
		}
	}
	catch ( BaseException& bex)
	{
		if (coreCallback)
			coreCallback( coreObject, CoreCallbackType::CALLBACK_PROVIDER_EXCEPTION, std::make_any<std::exception_ptr>(std::current_exception()) );

		retVal = CORE_ERROR::ERR_PROVIDER_EXCEPTION;
	}
	catch ( std::runtime_error& re)
	{
		if (coreCallback)
			coreCallback( coreObject, CoreCallbackType::CALLBACK_PROVIDER_EXCEPTION, std::make_any<BaseException>(BaseException(re, CORE_ERROR::ERR_ALGORITHM_EXCEPTION, __FILE__, __LINE__)) );

		retVal = CORE_ERROR::ERR_PROVIDER_EXCEPTION;
	}
	catch ( std::exception& ex)
	{
		if (coreCallback)
			coreCallback( coreObject, CoreCallbackType::CALLBACK_PROVIDER_EXCEPTION, std::make_any<BaseException>(BaseException(ex, CORE_ERROR::ERR_ALGORITHM_EXCEPTION, __FILE__, __LINE__)) );
		retVal = CORE_ERROR::ERR_PROVIDER_EXCEPTION;
	}
	catch (...)
	{
		if (coreCallback)
			coreCallback( coreObject, CoreCallbackType::CALLBACK_PROVIDER_EXCEPTION, std::make_any<BaseException>(BaseException( CORE_ERROR(CORE_ERROR::ERR_ALGORITHM_EXCEPTION, "Unknown exception"), __FILE__, __LINE__)) );
		retVal = CORE_ERROR::ERR_PROVIDER_EXCEPTION;
	}

	const auto& tFinish = Utility::now_in_microseconds();
	if ( retVal == RESULT_OK)
	{
	
		// call viewer function to pass image data for viewing
		coreCallback( coreObject
					, CoreCallbackType::CALLBACK_PROVIDER_FRAME_IMAGE_DATA
					, std::make_any<std::shared_ptr<Core::SharedFrameData>>(
										std::make_shared<Core::SharedFrameData>(
													frameRefObj.get(), frameProvider->getFrameLifeSpan()
												)
						)
		);

		// push loaded frame reference object to queue
		framePool->pushNextLoaded(std::move(frameRefObj));


		// report core about acquired frame
		// TODO : report about successfullt acquired frame

		RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_acquiredFramesOk, (tFinish - tStart) * 1.0e-6);
	}
	else
	{
		if (frameRefObj)
		{
			// release frame to pool
			framePool->release(std::move(frameRefObj));
			RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_acquiredFramesFailures, (tFinish - tStart) * 1.0e-6);
		}
	}
	return retVal;
}

void Functions::frameGeneratorCleanup	(FrameProviderPtr frameProvider, Core::ICore * coreObject, CoreEventCallback coreEventCallback)
{
	if ( !frameProvider )
		return;
	
	try
	{
		frameProvider->cleanup();
	}
	catch ( BaseException& bex)
	{
		if (coreEventCallback)
			coreEventCallback( coreObject, CoreCallbackType::CALLBACK_PROVIDER_EXCEPTION, std::make_any<std::exception_ptr>(std::current_exception()) );
	}
	catch ( std::runtime_error& re)
	{
		if (coreEventCallback)
			coreEventCallback( coreObject, CoreCallbackType::CALLBACK_PROVIDER_EXCEPTION, std::make_any<BaseException>(BaseException(re, CORE_ERROR::ERR_PROVIDER_EXCEPTION, __FILE__, __LINE__)) );
	}
	catch ( std::exception& ex)
	{
		if (coreEventCallback)
			coreEventCallback( coreObject, CoreCallbackType::CALLBACK_PROVIDER_EXCEPTION, std::make_any<BaseException>(BaseException(ex, CORE_ERROR::ERR_PROVIDER_EXCEPTION, __FILE__, __LINE__)) );
	}
	catch (...)
	{
		if (coreEventCallback)
			coreEventCallback( coreObject, CoreCallbackType::CALLBACK_PROVIDER_EXCEPTION, std::make_any<BaseException>(BaseException( CORE_ERROR(CORE_ERROR::ERR_PROVIDER_EXCEPTION, "Unknown exception"), __FILE__, __LINE__)) );
	}
}