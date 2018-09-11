#include "baseAlgorithmRunner.h"

////////////////////////////////
// algorithmic functions
#include "algo_edge_impl.h"
#include "algo_i2s_impl.h"
#include "algo_c2c_roi_impl.h"
#include "util.h"
#include <filesystem>
#include <Windows.h>

#include "typeConverters.hpp"
#include <fstream>
#include "algo_wave_impl.h"

namespace fs = std::filesystem;
using namespace concurrency;

using namespace LandaJune;
using namespace Algorithms;
using namespace Parameters;
using namespace Core;

#define BASE_RUNNER_SCOPED_LOG PRINT_INFO6 << "[baseAlgorithmRunner] : "
#define BASE_RUNNER_SCOPED_ERROR PRINT_ERROR << "[baseAlgorithmRunner] : "
#define BASE_RUNNER_SCOPED_WARNING PRINT_WARNING << "[baseAlgorithmRunner] : "

void baseAlgorithmRunner::init(std::shared_ptr<BaseParameters> parameters)
{
	createCSVFolder();
	validateProcessParameters(parameters);
}

////////////////////////////////////////////////////////
/////////////////  core functions

void baseAlgorithmRunner::process(const FrameRef* frame)
{
	// minimal process implementation
	_frame = frame;
	constructFrameContainer(frame, _processParameters->ScanBitDepth());

	_bParallelCalc = _processParameters->ParalellizeCalculations();
	_frameIndex = frame->getIndex();
	_imageIndex = _frameIndex % _processParameters->PanelCount();
	if ( _imageIndex == 0 && _frameIndex != 0 )
		_imageIndex = _processParameters->PanelCount();

	getSourceFrameIndexString();
}

void baseAlgorithmRunner::validateProcessParameters(std::shared_ptr<BaseParameters> parameters)
{
	_processParameters = std::dynamic_pointer_cast<ProcessParameters>(parameters);
}


void baseAlgorithmRunner::constructFrameContainer(const FrameRef* frame, int bitsPerPixel)
{
	_frameContainer = std::make_unique<cv::Mat>(frame->getHeight(), frame->getWidth(),
		CV_MAKETYPE(CV_8U, bitsPerPixel / 8), (void*)frame->getBits());
}

////////////////////////////////////////////////////////
/////////////////  fill data structures functions

void baseAlgorithmRunner::fillCommonProcessParameters(ABSTRACT_INPUT& input)
{
	input.setGenerateOverlay(_processParameters->GenerateOverlays());
	input.setPixel2MM_X(_processParameters->Pixel2MM_X());
	input.setPixel2MM_Y(_processParameters->Pixel2MM_Y());
}


void baseAlgorithmRunner::fillSheetProcessParameters(PARAMS_C2C_SHEET_INPUT& input)
{
	// sheet
	fillCommonProcessParameters(static_cast<ABSTRACT_INPUT&>(input));
	
	// strips 
	if (_processParameters->ProcessLeftStrip())
		fillStripProcessParameters(input._stripInputParamLeft, LEFT);
	if (_processParameters->ProcessRightStrip())
		fillStripProcessParameters(input._stripInputParamRight, RIGHT);

	if (_processParameters->ProcessWave())
		fillWaveProcessParameters(input._waveInputs);
}

