#pragma once
#include "include/format.h"
#include <string>
#include "util.h"
#include "functions.h"
#include <ppltasks.h>
#include <filesystem>
#include "writequeue.h"
#include <fstream>
#include "common/june_errors.h"

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
				PRINT_ERROR << "[createDirectoryIfNeeded] : Cannot create folder " << pathName.c_str() << "; exception caught : " << er.what();
				std::throw_with_nested(BaseException(CORE_ERROR{CORE_ERROR::ERR_CORE_CANNOT_CREATE_FOLDER, "Cannot create folder " + pathName }, __FILE__, __LINE__));
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
				try
				{
					const auto data = std::make_shared<std::vector<unsigned char>>();
					if ( imencode(".bmp",*img.get(), *data ) )
					{
						Core::dumpThreadPostJob(data, filePath);
					}
					else
					{
						throw std::runtime_error ("[dumpMatFile] : cannot encode data to BMP");
					}
				}
				catch ( const std::exception& ex)
				{
					PRINT_ERROR << "[dumpMatFile] : exception caught : " << ex.what();
					std::throw_with_nested(BaseException(CORE_ERROR{CORE_ERROR::ERR_CORE_CANNOT_ENCODE_TO_BMP, "" }, __FILE__, __LINE__));
				}

			});
		}
		else
		{
			createDirectoryIfNeeded(filePath);
			const auto data = std::make_shared<std::vector<unsigned char>>();
			if ( imencode(".bmp",*img.get(), *data ) )
			{
				auto params = std::make_tuple(data, filePath);
				Functions::frameSaveData(params);
			}
			else
			{
				PRINT_ERROR << "[dumpMatFile] : cannot encode data to BMP";
			}
		}
	}


	static std::string getBatchRootFolder(std::shared_ptr<Parameters::ProcessParameters> processParameters)
	{
		return std::to_string(processParameters->JobID());
	}

	static std::string generateFullPathForRegCSV(std::shared_ptr<PARAMS_C2C_STRIP_OUTPUT> out, const std::string csvFolder, const int frameIndex )
	{
		return fmt::format("{0}\\Registration_{1}_{2}.csv", csvFolder, SIDE_NAMES[out->_input->_side], frameIndex );
	}

	static std::string generateFullPathForPlacementCSV(const SHEET_SIDE side, const std::string& csvFolder)
	{
		return fmt::format("{0}\\ImagePlacement_{1}.csv", csvFolder, SIDE_NAMES[side]);
	}

	static std::string generateFullPathForWaveCSV(const std::string csvFolder, const int frameIndex )
	{
		return fmt::format("{0}\\WaveY_{1}.csv", csvFolder, frameIndex );
	}


	static std::string getElementPrefix(const int frameIndex, const int imageIndex )
	{
		//file name for ROIs : <Frame_ID>_<ImageIndex>_C2C_LEFT_00_[x,y].bmp
		return (
			fmt::format(
				"{0}_{1}"
				, frameIndex
				, imageIndex)
			);
	}

	static std::string generateFullPathForElement(const std::string& elementName
										, const std::string& ext
										, std::shared_ptr<Parameters::ProcessParameters> processParameters
										, const int frameIndex
										, const int imageIndex
										, const std::string frameFolderName  )
	{
		// target folder <root_folder>\0\11_Reg_Left\\<Frame_ID>_<ImageIndex>_EDGE_LEFT or
		// target folder <root_folder>\0\11_Reg_Left\\<Frame_ID>_<ImageIndex>_C2C_LEFT_00_[x,y].bmp

		return (
			fmt::format(
				R"({0}\{1}\{2}\{3}_{4}.{5})"
				, processParameters->RootOutputFolder().toStdString()
				, getBatchRootFolder(processParameters)
				, frameFolderName
				, getElementPrefix(frameIndex, imageIndex)
				, elementName
				, ext)
			);
	}

	template<typename T>
	static std::string generateFullPathForElement(T& inout
									, const std::string& ext
									, std::shared_ptr<Parameters::ProcessParameters> processParameters
									, const int frameIndex
									, const int imageIndex
									, const std::string frameFolderName )
	{
		return generateFullPathForElement(inout->getElementName(), ext, processParameters, frameIndex, imageIndex, frameFolderName);
	}

	static std::string createCSVFolder(std::shared_ptr<Parameters::ProcessParameters> processParameters)
	{
		auto rootPath = processParameters->RootOutputFolder().toStdString();
		if (rootPath.empty())
		{
			rootPath = DEFAULT_OUT_FOLDER;
		}

		const fs::path p{ 
			fmt::format("{0}\\{1}\\RawResults"
			, rootPath
			, getBatchRootFolder(processParameters) )
		};

		std::string retVal = p.generic_u8string();
		
		if (!is_directory(p) || !exists(p))
		{
			create_directories(p); // create CSV folder
		}

		return retVal;
	}


	static void dumpWaveCSV(const concurrent_vector<std::shared_ptr<PARAMS_WAVE_OUTPUT>> & waveOutputs
				, const int jobID
				, const int frameIndex
				, const int imageIndex
				, const std::string csvFolder  )
	{

		auto overallResult = 
		std::all_of(waveOutputs.begin(), waveOutputs.end(), [](auto& colorWave) { return colorWave->_result == ALG_STATUS_SUCCESS; } )
		? ALG_STATUS_SUCCESS
		: ALG_STATUS_FAILED;

		std::string resultName = (overallResult == ALG_STATUS_SUCCESS ) ? "Success" : "Fail";

		std::ostringstream ss;
		ss << "Pattern Type :,WaveY" << std::endl << std::endl
		<< "Job Id :," <<jobID << std::endl
		<< "Flat ID :," << frameIndex << std::endl
		<< "ImageIndex ID :," << imageIndex << std::endl
		<< "Mark Index In Layout Id :,2" << std::endl
		<< "Wave Y Overall Status :," << resultName << std::endl
		<< "DotIndex\\Separation,";

		const auto colorCount = waveOutputs.size();
		auto counter = 0;
		for (const auto& out : waveOutputs)
		{
			resultName =  (out->_result == ALG_STATUS_SUCCESS ) ? "Success" : "Fail";
			ss << out->_input->_circleColor._colorName << "(" << resultName << ")";
			if ( counter < static_cast<int>(colorCount) -1 )
				ss << ",";
			counter++;
		}
		ss << std::endl;

		const static std::string nan = "NaN";

		int i, j;
		const auto printVals = [&ss, &i, &j, &waveOutputs] ()
		{
			if ( const auto& val =  waveOutputs[j]->_colorCenters[i]._y; val != -1 )
					ss <<  val;
				else
					ss <<  nan;
		};

		for ( i = 0; i < waveOutputs[0]->_colorCenters.size(); i++)
		{
			ss << i << ",";
			for ( j = 0; j < colorCount-1; j++)
			{
				printVals();
				ss  << ",";
			}
			j = colorCount-1;
			printVals();
			ss << std::endl;
		}

		auto const& fPath = generateFullPathForWaveCSV(csvFolder,frameIndex );
		const auto& outString = ss.str();
		const auto& charData = outString.c_str();

		auto dataVector = std::make_shared<std::vector<unsigned char>>();
		dataVector->assign(charData, charData + outString.size() + 1 );
		Core::dumpThreadPostJob(dataVector, fPath);
	}

	static void dumpRegistrationCSV(std::shared_ptr<PARAMS_C2C_STRIP_OUTPUT> stripOut
				, const int jobID
				, const int frameIndex
				, const int imageIndex
				, const std::string csvFolder  )
	{
		if ( stripOut->_c2cROIOutputs.empty() )
		{
			//BASE_RUNNER_SCOPED_WARNING << "C2C array is empty, aborting CSV creation...";
			return;
		}

		std::string resultName = (stripOut->_result == ALG_STATUS_SUCCESS ) ? "Success" : "Fail";
	
		std::ostringstream ss;
		ss << "Pattern Type :,Registration" << std::endl << std::endl
		<< "Job Id :," <<jobID << std::endl
		<< "Flat ID :," << frameIndex << std::endl
		<< "ImageIndex ID :," << imageIndex << std::endl
		<< "Registration Side :" << SIDE_NAMES[stripOut->_input->_side] << std::endl
		<< "Registration Overall Status :," << resultName << std::endl
		<< "Ink\\Sets,";
	
		for (const auto& out : stripOut->_c2cROIOutputs)
		{
			resultName =  (out->_result == ALG_STATUS_SUCCESS ) ? "Success" : "Fail";
			ss << "Set #" << out->_input->_roiIndex + 1 << " :," << resultName << ",";
		}
		ss << std::endl;

		for ( size_t i = 0; i < stripOut->_c2cROIOutputs[0]->_input->_colors.size(); i++)
		{
			ss << stripOut->_c2cROIOutputs[0]->_input->_colors[i]._colorName;
			for (const auto& out : stripOut->_c2cROIOutputs)
			{
				ss << "," << out->_colorCenters[i]._x << "," << out->_colorCenters[i]._y;
			}
			ss << std::endl;
		}

		auto const& fPath = generateFullPathForRegCSV(stripOut, csvFolder,frameIndex );
		const auto& outString = ss.str();
		const auto& charData = outString.c_str();

		auto dataVector = std::make_shared<std::vector<unsigned char>>();
		dataVector->assign(charData, charData + outString.size() + 1 );
		Core::dumpThreadPostJob(dataVector, fPath);
	}
	
	static void dumpPlacementCSV(std::shared_ptr<PARAMS_C2C_STRIP_OUTPUT> stripOut
							, const int frameIndex
							, const int imageIndex
							, const std::string csvFolder)
	{
		const auto& i2sOut = stripOut->_i2sOutput;
		static const std::string colons = ",,,,,,,";

		const auto& csvOutFilePath = QString::fromStdString(generateFullPathForPlacementCSV(stripOut->_input->_side, csvFolder));
		
		const auto fileExists = QFileInfo(csvOutFilePath).exists();

		const QFile::OpenMode flags = (fileExists) ? QFile::Append : QFile::WriteOnly;
		
		QFile outFile(csvOutFilePath);
		outFile.open(flags | QFile::Text);

		const std::string resultName =  (i2sOut->_result == ALG_STATUS_SUCCESS ) ? "Success" : "Fail";
		std::ostringstream ss;

		if (!fileExists)
			ss << "Flat Id,Panel Id,Status,,,,,,,T1->X,T1->Y" << std::endl ;

		// TODO : move pixel density multiplication to algorithm function
		ss << frameIndex 
			<< "," 
			<< imageIndex
			<< "," 
			<< resultName 
			<< colons 
			<< i2sOut->_triangeCorner._x  *  1000 
			<< "," << i2sOut->_triangeCorner._y * 1000 
			<< std::endl;

		const auto& outString = ss.str();
		outFile.write(outString.c_str(), outString.size());
	}
}
