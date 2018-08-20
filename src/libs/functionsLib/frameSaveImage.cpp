#include "functions.h"
#include <string>
#include "util.h"
#include "RealTimeStats.h"
#include <opencv2/imgcodecs.hpp>

using namespace LandaJune;
using namespace Helpers;

void Functions::frameSaveImage(const cv::Mat& image, const std::string& pathName)
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

}