void baseAlgorithmRunner::fillStripProcessParameters(PARAMS_C2C_STRIP_INPUT& input, SHEET_SIDE side)
{
	// fill base class parameters
	fillCommonProcessParameters(static_cast<ABSTRACT_INPUT&>(input));
	input._side = side;

	// fill edge parameters
	if (    side == LEFT && _processParameters->ProcessLeftEdge() 
		 || side == RIGHT && _processParameters->ProcessRightEdge() 
		)
		fillEdgeProcessParameters(input._paperEdgeInput, side);

	// fill I2S parameters
	if (    side == LEFT && _processParameters->ProcessLeftI2S() 
		 || side == RIGHT && _processParameters->ProcessRightI2S() 
		)
	fillI2SProcessParameters( input._i2sInput, side);

	// get rectangles of correspondent C2C ROIs
	if (    side == LEFT && _processParameters->ProcessLeftC2C() 
		 || side == RIGHT && _processParameters->ProcessRightC2C() 
		)

	{
		const auto& ROIArray = (input._side == LEFT)
			? _processParameters->C2CROIArrayLeft()
			: _processParameters->C2CROIArrayRight();

		// create array of C2C ROIs
		if (_processParameters->C2CROISetsCount() != 0)
		{
			std::vector<HSV> hsv;
			for (auto i = 0; i < _processParameters->ColorArray().size(); i++)
			{
				hsv.emplace_back(color2HSV(_processParameters->ColorArray()[i]));
			}

			// crate array of PARAMS_C2C_ROI_INPUT objects accordingly to C2C ROIs number
			for (auto i = 0; i < _processParameters->C2CROISetsCount(); i++)
			{
				input._c2cROIInputs.emplace_back(
					PARAMS_C2C_ROI_INPUT
					(
						_frame
						, input._side
						, hsv
						, toROIRect(ROIArray[i])
						, i
					)
				);
			}

			for (auto i = 0; i < _processParameters->C2CROISetsCount(); i++)
			{
				fillC2CProcessParameters(input._c2cROIInputs[i], side );
			}
		}
	}
}

void baseAlgorithmRunner::fillEdgeProcessParameters(PARAMS_PAPEREDGE_INPUT& input, SHEET_SIDE side)
{
	fillCommonProcessParameters(static_cast<ABSTRACT_INPUT&>(input));
	input._approxDistanceFromEdgeX = _processParameters->EdgeApproximateDistanceX_px();
	input._triangeApproximateY = _processParameters->EdgeTriangleApproximateY_px();
	input._side = side;
}

void baseAlgorithmRunner::fillI2SProcessParameters(PARAMS_I2S_INPUT& input, SHEET_SIDE side)
{
	fillCommonProcessParameters(static_cast<ABSTRACT_INPUT&>(input));
	input._side = side;
}

void baseAlgorithmRunner::fillC2CProcessParameters(PARAMS_C2C_ROI_INPUT& input, SHEET_SIDE side)
{
	fillCommonProcessParameters(static_cast<ABSTRACT_INPUT&>(input));
	input._side = side;
}

void baseAlgorithmRunner::fillWaveProcessParameters(std::vector<PARAMS_WAVE_INPUT>& inputs)
{
	for (const auto& color : _processParameters->ColorArray() )
	{
		PARAMS_WAVE_INPUT input(_frame);
		fillCommonProcessParameters(static_cast<ABSTRACT_INPUT&>(input));
		input._circleColor = color2HSV(color);
		input._waveROI = toROIRect(_processParameters->WaveROI());
		input._circlesCount = _processParameters->NumberOfColorDotsPerLine();
		inputs.emplace_back(std::move(input));
	}
}


////////////////////////////////////////////////////////
////////////   Region generation

void baseAlgorithmRunner::generateSheetRegions(PARAMS_C2C_SHEET_INPUT& input, IMAGE_REGION_LIST& regionList) const
{
	if ( _processParameters->ProcessLeftStrip())
		generateStripRegions(input._stripInputParamLeft, regionList);
	if ( _processParameters->ProcessRightStrip())
		generateStripRegions(input._stripInputParamRight, regionList);

	// Wave regions
	if (  _processParameters->ProcessWave() )
	{
		auto waveCount = 0;
		for (auto& waveInput : input._waveInputs)
		{
			generateWaveRegion(waveInput, regionList, waveCount == 0);
			++waveCount;
		}
	}
}

