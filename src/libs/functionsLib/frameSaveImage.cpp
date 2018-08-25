#include "functions.h"
#include <string>
#include "util.h"
#include "RealTimeStats.h"
#include <filesystem>

#include <opencv2/imgcodecs.hpp>

using namespace LandaJune;
using namespace Helpers;

namespace fs = std::filesystem;

static std::mutex _createDirmutex;

void Functions::frameSaveImage(const cv::Mat& image, const std::string& pathName)
{
	const auto t0 = Helpers::Utility::now_in_microseconds();
	fs::path p{ pathName };
	auto const parentPath = p.parent_path();
	if (!is_directory(parentPath) || !exists(parentPath))
	{
		std::lock_guard<std::mutex> _lock(_createDirmutex);
		try
		{
			create_directories(parentPath); // create src folder
		}
		catch (fs::filesystem_error& er)
		{

		}
	}
	try
	{
		auto bSaved = cv::imwrite(pathName.c_str(), image);
	}
	catch (...)
	{


	}
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