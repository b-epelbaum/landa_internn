#include "functions.h"
#include "util.h"
#include "ProcessParameters.h"
#include "applog.h"
#include "RealTimeStats.h"
#include "frameRef.h"
#include "interfaces/IAlgorithmRunner.h"

using namespace LandaJune;
using namespace Helpers;
using namespace Algorithms;
using namespace Core;


#define FRAMEALGO_SCOPED_LOG PRINT_INFO4 << "[frameRunAlgorithms func] : "
#define FRAMEALGO_SCOPED_ERROR PRINT_ERROR << "[frameRunAlgorithms func] : "
#define FRAMEALGO_SCOPED_WARNING PRINT_WARNING << "[frameRunAlgorithms func] : "

void Functions::frameRunAlgorithms(FrameRef *frame, const AlgorithmRunnerUniquePtr& algorithmRunner)
{
	// call root analysis function, which performed in calling thread
	const auto& tStart = Utility::now_in_microseconds();

	const auto processParameters = std::dynamic_pointer_cast<Parameters::ProcessParameters>(algorithmRunner->getParameters());

	if (processParameters->EnableProcessing())
		algorithmRunner->process(frame);
	else
	{
		FRAMEALGO_SCOPED_LOG << "---- All algorithm processing is disabled. Skipping calculations ----";
	}

	const auto& tFinish = Utility::now_in_microseconds();

	const double& perfTime = (static_cast<double>(tFinish) - static_cast<double>(tStart)) / 1000;
	FRAMEALGO_SCOPED_LOG << "finished in " << perfTime << " msec...";


	RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_framesHandledOk, perfTime);
}