void baseAlgorithmRunner::generateStripRegions(PARAMS_C2C_STRIP_INPUT& input, IMAGE_REGION_LIST& regionList) const
{
	const auto saveSourceStrip = (input._side == LEFT) ? 
		_processParameters->SaveSourceLeftStrip() 
	  : _processParameters->SaveSourceRightStrip();

	const auto& stripRect = (input._side == LEFT) 
		? _processParameters->LeftStripRect() 
		: _processParameters->RightStripRect();

	const auto& approxRect = (input._side == LEFT)
		? toROIRect(_processParameters->I2SApproximateTriangleRectLeft())
		: toROIRect(_processParameters->I2SApproximateTriangleRectRight());

	input._i2sInput._approxTriangeROI = approxRect;
	auto& inputI2S = input._i2sInput;

	// add strip region
	regionList.push_back(
		std::move(ImageRegion::createRegion
		(
			*_frameContainer
			, input._paperEdgeInput._stripImageSource
			, _processParameters
			, qrect2cvrect(stripRect)
			, _frameIndex
			, generateFullPathForElement<PARAMS_C2C_STRIP_INPUT>(input, "bmp")
			, saveSourceStrip
		))
	);

	// get I2S region
	if ( input._side == LEFT && _processParameters->ProcessLeftI2S() 
		 || input._side == RIGHT && _processParameters->ProcessRightI2S() 
		)
	generateI2SRegion(inputI2S, regionList);

	///////////////////////
	////////// C2C ROIs
	if ( input._side == LEFT && _processParameters->ProcessLeftC2C() 
		 || input._side == RIGHT && _processParameters->ProcessRightC2C() 
		)
	{
		for (auto& _c2cROIInput : input._c2cROIInputs)
		{
			generateC2CRegion(_c2cROIInput, regionList);
		}
	}
}

void baseAlgorithmRunner::generateI2SRegion(PARAMS_I2S_INPUT& input, IMAGE_REGION_LIST& regionList) const
{
	const auto saveSourceI2S = (input._side == LEFT) ? 
		_processParameters->SaveSourceLeftI2S() 
	  : _processParameters->SaveSourceRightI2S();

	regionList.push_back(std::move(ImageRegion::createRegion(*_frameContainer
			, input._triangleImageSource
			, _processParameters
			, roirect2cvrect(input._approxTriangeROI)
			, _frameIndex
			, this->generateFullPathForElement<PARAMS_I2S_INPUT>(input, "bmp")
			, saveSourceI2S)));

}

void baseAlgorithmRunner::generateC2CRegion(PARAMS_C2C_ROI_INPUT& input, IMAGE_REGION_LIST& regionList) const
{
	const auto saveSourceC2C = (input._side == LEFT) ? 
		_processParameters->SaveSourceLeftC2C() 
	  : _processParameters->SaveSourceRightC2C();

	auto& ROI = input;
	ROI.setGenerateOverlay(input.GenerateOverlay());
	
	regionList.push_back(
		std::move(ImageRegion::createRegion
		(
			*_frameContainer
			, ROI._ROIImageSource
			, _processParameters
			, roirect2cvrect(ROI._ROI)
			, _frameIndex
			, generateFullPathForElement<PARAMS_C2C_ROI_INPUT>(input, "bmp")
			, saveSourceC2C
		))
	);
}

void baseAlgorithmRunner::generateWaveRegion(PARAMS_WAVE_INPUT& input, IMAGE_REGION_LIST& regionList, bool bDumpWave ) const
{
	auto& ROI = input;
	ROI.setGenerateOverlay(input.GenerateOverlay());

	regionList.push_back(
		std::move(ImageRegion::createRegion
		(
			*_frameContainer
			, ROI._waveImageSource
			, _processParameters
			, roirect2cvrect(ROI._waveROI)
			, _frameIndex
			, generateFullPathForElement<PARAMS_WAVE_INPUT>(input, "bmp")
			, bDumpWave && _processParameters->SaveSourceWave()
		))
	);
}


////////////////////////////////////////////////////////
/////////////////  copy regions functions

void baseAlgorithmRunner::copyRegions(IMAGE_REGION_LIST& regionList)
{
	if (_bParallelCalc)
	{
		parallel_for_each (regionList.begin(), regionList.end(), [&](auto &in) 
		{
			ImageRegion::performCopy(std::ref(in));
		});
	}
	else
	{
		std::for_each(regionList.begin(), regionList.end(), ImageRegion::performCopy );
	}
}


/////////////////////////////////////////////////////////////////
/////////////////  Algorithm processing functions per object

