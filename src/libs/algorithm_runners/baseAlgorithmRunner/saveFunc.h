#pragma once
#include "include/format.h"
#include <string>
#include "util.h"
#include "functions.h"
#include <ppltasks.h>

using namespace concurrency;

namespace LandaJune
{
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

	static void dumpMatFile (std::shared_ptr<cv::Mat> img, const std::string& filePath, bool asyncWrite)
	{
		if (asyncWrite)	
		{
			auto targetImg = new cv::Mat(std::move(img->clone()));
			task<void> t([targetImg, filePath]()
			{
				 Functions::frameSaveImage(targetImg, filePath);
			});
		}
		else
		{
			Functions::frameSaveImage(img, filePath);
		}
	}
}
