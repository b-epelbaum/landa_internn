#include "FrameRefPool.h"
#include "util.h"
#include "interfaces/IAlgorithmRunner.h"
#include "ProcessParameters.h"
#include "RealTimeStats.h"

#include "functions.h"
#include "common/type_usings.h"
#include "common/june_enums.h"
#include "applog.h"

using namespace LandaJune;
using namespace Helpers;
using namespace Algorithms;

using namespace LandaJune;


#define FRAMECONSUME_SCOPED_LOG PRINT_INFO3 << "[frameConsume func] : "
#define FRAMECONSUME_SCOPED_ERROR PRINT_ERROR << "[frameConsume func] : "
#define FRAMECONSUME_SCOPED_WARNING PRINT_WARNING << "[frameConsume func] : "
CORE_ERROR Functions::frameConsume(BaseParametersPtr parameters, AlgorithmRunnerPtr algorithmRunner, Core::ICore * coreObject, CoreEventCallback coreCallback )
{
	// sanity check
	if (!parameters)
		return CORE_ERROR::ERR_FUNC_INVALID_PARAM_PTR;

	if (!algorithmRunner)
		return CORE_ERROR::ERR_FUNC_INVALID_ALGO_PTR;

	if (!coreObject)
		return CORE_ERROR::ERR_FUNC_INVALID_CORE_OBJECT;

	if (!coreCallback)
		return CORE_ERROR::ERR_FUNC_INVALID_CORE_CALLBACK;

	// start sheet analysis by calling a root function in the same thread
	// we can release frame ref object after all calculation
	// internal analysis functions calls will be parallelized inside the root function recursively

	const auto& tStart = Utility::now_in_microseconds();

	// get frame reference object pool
	auto framesPool = Core::FrameRefPool::frameRefPool();
	std::unique_ptr<Core::FrameRef> frameRefObj;
	CORE_ERROR retVal = CORE_ERROR::RESULT_OK;

	try
	{
		// init FrameProvider object

		if ( !algorithmRunner->isInited ())
		{
			algorithmRunner->init(parameters, coreObject, coreCallback );
			return RESULT_OK;
		}
	
		frameRefObj = framesPool->pullFirstLoaded();

		if (!frameRefObj)
		{
			// TODO : think about configurable sleep 
			// TODO : if no frames from provide the thread eats all CPU time
			// TODO : notify core about skipped frame

			RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_framesHandledSkipped, 1);

			if (coreCallback)
				coreCallback( coreObject, CoreCallbackType::CALLBACK_RUNNER_FRAME_SKIPPED, std::make_any<int>(1) );

			static const std::chrono::milliseconds timeout(10);
			std::this_thread::sleep_for(timeout);
			return CORE_ERROR::RESULT_OK;
		}

		const auto processParameters = std::dynamic_pointer_cast<Parameters::ProcessParameters>(parameters);

		if (!processParameters->EnableProcessing())
		{
			static auto loggedState = false;
			if (!loggedState)
			{
				FRAMECONSUME_SCOPED_WARNING << "---- All algorithm processing is disabled. Skipping calculations ----";
			}
			loggedState = true;
			framesPool->release(std::move(frameRefObj));
			const auto& tFinish = Utility::now_in_microseconds();
			const auto& perfTime = (static_cast<double>(tFinish) - static_cast<double>(tStart)) / 1000;
			RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_framesHandledOk, perfTime);
			return CORE_ERROR::RESULT_OK;
		}

		// we should supply a clone of Algorithm Runner for every frame, because it contains frame-related data !!! ?
		// get a result from image processing
		// it can be 
		// SUCCESS
		// FAIL
		// NOT INITED YET

		retVal =  algorithmRunner->clone()->process(frameRefObj.get());

		// if not inited yet
		// init and return;
		if (retVal == CORE_ERROR::ERR_RUNNER_UNINITIALIZED )
		{
			// if init failed, it throws exception
			// which is caught in handler below

			algorithmRunner->init(parameters, coreObject, coreCallback );
			framesPool->release(std::move(frameRefObj));
			return RESULT_OK;
		}
	}
	catch ( BaseException& bex)
	{
		if (coreCallback)
			coreCallback( coreObject, CoreCallbackType::CALLBACK_RUNNER_EXCEPTION, std::make_any<std::exception_ptr>(std::current_exception()) );

		retVal = CORE_ERROR::ERR_ALGORITHM_EXCEPTION;
	}
	catch ( std::runtime_error& re)
	{
		if (coreCallback)
			coreCallback( coreObject, CoreCallbackType::CALLBACK_RUNNER_EXCEPTION, std::make_any<BaseException>(BaseException(re, CORE_ERROR::ERR_ALGORITHM_EXCEPTION, __FILE__, __LINE__)) );

		retVal = CORE_ERROR::ERR_ALGORITHM_EXCEPTION;
	}
	catch ( std::exception& ex)
	{
		if (coreCallback)
			coreCallback( coreObject, CoreCallbackType::CALLBACK_RUNNER_EXCEPTION, std::make_any<BaseException>(BaseException(ex, CORE_ERROR::ERR_ALGORITHM_EXCEPTION, __FILE__, __LINE__)) );
		retVal = CORE_ERROR::ERR_ALGORITHM_EXCEPTION;
	}
	
	catch (...)
	{
		if (coreCallback)
			coreCallback( coreObject, CoreCallbackType::CALLBACK_RUNNER_EXCEPTION, std::make_any<BaseException>(BaseException( CORE_ERROR(CORE_ERROR::ERR_ALGORITHM_EXCEPTION, "Unknown exception"), __FILE__, __LINE__)) );
		retVal = CORE_ERROR::ERR_ALGORITHM_EXCEPTION;
	}

	// get handled frame index
	// it could be -1, in case of bad handling

	auto const frameIndex = frameRefObj ? frameRefObj->getIndex() : -1;

	if (frameRefObj)
		framesPool->release(std::move(frameRefObj));

	// update statistics
	const auto& tFinish = Utility::now_in_microseconds();
	const auto& perfTime = (static_cast<double>(tFinish) - static_cast<double>(tStart)) / 1000;

	if (retVal == RESULT_OK)
	{
		// frame has been handled OK
		// detection has succeeded

		RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_framesHandledOk, perfTime);
		RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_performedAlgoSucess, perfTime);

		if (coreCallback)
		{
			coreCallback( coreObject, CoreCallbackType::CALLBACK_RUNNER_FRAME_HANDLED_OK, std::make_any<int>(frameIndex) );
			coreCallback( coreObject, CoreCallbackType::CALLBACK_RUNNER_DETECTION_OK, std::make_any<int>(frameIndex) );
		}
	}
	else if (retVal == CORE_ERROR::ERR_RUNNER_ANALYSIS_FAILED)
	{
		// frame has been handled OK
		// detection has failed

		RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_framesHandledOk, perfTime);
		RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_performedAlgoFail, perfTime);

		if (coreCallback)
		{
			coreCallback( coreObject, CoreCallbackType::CALLBACK_RUNNER_FRAME_HANDLED_OK, std::make_any<int>(frameIndex) );
			coreCallback( coreObject, CoreCallbackType::CALLBACK_RUNNER_DETECTION_FAILED, std::make_any<int>(frameIndex) );
		}
	}
	else
	{
		// frame handling failed
		RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_framesHandledFailures, perfTime);
	}
	return retVal;
}