// ------------------------------------------------------
//				WHOLE SHEET  FUNCTION 

PARAMS_C2C_SHEET_OUTPUT baseAlgorithmRunner::processSheet(const PARAMS_C2C_SHEET_INPUT& sheetInput)
{
	PARAMS_C2C_SHEET_OUTPUT retVal;
	retVal._result = ALG_STATUS_SUCCESS;

	const auto leftStripLambda = [&] { retVal._stripOutputParameterLeft = std::move(processStrip(std::cref(sheetInput._stripInputParamLeft ))); };
	const auto rightStripLambda = [&] { retVal._stripOutputParameterRight = std::move(processStrip(std::cref(sheetInput._stripInputParamRight))); };
	const auto waveLambda = [&] { retVal._waveOutputs = std::move(processWaves(std::cref(sheetInput._waveInputs))); };
	
	std::vector<std::function<void()>> processLambdas;
	if ( _processParameters->ProcessLeftStrip())
		processLambdas.emplace_back(leftStripLambda);
	if ( _processParameters->ProcessRightStrip())
		processLambdas.emplace_back(rightStripLambda);
	if ( _processParameters->ProcessWave())
		processLambdas.emplace_back(waveLambda);


	if (_bParallelCalc)
	{
 		parallel_for_each(processLambdas.begin(), processLambdas.end(), [&](auto &func) 
		{
			func();
		});
	}
	else
	{
		std::for_each(processLambdas.begin(), processLambdas.end(), [&](auto &func) 
		{
			func();
		});
	}

	retVal._input = std::move(sheetInput);
	return std::move(retVal);
}

// ------------------------------------------------------
//				Strip processing  FUNCTION 

PARAMS_C2C_STRIP_OUTPUT baseAlgorithmRunner::processStrip(const PARAMS_C2C_STRIP_INPUT& stripInput)
{
	PARAMS_C2C_STRIP_OUTPUT retVal;

	const auto edgeLambda = [&] { retVal._paperEdgeOutput = processEdge(std::cref(stripInput._paperEdgeInput)); };
	const auto i2sLambda = [&] { retVal._i2sOutput = processI2S(std::cref(stripInput._i2sInput)); };

	auto& roiInputs = stripInput._c2cROIInputs;
	if (roiInputs.empty())
	{
		BASE_RUNNER_SCOPED_WARNING << "No C2C ROI defined in input parameters.";
	}

	auto calculateC2CfuncPar = 
		[&]{
			parallel_for_each (std::begin(roiInputs), std::end(roiInputs), [&](auto &in) 
			{
				retVal._c2cROIOutputs.push_back(processC2CROI(std::ref(in)));
			});
		};

	auto calculateC2CfuncSec = 
		[&]{
			std::for_each (std::begin(roiInputs), std::end(roiInputs), [&](auto &in) 
			{
				retVal._c2cROIOutputs.push_back(processC2CROI(in));
			});
		};

	std::vector<std::function<void()>> processLambdas;
	if (( _processParameters->ProcessLeftEdge() && stripInput._side == LEFT ||  _processParameters->ProcessRightEdge() && stripInput._side == RIGHT ))
		processLambdas.emplace_back(edgeLambda);
	if (( _processParameters->ProcessLeftI2S() && stripInput._side == LEFT ||  _processParameters->ProcessRightI2S() && stripInput._side == RIGHT ))
		processLambdas.emplace_back(i2sLambda);
	if (( _processParameters->ProcessLeftC2C() && stripInput._side == LEFT ||  _processParameters->ProcessRightC2C() && stripInput._side == RIGHT ))
	{
		if (_bParallelCalc)
			processLambdas.emplace_back(calculateC2CfuncPar);
		else
			processLambdas.emplace_back(calculateC2CfuncSec);
	}

	if (_bParallelCalc)
	{
 		parallel_for_each(processLambdas.begin(), processLambdas.end(), [&](auto &func) 
		{
			func();
		});
	}
	else
	{
		std::for_each(processLambdas.begin(), processLambdas.end(), [&](auto &func) 
		{
			func();
		});
	}

	retVal._result = 
		std::all_of(retVal._c2cROIOutputs.begin(), retVal._c2cROIOutputs.end(), [](auto& out) { return out._result == ALG_STATUS_SUCCESS; } )
		? ALG_STATUS_SUCCESS
		: ALG_STATUS_FAILED;

	// TODO : update C2C rectangles with I2S offset
	retVal._input = std::move(stripInput);
	return std::move(retVal);
}

