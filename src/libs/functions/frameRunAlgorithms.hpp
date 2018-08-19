#pragma once
#include "jutils.h"
#include "frameRef.h"
#include "algorithm_wrappers.h"


using namespace LandaJune::Helpers;
using namespace LandaJune::Algorithms;

namespace LandaJune 
{
	namespace Functions
	{
		#define FRAMEALGO_SCOPED_LOG PRINT_INFO4 << "[frameRunAlgorithms func] : "
		#define FRAMEALGO_SCOPED_ERROR PRINT_ERROR << "[frameRunAlgorithms func] : "
		#define FRAMEALGO_SCOPED_WARNING PRINT_WARNING << "[frameRunAlgorithms func] : "

		static void frameRunAlgorithms(FrameRef *frame)
		{
			// call root analysis function, which performed in calling thread
			const auto& tStart = Utility::now_in_microseconds();
			
			auto retVal = calculateAll(frame);

			const auto& tFinish = Utility::now_in_microseconds();

			const double& perfTime = (static_cast<double>(tFinish) - static_cast<double>(tStart)) / 1000;
			FRAMEALGO_SCOPED_LOG << "finished in " << perfTime << " msec...";

			
			if (retVal._result == ALG_STATUS_SUCCESS )
			{
				RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_performedAlgoOk, perfTime);
			}
			else
			{
				RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_performedAlgoFail, perfTime);
			}

			/*
			try
			{
				//LandaJune::Algo::algoTest(pParams->inputParams(), frame);
				Algo::algoTest(pParams->inputParams(), idx, img);
			}
			catch (std::exception &e)
			{
				msg = e.what();
				result = FRAME_PROVIDER_ERROR::ERR_GENERAL_ERROR;
			}
			catch (...)
			{
				msg = "Unknown error";
				result = FRAME_PROVIDER_ERROR::ERR_GENERAL_ERROR;
			}

			const auto& tFinish = Utility::now_in_microseconds();

			//CORE1_SCOPED_LOG << "[ALGO time] : " << t1 - t0 << " msec...";

			if (result == FRAME_PROVIDER_ERROR::ERR_NO_ERROR)
			{
				RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_performedAlgoOk, (tFinish - tStart) * 1.0e-6);
			}
			else
			{
				RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_performedAlgoFail, (tFinish - tStart) * 1.0e-6);
			}

			// send result to processing
			//std::vector<variant> results{};
			//saveBitmapPool().call(frameProcessIPResultsFunc, std::move(results));
			*/
		}
	}
}