void Functions::frameRunnerCleanup	(AlgorithmRunnerPtr algorithmRunner, Core::ICore * coreObject, CoreEventCallback coreCallback)
{
	if ( !algorithmRunner )
		return;
	
	try
	{
		algorithmRunner->cleanup();
	}
	catch ( BaseException& bex)
	{
		if (coreCallback)
			coreCallback( coreObject, CoreCallbackType::CALLBACK_RUNNER_EXCEPTION, std::make_any<std::exception_ptr>(std::current_exception()) );
	}
	catch ( std::runtime_error& re)
	{
		if (coreCallback)
			coreCallback( coreObject, CoreCallbackType::CALLBACK_RUNNER_EXCEPTION, std::make_any<BaseException>(BaseException(re, CORE_ERROR::ERR_ALGORITHM_EXCEPTION, __FILE__, __LINE__)) );
	}
	catch ( std::exception& ex)
	{
		if (coreCallback)
			coreCallback( coreObject, CoreCallbackType::CALLBACK_RUNNER_EXCEPTION, std::make_any<BaseException>(BaseException(ex, CORE_ERROR::ERR_ALGORITHM_EXCEPTION, __FILE__, __LINE__)) );
	}
	catch (...)
	{
		if (coreCallback)
			coreCallback( coreObject, CoreCallbackType::CALLBACK_RUNNER_EXCEPTION, std::make_any<BaseException>(BaseException( CORE_ERROR(CORE_ERROR::ERR_ALGORITHM_EXCEPTION, "Unknown exception"), __FILE__, __LINE__)) );
	}
}