void baseAlgorithmRunner::initEdge(const INIT_PARAMETER& initParam) const
{
	try
	{
		detect_edge_init(initParam);
	}
	catch (...)
	{
		BASE_RUNNER_SCOPED_ERROR << "Function detect_edge_init has thrown exception";
	}
}

// ------------------------------------------------------
//				Edge processing  FUNCTION 

PARAMS_PAPEREDGE_OUTPUT baseAlgorithmRunner::processEdge(const PARAMS_PAPEREDGE_INPUT& input)
{
	PARAMS_PAPEREDGE_OUTPUT retVal;
	if ( !_processParameters->EnableAlgorithmProcessing() )
	{
		BASE_RUNNER_SCOPED_LOG << "Edge detection [side " << input._side << "] skipped due to process configuration";
		return retVal;
	}

	BASE_RUNNER_SCOPED_LOG << "Edge detection [side " << input._side << "] runs on thread #" << GetCurrentThreadId();

	try
	{
		detect_edge(input, retVal);
	}
	catch (...)
	{
		BASE_RUNNER_SCOPED_ERROR << "Function detect_edge has thrown exception";
		retVal._result = ALG_STATUS_EXCEPTION_THROWN;
	}

	retVal._input = std::move(input);
	dumpOverlay<PARAMS_PAPEREDGE_OUTPUT>(retVal);

	return std::move(retVal);
}

void baseAlgorithmRunner::shutdownEdge() const
{
	try
	{
		detect_edge_shutdown();
	}
	catch (...)
	{
		BASE_RUNNER_SCOPED_ERROR << "Function detect_edge_shutdown has thrown exception";
	}
}

// ------------------------------------------------------
//				I2S processing  FUNCTION 

void baseAlgorithmRunner::initI2S(const INIT_PARAMETER& initParam) const
{
	try
	{
		detect_i2s_init(initParam);
	}
	catch (...)
	{
		BASE_RUNNER_SCOPED_ERROR << "Function detect_i2s_init has thrown exception";
	}
}

PARAMS_I2S_OUTPUT baseAlgorithmRunner::processI2S(const PARAMS_I2S_INPUT& input)
{
	PARAMS_I2S_OUTPUT retVal;

	if ( !_processParameters->EnableAlgorithmProcessing() )
	{
		BASE_RUNNER_SCOPED_LOG << "I2S detection [side " << input._side << "] skipped due to process configuration";
		return retVal;
	}

	BASE_RUNNER_SCOPED_LOG << "I2S detection [side " << input._side << "] runs on thread #" << GetCurrentThreadId();
	try
	{
		detect_i2s(input, retVal);
	}
	catch (...)
	{
		BASE_RUNNER_SCOPED_ERROR << "Function detect_i2s has thrown exception";
		retVal._result = ALG_STATUS_EXCEPTION_THROWN;
	}

	retVal._input = std::move(input);
	dumpOverlay<PARAMS_I2S_OUTPUT>(retVal);
	return std::move(retVal);
}

void baseAlgorithmRunner::shutdownI2S() const
{
	try
	{
		detect_i2s_shutdown();
	}
	catch (...)
	{
		BASE_RUNNER_SCOPED_ERROR << "Function detect_i2s_shutdown has thrown exception";
	}
}



