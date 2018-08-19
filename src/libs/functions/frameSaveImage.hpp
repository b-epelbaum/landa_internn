#pragma once
#include <string>
#include "jutils.h"
#include "RealTimeStats.h"

using namespace LandaJune::Core;
using namespace LandaJune::Helpers;

namespace LandaJune 
{
	namespace Functions
	{
		static void frameSaveImage(const cv::Mat& image, const std::string& pathName)
		{
			const auto t0 = Helpers::Utility::now_in_microseconds();
			const auto fres = cv::imwrite(pathName.c_str(), image);

			if (fres)
			{
				RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_savedBitmapsOk, (Utility::now_in_microseconds() - t0) * 1.0e-6, image.step[0] * image.rows);
			}
			else
			{
				RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_savedBitmapsFail, (Utility::now_in_microseconds() - t0) * 1.0e-6);
			}

		};
	}
}
