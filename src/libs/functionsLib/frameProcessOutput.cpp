#include "functions.h"
#include <thread>
#include "util.h"
#include "RealTimeStats.h"

using namespace LandaJune;
using namespace Helpers;

void Functions::frameProcessOutput()
{
	const auto t0 = Helpers::Utility::now_in_microseconds();

	// TODO: process algo results here
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	const auto fres = true;

	const auto t1 = Utility::now_in_microseconds();

	if (fres)
	{
		RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_performedAlgoResultOk, (Utility::now_in_microseconds() - t0) * 1.0e-6);
	}
	else
	{
		RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_performedAlgoResultOk, (Utility::now_in_microseconds() - t0) * 1.0e-6);
	}
}