void baseAlgorithmRunner::initC2CRoi(const INIT_PARAMETER& initParam) const
{
	C2C_ROI_INIT_PARAMETER c2cInitParam(initParam);
	const auto& buf = _processParameters->CircleTemplateBuffer().constData();
	const std::vector<char> data(buf, buf + _processParameters->CircleTemplateBuffer().size());
	c2cInitParam._templateImage = std::move(cv::imdecode(cv::Mat(data), CV_LOAD_IMAGE_GRAYSCALE));

	try
	{
		detect_c2c_roi_init(c2cInitParam);
	}
	catch (...)
	{
		BASE_RUNNER_SCOPED_ERROR << "Function detect_c2c_roi_init has thrown exception";
	}
}

// ------------------------------------------------------
//				C2C 1 ROI processing  FUNCTION 

PARAMS_C2C_ROI_OUTPUT baseAlgorithmRunner::processC2CROI(const PARAMS_C2C_ROI_INPUT& input)
{
	PARAMS_C2C_ROI_OUTPUT retVal;
	if ( !_processParameters->EnableAlgorithmProcessing() )
	{
		BASE_RUNNER_SCOPED_LOG << "C2C Detection [side : " << input._side << "; index : " << input._roiIndex << "] skipped due to process configuration";
		return retVal;
	}

	// allocate array of C2C outputs
	const auto& hsvCount = input._colors.size();

	retVal._result = ALG_STATUS_FAILED;
	retVal._colorStatuses = { hsvCount, ALG_STATUS_FAILED };
	retVal._colorCenters = { hsvCount, {0,0} };
	
	BASE_RUNNER_SCOPED_LOG << "C2C Detection [side : " << input._side << "; index : " << input._roiIndex << "] runs in thread #" << GetCurrentThreadId();
	try
	{
		detect_c2c_roi(input, retVal);
	}
	catch (...)
	{
		BASE_RUNNER_SCOPED_ERROR << "Function detect_c2c_roi has thrown exception";
		retVal._result = ALG_STATUS_EXCEPTION_THROWN;
	}

	// move input parameters to output
	retVal._input = std::move(input);

	dumpOverlay<PARAMS_C2C_ROI_OUTPUT>(retVal);
	return std::move(retVal);
}

void baseAlgorithmRunner::shutdownC2CRoi() const
{
	try
	{
		detect_c2c_roi_shutdown();
	}
	catch (...)
	{
		BASE_RUNNER_SCOPED_ERROR << "Function detect_c2c_roi_shutdown has thrown exception";
	}
}

void baseAlgorithmRunner::initWave(const INIT_PARAMETER& initParam)
{
	WAVE_INIT_PARAMETER waveInitParam(initParam);
	const auto& buf = _processParameters->CircleTemplateBuffer().constData();
	const std::vector<char> data(buf, buf + _processParameters->CircleTemplateBuffer().size());
	waveInitParam._templateImage = std::move(cv::imdecode(cv::Mat(data), CV_LOAD_IMAGE_GRAYSCALE));

	try
	{
		detect_wave_init(waveInitParam);
	}
	catch (...)
	{
		BASE_RUNNER_SCOPED_ERROR << "Function detect_wave_init has thrown exception";
	}
}

// ------------------------------------------------------
//				Wave processing  FUNCTION 

PARAMS_WAVE_OUTPUT baseAlgorithmRunner::processWave(const PARAMS_WAVE_INPUT& input)
{
	PARAMS_WAVE_OUTPUT retVal;
	if ( !_processParameters->EnableAlgorithmProcessing() )
	{
		BASE_RUNNER_SCOPED_LOG << "WAVE Detection [color : " << input._circleColor._colorName.c_str() << "] skipped due to process configuration";
		return retVal;
	}

	// allocate array of wave outputs
	const auto& circleCount = input._circlesCount;

	retVal._result = ALG_STATUS_FAILED;
	retVal._colorDetectionResults = { static_cast<const uint64_t>(circleCount), ALG_STATUS_FAILED };
	retVal._colorCenters = { static_cast<const uint64_t>(circleCount), {0,0} };
	
	BASE_RUNNER_SCOPED_LOG << "WAVE Detection [color : " << input._circleColor._colorName.c_str() << "] runs in thread #" << GetCurrentThreadId();
	try
	{
		detect_wave(input, retVal);
	}
	catch (...)
	{
		BASE_RUNNER_SCOPED_ERROR << "Function detect_wave has thrown exception";
		retVal._result = ALG_STATUS_EXCEPTION_THROWN;
	}

	// move input parameters to output
	retVal._input = std::move(input);

	dumpOverlay<PARAMS_WAVE_OUTPUT>(retVal);
	return std::move(retVal);
}

