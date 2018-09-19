#pragma once
#include "include/format.h"
#include <string>
#include "util.h"
#include "functions.h"
#include <ppltasks.h>
#include <filesystem>
#include "writequeue.h"

using namespace concurrency;
namespace fs = std::filesystem;

namespace LandaJune
{
	static std::mutex _createDirmutex;
	static const std::string DEFAULT_OUT_FOLDER = "c:\\temp\\june_out";

	enum SAVED_IMAGE_TYPE
	{
		DUMP_WHOLE
		, DUMP_STRIP
		, DUMP_EDGE_OVERLAY
		, DUMP_I2S
		, DUMP_I2S_OVERLAY
		, DUMP_C2C
		, DUMP_C2C_OVERLAY
		, DUMP_WAVE
		, DUMP_WAVE_OVERLAY
		, DUMP_LAST
	};

	static std::string SAVE_IMAGE_TYPE_NAME [] =
	{
		  "FULL_FRAME"
		, "EDGE_OVERLAY"
		, "I2S"
		, "I2S_OVERLAY"
		, "C2C"
		, "C2C_OVERLAY"
		, "WAVE"
		, "WAVE_OVERLAY"
		, ""

	};


	static std::string generateImageSavePath
	( 
			  const std::string& rootFolder // c:\\temp\\out
			, const std::string& imageFolder // 11_Reg_Left or Frame_<FrameID>_<ImageIndex>_algo_name
			, int JobID
			, int frameID
			, int imageIdx
			, SAVED_IMAGE_TYPE imageType
			, const std::string& customPreffix = ""
			, const std::string& customExtension = ".bmp"
	)
	{
		std::string filePath = rootFolder;
		if (rootFolder.empty())
		{
			filePath = DEFAULT_OUT_FOLDER;
		}

		filePath =  fmt::format("{0}\\{1}\\{2}\\{3}.bmp"
						, filePath
						, JobID
						, imageFolder
						, SAVE_IMAGE_TYPE_NAME[imageType]
					);

		return filePath;
	}

	static void createDirectoryIfNeeded (const std::string& pathName)
	{
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
	}

	static void dumpMatFile (std::shared_ptr<cv::Mat> img, const std::string& filePath, bool asyncWrite)
	{
		if (asyncWrite)	
		{
			task<void> t([img, filePath]()
			{
				createDirectoryIfNeeded(filePath);
				const auto data = std::make_shared<std::vector<unsigned char>>();
				if ( cv::imencode(".bmp",*img.get(), *data ) )
				{
					Core::dumpThreadPostJob(data, filePath);
				}
				else
				{
					PRINT_ERROR << "[dumpMatFile] : cannot encode data to BMP";
					// TODO : trow exception
				}
			});
		}
		else
		{
			const auto data = std::make_shared<std::vector<unsigned char>>();

			if ( cv::imencode(".bmp",*img.get(), *data ) )
			{
				auto params = std::make_tuple(data, filePath);
				Functions::frameSaveImage(params);
			}
			else
			{
				PRINT_ERROR << "[dumpMatFile] : cannot encode data to BMP";
				// TODO : trow exception
			}

		}
	}
}
