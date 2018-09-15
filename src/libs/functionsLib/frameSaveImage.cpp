#include "functions.h"
#include <string>
#include "util.h"
#include "RealTimeStats.h"
#include <filesystem>
#include <Windows.h>

#include <opencv2/imgcodecs.hpp>
#include "applog.h"

using namespace LandaJune;
using namespace Helpers;

namespace fs = std::filesystem;

static std::mutex _createDirmutex;

void Functions::frameSaveImage(cv::Mat * pimage, const std::string pathName)
{
	//PRINT_INFO7 << "frameSaveImage [file " << pathName.c_str() << "] runs on thread #" << GetCurrentThreadId();

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
		//std::this_thread::sleep_for(std::chrono::milliseconds(100));
		auto bSaved = cv::imwrite(pathName.c_str(), *pimage);
	}
	catch (...)
	{


	}

	RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_savedBitmapsOk, (Utility::now_in_microseconds() - t0) * 1.0e-6, pimage->step[0] * pimage->rows);

	delete pimage;
}

void Functions::frameSaveImage(std::shared_ptr<cv::Mat> image, const std::string pathName)
{
	//PRINT_INFO7 << "frameSaveImage [file " << pathName.c_str() << "] runs on thread #" << GetCurrentThreadId();

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
		//std::this_thread::sleep_for(std::chrono::milliseconds(100));
		auto bSaved = cv::imwrite(pathName.c_str(), *image);
	}
	catch (...)
	{


	}

	RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_savedBitmapsOk, (Utility::now_in_microseconds() - t0) * 1.0e-6, image->step[0] * image->rows);

}