concurrent_vector<PARAMS_WAVE_OUTPUT> baseAlgorithmRunner::processWaves(const std::vector<PARAMS_WAVE_INPUT>& inputs)
{
	concurrent_vector<PARAMS_WAVE_OUTPUT> retVal;
	if (_bParallelCalc)
	{
		parallel_for_each (inputs.begin(), inputs.end(), [&](auto &in) 
		{
			retVal.push_back(std::move(processWave(std::cref(in))));
		});
	}
	else
	{
		std::for_each (inputs.begin(), inputs.end(), [&](auto &in) 
		{
			retVal.push_back(std::move(processWave(in)));
		});
	}

	return retVal;
}

void baseAlgorithmRunner::shutdownWave()
{
	try
	{
		detect_wave_shutdown();
	}
	catch (...)
	{
		BASE_RUNNER_SCOPED_ERROR << "Function detect_wave_shutdown has thrown exception";
	}
}


////////////////////////////////////////////////////////
/////////////////  file saving  functions

void baseAlgorithmRunner::dumpRegistrationCSV(const PARAMS_C2C_STRIP_OUTPUT& stripOut)
{
	if (!_processParameters->SaveC2CRegistrationCSV())
		return;

	if ( stripOut._c2cROIOutputs.empty() )
	{
		BASE_RUNNER_SCOPED_WARNING << "C2C array is empty, aborting CSV creation...";
		return;
	}

	std::string resultName = (stripOut._result == ALG_STATUS_SUCCESS ) ? "Success" : "Fail";

	std::ostringstream ss;
	ss << "Pattern Type :,Registration" << std::endl << std::endl
	<< "Job Id :," << _processParameters->JobID() << std::endl
	<< "Flat ID :," << _frameIndex << std::endl
	<< "ImageIndex ID :," << _imageIndex << std::endl
	<< "Registration Side :" << SIDE_NAMES[stripOut._input->_side] << std::endl
	<< "Registration Overall Status :," << resultName << std::endl
	<< "Ink\\Sets,";
	
	for (const auto& out : stripOut._c2cROIOutputs)
	{
		resultName =  (out._result == ALG_STATUS_SUCCESS ) ? "Success" : "Fail";
		ss << "Set #" << out._input->_roiIndex + 1 << " :," << resultName << ",";
	}
	ss << std::endl;

	for ( size_t i = 0; i < stripOut._c2cROIOutputs[0]._input->_colors.size(); i++)
	{
		ss << stripOut._c2cROIOutputs[0]._input->_colors[i]._colorName;
		for (const auto& out : stripOut._c2cROIOutputs)
		{
			ss << "," << out._colorCenters[i]._x << "," << out._colorCenters[i]._y;
		}
		ss << std::endl;
	}

	auto const& fPath = generateFullPathForRegCSV(stripOut);
	std::ofstream outFile;
	outFile.open (fPath);
	if (outFile.fail())
	{
		BASE_RUNNER_SCOPED_ERROR << "cannot save file " << fPath.c_str() << "; error : " << strerror(errno);
	}
	else
	{
		outFile << ss.str();
		outFile.close();
	}
}

