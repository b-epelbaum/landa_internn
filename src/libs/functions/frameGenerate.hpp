#pragma once
#include "FrameRefPool.h"
#include "applog.h"
#include "jutils.h"
#include "RealTimeStats.h"
#include "IFrameProvider.h"

#define FRAMEGENERATE_SCOPED_LOG PRINT_INFO3 << "[frameGenerate func] : "
#define FRAMEGENERATE_SCOPED_ERROR PRINT_ERROR << "[frameGenerate func] : "
#define FRAMEGENERATE_SCOPED_WARNING PRINT_WARNING << "[frameGenerate func] : "

/**
 * frameGenerate free static function
 * This function is a main entry point of one of two "main" processing threads :
*	- frame generating thread
*	- frame consuming thread
*	
*	The function rus in thread with a timeout of 1 millisecond sleep ( in case of empty implementation - see thread implementation )
*	and tries to aqcuire one free frameReference object ( slot ) from the frame reference object list.
*	Once it suceeds, it fills the frame object with needed metadata and invokes the current image provider for accessing 
*	bits of a new incoming image.
*	
*	After bits acquisition, the function pushes prepared frame reference object back to the queue.
*	The prepared frame object is waiting for a first free thread for handling.
*	
 * 
 */

using namespace LandaJune::Helpers;
using namespace LandaJune::FrameProviders;

namespace LandaJune
{
	namespace Core
	{
		#define FRAMEGENERATE_SCOPED_LOG PRINT_INFO3 << "[frameGenerate func] : "
		#define FRAMEGENERATE_SCOPED_ERROR PRINT_ERROR << "[frameGenerate func] : "
		#define FRAMEGENERATE_SCOPED_WARNING PRINT_WARNING << "[frameGenerate func] : "

		static void frameGenerate(IFrameProvider::FrameProviderPtr frameProvider)
		{
			// get a first free 
			auto framePool = FrameRefPool::frameRefPool();
			auto frameRef = framePool->pullFirstFree();

			if (!frameRef) // no free frame - all threads are busy handling another frames
			{
				static auto _droppedFrameCount = 0;
				FRAMEGENERATE_SCOPED_WARNING << "No free FrameRef object in pool ! [ FRAME # " << frameProvider->getCurrentFrameIndex() << " DROPPED] Total dropped : " << ++_droppedFrameCount;
				RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_acquiredFramesFail, 1.0e-6);
				static const std::chrono::milliseconds timeout(frameProvider->getFrameDropDelayTimeout());
				std::this_thread::sleep_for(timeout);
				return;
			}

			FRAME_PROVIDER_ERROR retVal;
			std::string msg;
			const auto& tStart = Utility::now_in_microseconds();
			try
			{
				// ask producer to perform preproces task, if any
				//if (retVal = IAbstractImageProvider::currentProvider()->dataPreProcess(frameRef.get()); retVal != FRAME_PROVIDER_ERROR::ERR_NO_ERROR )
				retVal = frameProvider->dataPreProcess(frameRef.get());
				if (retVal != FRAME_PROVIDER_ERROR::ERR_NO_ERROR)
				{
					framePool->release(std::move(frameRef));
					if (frameProvider->canContinue(retVal))
					{
						return;
					}
					throw std::runtime_error(msg.c_str());
				}


				// perform an actual acquisition
				retVal = frameProvider->dataAccess(frameRef.get());
				if (retVal != FRAME_PROVIDER_ERROR::ERR_NO_ERROR)
				{
					framePool->release(std::move(frameRef));
					FRAMEGENERATE_SCOPED_ERROR << "Image provider failed getting frame. Error : " << toInt(retVal);
					if (frameProvider->canContinue(retVal))
						return;
					throw std::runtime_error(msg.c_str());
				}

				retVal = frameProvider->dataPostProcess(frameRef.get());
				if (retVal != FRAME_PROVIDER_ERROR::ERR_NO_ERROR)
				{
					framePool->release(std::move(frameRef));
					FRAMEGENERATE_SCOPED_ERROR << "Image provider failed getting post processing frame. Error : " << toInt(retVal);
					if (frameProvider->canContinue(retVal))
						return;

					throw std::runtime_error(msg.c_str());
				}

				// push loaded frame reference object to queue
				framePool->pushNextLoaded(std::move(frameRef));
			}

			catch (ProviderException &fpe)
			{
				auto err = fpe.error();
				retVal = FRAME_PROVIDER_ERROR::ERR_GENERAL_ERROR;
			}

			catch (std::exception &e)
			{
				msg = e.what();
				retVal = FRAME_PROVIDER_ERROR::ERR_GENERAL_ERROR;
			}
			catch (...)
			{
				msg = "Unknown error";
				retVal = FRAME_PROVIDER_ERROR::ERR_GENERAL_ERROR;
			}
			const auto& tFinish = Utility::now_in_microseconds();

			if (retVal == FRAME_PROVIDER_ERROR::ERR_NO_ERROR)
			{
				RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_acquiredFramesOk, (tFinish - tStart) * 1.0e-6);
			}
			else
			{
				RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_generatedImagesFail, (tFinish - tStart) * 1.0e-6);
			}

			if (retVal != FRAME_PROVIDER_ERROR::ERR_NO_ERROR)
			{
				throw std::runtime_error(msg.c_str());
			}
		};
	}
}