void baseAlgorithmRunner::dumpPlacementCSV(const PARAMS_C2C_STRIP_OUTPUT& stripOut)
{
	if (!_processParameters->SaveI2SPlacementCSV())
		return;

	const auto& i2sOut = stripOut._i2sOutput;
	static const std::string colons = ",,,,,,,";

	const auto& csvOutFilePath = QString::fromStdString(generateFullPathForPlacementCSV(stripOut._input->_side));
	
	const auto fileExists = QFileInfo(csvOutFilePath).exists();

	const QFile::OpenMode flags = (fileExists) ? QFile::Append : QFile::WriteOnly;
	
	QFile outFile(csvOutFilePath);
	outFile.open(flags | QFile::Text);

	const std::string resultName =  (i2sOut._result == ALG_STATUS_SUCCESS ) ? "Success" : "Fail";
	std::ostringstream ss;

	if (!fileExists)
		ss << "Flat Id,Panel Id,Status,,,,,,,T1->X,T1->Y" << std::endl ;

	// TODO : move pixel density multiplication to algorithm function
	ss << _frameIndex 
		<< "," 
		<< _imageIndex
		<< "," 
		<< resultName 
		<< colons 
		<< i2sOut._triangeCorner._x  *  1000 
		<< "," << i2sOut._triangeCorner._y * 1000 
		<< std::endl;

	const std::string& outString = ss.str();
	outFile.write(outString.c_str(), outString.size());
}


void baseAlgorithmRunner::createCSVFolder()
{
	auto rootPath = _processParameters->RootOutputFolder().toStdString();
	if (rootPath.empty())
	{
		rootPath = DEFAULT_OUT_FOLDER;
	}

	const fs::path p{ 
		fmt::format("{0}\\{1}\\RawResults"
		, rootPath
		, getBatchRootFolder() )
	};
	
	if (!is_directory(p) || !exists(p))
	{
		try
		{
			create_directories(p); // create CSV folder
		}
		catch (fs::filesystem_error& er)
		{
			// 
		}
	}
	_csvFolder = p.string();
}

//////////////////////////////////////////////
//////////////  file naming functions 


std::string baseAlgorithmRunner::getBatchRootFolder() const
{
	return std::move(std::to_string(_processParameters->JobID()));
}


std::string baseAlgorithmRunner::getFrameFolderName() const
{
	return fmt::format("frame_#{0}", _frameIndex);
}

std::string baseAlgorithmRunner::getElementPrefix() const
{
	//file name for ROIs : <Frame_ID>_<ImageIndex>_C2C_LEFT_00_[x,y].bmp
	return std::move(
		fmt::format(
			"{0}_{1}_"
			, _frameIndex
			, _imageIndex)
	);
}

std::string baseAlgorithmRunner::generateFullPathForElement(const std::string& elementName, const std::string& ext ) const
{
	// target folder <root_folder>\0\11_Reg_Left\\<Frame_ID>_<ImageIndex>_EDGE_LEFT or
	// target folder <root_folder>\0\11_Reg_Left\\<Frame_ID>_<ImageIndex>_C2C_LEFT_00_[x,y].bmp

	return std::move(
		fmt::format(
			R"({0}\{1}\{2}\{3}_{4}.{5})"
			, _processParameters->RootOutputFolder().toStdString()
			, getBatchRootFolder()
			, getFrameFolderName()
			, getElementPrefix()
			, elementName
			, ext)
	);
}

std::string baseAlgorithmRunner::generateFullPathForRegCSV(const PARAMS_C2C_STRIP_OUTPUT& out) const
{
	return fmt::format("{0}\\Registration_{1}_{2}.csv", _csvFolder, SIDE_NAMES[out._input->_side], _frameIndex );
}

std::string baseAlgorithmRunner::generateFullPathForPlacementCSV(SHEET_SIDE side) const
{
	return fmt::format("{0}\\ImagePlacement_{1}.csv", _csvFolder, SIDE_NAMES[side]);
}

void baseAlgorithmRunner::getSourceFrameIndexString()
{
	std::pair<std::string,bool> retVal;

	// get source frame ID from custom parameter passed by provider
	_sourceFrameIndexStr.clear();
	try
	{
		const auto framePath = std::any_cast<std::string>(_frame->getNamedParameter(NAMED_PROPERTY_SOURCE_PATH));
		_sourceFrameIndexStr = parseSourceFrameIndexString(framePath);
	}
	catch (const std::bad_any_cast& e)
	{
	}

	if ( _sourceFrameIndexStr.empty())
		_sourceFrameIndexStr = std::to_string(_frameIndex);
	else
		_frameIndex = std::stoi(_sourceFrameIndexStr);
}

