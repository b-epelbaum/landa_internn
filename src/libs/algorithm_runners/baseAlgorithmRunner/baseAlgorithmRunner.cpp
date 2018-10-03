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
#include "algo_wave_impl.h"
#include "RealTimeStats.h"

using namespace concurrency;

using namespace LandaJune;
using namespace Algorithms;
using namespace Parameters;
using namespace Helpers;
using namespace Core;
using namespace Files;

#define BASE_RUNNER_SCOPED_LOG PRINT_INFO6 << "[baseAlgorithmRunner] : "
#define BASE_RUNNER_SCOPED_ERROR PRINT_ERROR << "[baseAlgorithmRunner] : "
#define BASE_RUNNER_SCOPED_WARNING PRINT_WARNING << "[baseAlgorithmRunner] : "


baseAlgorithmRunner::baseAlgorithmRunner(const baseAlgorithmRunner& other)
	: _frame(other._frame)
	, _coreObject(other._coreObject)
	, _callback(other._callback)
	, _processParameters(other._processParameters)
	, _frameIndex (other._frameIndex)
	, _imageIndex (other._imageIndex)
	, _bParallelCalc (other._bParallelCalc)
	, _bAsyncWrite (other._bAsyncWrite)
	, _bOfflineSource (other._bOfflineSource)
	, _sourceFrameIndexStr (other._sourceFrameIndexStr)
	, _sourceFramePath (other._sourceFramePath)
	, _csvOutFolder (other._csvOutFolder)
{
}


////////////////////////////////////////////////////////
/////////////////  core functions

void baseAlgorithmRunner::process(const FrameRef* frame)
{
	setupFrameData(frame);
	processInternal();
}

void baseAlgorithmRunner::setupFrameData(const FrameRef* frame )
{
	_frame = frame;
	
	_bParallelCalc = _processParameters->ParalellizeCalculations();
	_frameIndex =		frame->getIndex();
	_bAsyncWrite =		frame->getAsyncWrite();
	_bOfflineSource =	frame->isOfflineSource();

	_imageIndex = _frameIndex % _processParameters->PanelCount();
	if ( _imageIndex == 0 && _frameIndex != 0 )
		_imageIndex = _processParameters->PanelCount();

	if (_bOfflineSource)
		getSourceFrameIndexString();
}

void baseAlgorithmRunner::validateProcessParameters(BaseParametersPtr parameters)
{
	_processParameters = std::dynamic_pointer_cast<ProcessParameters>(parameters);
}


////////////////////////////////////////////////////////
/////////////////  setup data structures functions

void baseAlgorithmRunner::setupCommonProcessParameters(ABSTRACT_INPUT_PTR input)
{
	input->setGenerateOverlay(_processParameters->GenerateOverlays());
	input->setPixel2MM_X(_processParameters->Pixel2MM_X());
	input->setPixel2MM_Y(_processParameters->Pixel2MM_Y());
}


void baseAlgorithmRunner::setupSheetProcessParameters(PARAMS_C2C_SHEET_INPUT_PTR input)
{
	// sheet
	setupCommonProcessParameters(std::static_pointer_cast<ABSTRACT_INPUT>(input));
	
	// strips 
	if (_processParameters->ProcessLeftStrip())
		setupStripProcessParameters(input->_stripInputParamLeft, LEFT);
	if (_processParameters->ProcessRightStrip())
		setupStripProcessParameters(input->_stripInputParamRight, RIGHT);

	if (_processParameters->ProcessWave())
		setupWaveProcessParameters(input->_waveInputs);
}

void baseAlgorithmRunner::setupStripProcessParameters(PARAMS_C2C_STRIP_INPUT_PTR input, SHEET_SIDE side)
{
	// setup base class parameters
	setupCommonProcessParameters(std::static_pointer_cast<ABSTRACT_INPUT>(input));
	input->_side = side;

	// setup edge parameters
	if (    side == LEFT && _processParameters->ProcessLeftEdge() 
		 || side == RIGHT && _processParameters->ProcessRightEdge() 
		)
		setupEdgeProcessParameters(input->_paperEdgeInput, side);

	// setup I2S parameters
	if (    side == LEFT && _processParameters->ProcessLeftI2S() 
		 || side == RIGHT && _processParameters->ProcessRightI2S() 
		)
	setupI2SProcessParameters( input->_i2sInput, side);

	// get rectangles of correspondent C2C ROIs
	if (    side == LEFT && _processParameters->ProcessLeftC2C() 
		 || side == RIGHT && _processParameters->ProcessRightC2C() 
		)

	{
		const auto& ROIArray = (input->_side == LEFT)
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
				input->_c2cROIInputs.emplace_back(
					std::make_shared<PARAMS_C2C_ROI_INPUT>
					(
						_frame
						, input->_side
						, hsv
						, toROIRect(ROIArray[i])
						, i
					)
				);
			}

			for (auto i = 0; i < _processParameters->C2CROISetsCount(); i++)
			{
				setupC2CProcessParameters(input->_c2cROIInputs[i], side );
			}
		}	
	}
}

void baseAlgorithmRunner::setupEdgeProcessParameters(PARAMS_PAPEREDGE_INPUT_PTR input, const SHEET_SIDE side)
{
	setupCommonProcessParameters(std::static_pointer_cast<ABSTRACT_INPUT>(input));
	input->_approxDistanceFromEdgeX = _processParameters->EdgeApproximateDistanceX_px();
	input->_triangeApproximateY = _processParameters->EdgeTriangleApproximateY_px();
	input->_side = side;
}

void baseAlgorithmRunner::setupI2SProcessParameters(PARAMS_I2S_INPUT_PTR input, SHEET_SIDE side)
{
	setupCommonProcessParameters(std::static_pointer_cast<ABSTRACT_INPUT>(input));
	input->_side = side;
}

void baseAlgorithmRunner::setupC2CProcessParameters(PARAMS_C2C_ROI_INPUT_PTR input, const SHEET_SIDE side)
{
	setupCommonProcessParameters(std::static_pointer_cast<ABSTRACT_INPUT>(input));
	input->_side = side;
}

void baseAlgorithmRunner::setupWaveProcessParameters(std::vector<PARAMS_WAVE_INPUT_PTR>& inputs)
{
	for (const auto& color : _processParameters->ColorArray() )
	{
		auto input = std::make_shared<PARAMS_WAVE_INPUT>(_frame);
		setupCommonProcessParameters(std::static_pointer_cast<ABSTRACT_INPUT>(input));
		input->_circleColor = color2HSV(color);
		input->_waveROI = toROIRect(_processParameters->WaveROI());
		input->_circlesCount = _processParameters->WaveNumberOfColorDotsPerLine();
		inputs.emplace_back(std::move(input));
	}
}


////////////////////////////////////////////////////////
////////////   Region generation

void baseAlgorithmRunner::generateSheetRegions(PARAMS_C2C_SHEET_INPUT_PTR input, IMAGE_REGION_LIST& regionList) const
{
	if ( _processParameters->ProcessLeftStrip())
		generateStripRegions(input->_stripInputParamLeft, regionList);
	if ( _processParameters->ProcessRightStrip())
		generateStripRegions(input->_stripInputParamRight, regionList);

	// Wave regions
	if (  _processParameters->ProcessWave() )
	{
		auto waveCount = 0;
		for (auto& waveInput : input->_waveInputs)
		{
			generateWaveRegion(waveInput, regionList, waveCount == 0);
			++waveCount;
		}
	}
}

void baseAlgorithmRunner::generateStripRegions(PARAMS_C2C_STRIP_INPUT_PTR input, IMAGE_REGION_LIST& regionList) const
{
	const auto saveSourceStrip = (input->_side == LEFT) ? 
		_processParameters->SaveSourceLeftStrip() 
	  : _processParameters->SaveSourceRightStrip();

	const auto& stripRect = (input->_side == LEFT) 
		? _processParameters->LeftStripRect() 
		: _processParameters->RightStripRect();

	const auto& approxRect = (input->_side == LEFT)
		? toROIRect(_processParameters->I2SApproximateTriangleRectLeft())
		: toROIRect(_processParameters->I2SApproximateTriangleRectRight());

	input->_i2sInput->_approxTriangeROI = approxRect;
	auto& inputI2S = input->_i2sInput;

	// add strip region
	regionList.push_back(
		std::move(ImageRegion::createRegion
		(
			 //_frameContainer.get()
			_frame->image().get()
			, input->_paperEdgeInput->_stripImageSource
			, _processParameters
			, qrect2cvrect(stripRect)
			, _frameIndex
			, generateFullPathForElement<PARAMS_C2C_STRIP_INPUT_PTR>(input, "bmp", _processParameters, _frameIndex, _imageIndex, getFrameFolderName())
			, saveSourceStrip
		))
	);

	// get I2S region
	if ( input->_side == LEFT && _processParameters->ProcessLeftI2S() 
		 || input->_side == RIGHT && _processParameters->ProcessRightI2S() 
		)
	generateI2SRegion(inputI2S, regionList);

	///////////////////////
	////////// C2C ROIs
	if ( input->_side == LEFT && _processParameters->ProcessLeftC2C() 
		 || input->_side == RIGHT && _processParameters->ProcessRightC2C() 
		)
	{
		for (auto& _c2cROIInput : input->_c2cROIInputs)
		{
			generateC2CRegion(_c2cROIInput, regionList);
		}
	}
}

void baseAlgorithmRunner::generateI2SRegion(PARAMS_I2S_INPUT_PTR input, IMAGE_REGION_LIST& regionList) const
{
	const auto saveSourceI2S = (input->_side == LEFT) ? 
		_processParameters->SaveSourceLeftI2S() 
	  : _processParameters->SaveSourceRightI2S();

	regionList.push_back(std::move(ImageRegion::createRegion(_frame->image().get()
			, input->_triangleImageSource
			, _processParameters
			, roirect2cvrect(input->_approxTriangeROI)
			, _frameIndex
			, generateFullPathForElement<PARAMS_I2S_INPUT_PTR>(input, "bmp", _processParameters, _frameIndex, _imageIndex, getFrameFolderName())
			, saveSourceI2S)));

}

void baseAlgorithmRunner::generateC2CRegion(PARAMS_C2C_ROI_INPUT_PTR input, IMAGE_REGION_LIST& regionList) const
{
	const auto saveSourceC2C = (input->_side == LEFT) ? 
		_processParameters->SaveSourceLeftC2C() 
	  : _processParameters->SaveSourceRightC2C();

	auto& ROI = input;
	ROI->setGenerateOverlay(_processParameters->GenerateOverlays());
	
	regionList.push_back(
		std::move(ImageRegion::createRegion
		(
			_frame->image().get()
			, ROI->_ROIImageSource
			, _processParameters
			, roirect2cvrect(ROI->_ROI)
			, _frameIndex
			, generateFullPathForElement<PARAMS_C2C_ROI_INPUT_PTR>(input, "bmp", _processParameters, _frameIndex, _imageIndex, getFrameFolderName())
			, saveSourceC2C
		))
	);
}

void baseAlgorithmRunner::generateWaveRegion(PARAMS_WAVE_INPUT_PTR input, IMAGE_REGION_LIST& regionList, bool bDumpWave ) const
{
	auto& ROI = input;
	ROI->setGenerateOverlay(_processParameters->GenerateOverlays());

	regionList.push_back(
		std::move(ImageRegion::createRegion
		(
			_frame->image().get()
			, ROI->_waveImageSource
			, _processParameters
			, roirect2cvrect(ROI->_waveROI)
			, _frameIndex
			, generateFullPathForElement<PARAMS_WAVE_INPUT_PTR>(input, "bmp", _processParameters, _frameIndex, _imageIndex, getFrameFolderName())
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

PARAMS_C2C_SHEET_OUTPUT_PTR baseAlgorithmRunner::processSheet(PARAMS_C2C_SHEET_INPUT_PTR sheetInput)
{
	auto retVal = std::make_shared<PARAMS_C2C_SHEET_OUTPUT>(sheetInput);
	retVal->_result = ALG_STATUS_SUCCESS;
	   
	const auto leftStripLambda = [&] { retVal->_stripOutputParameterLeft = processStrip(sheetInput->_stripInputParamLeft); };
	const auto rightStripLambda = [&] { retVal->_stripOutputParameterRight = processStrip(sheetInput->_stripInputParamRight); };
	//const auto waveI2SLambda = [&] { retVal->_waveOutputs = processWaves(sheetInput->_waveInputs); };
	const auto waveLambda = [&] { retVal->_waveOutputs = processWaves(sheetInput->_waveInputs); };
	
	std::vector<std::function<void()>> processLambdas;
	if ( _processParameters->ProcessLeftStrip())
		processLambdas.emplace_back(leftStripLambda);
	if ( _processParameters->ProcessRightStrip())
		processLambdas.emplace_back(rightStripLambda);
	if ( _processParameters->ProcessWave())
		processLambdas.emplace_back(waveLambda);

	try
	{
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
	}
	catch ( BaseException& bex)
	{
		throw;
	}
	catch ( std::exception& ex)
	{
		RETHROW(CORE_ERROR{CORE_ERROR::ERR_CORE_ALGO_RUNNER_THROWN_RUNTIME_EXCEPTION, ""});
	}
	catch (...)
	{
		RETHROW(CORE_ERROR::ERR_CORE_ALGO_RUNNER_THROWN_RUNTIME_EXCEPTION, "");
	}
	return retVal;
}

// ------------------------------------------------------
//				Strip processing  FUNCTION 

PARAMS_C2C_STRIP_OUTPUT_PTR baseAlgorithmRunner::processStrip(PARAMS_C2C_STRIP_INPUT_PTR stripInput)
{
	const auto tStart = Utility::now_in_microseconds();
	auto retVal = std::make_shared<PARAMS_C2C_STRIP_OUTPUT>(stripInput);

	const auto edgeLambda = [&] { retVal->_paperEdgeOutput = processEdge(stripInput->_paperEdgeInput); };
	const auto i2sLambda = [&] { retVal->_i2sOutput = processI2S(stripInput->_i2sInput); };

	auto& roiInputs = stripInput->_c2cROIInputs;
	if (roiInputs.empty())
	{
		BASE_RUNNER_SCOPED_WARNING << "No C2C ROI defined in input parameters.";
	}

	auto calculateC2CfuncPar = 
		[&]{
			parallel_for_each (std::begin(roiInputs), std::end(roiInputs), [&](auto &in) 
			{
				retVal->_c2cROIOutputs.push_back(processC2CROI(in));
			});
		};

	auto calculateC2CfuncSec = 
		[&]{
			std::for_each (std::begin(roiInputs), std::end(roiInputs), [&](auto &in) 
			{
				retVal->_c2cROIOutputs.push_back(processC2CROI(in));
			});
		};

	std::vector<std::function<void()>> processLambdas;
	if (( _processParameters->ProcessLeftEdge() && stripInput->_side == LEFT ||  _processParameters->ProcessRightEdge() && stripInput->_side == RIGHT ))
		processLambdas.emplace_back(edgeLambda);
	if (( _processParameters->ProcessLeftI2S() && stripInput->_side == LEFT ||  _processParameters->ProcessRightI2S() && stripInput->_side == RIGHT ))
		processLambdas.emplace_back(i2sLambda);																					   
	if (( _processParameters->ProcessLeftC2C() && stripInput->_side == LEFT ||  _processParameters->ProcessRightC2C() && stripInput->_side == RIGHT ))
	{
		if (_bParallelCalc)
			processLambdas.emplace_back(calculateC2CfuncPar);
		else
			processLambdas.emplace_back(calculateC2CfuncSec);
	}

	try
	{
		if (_bParallelCalc)
		{
 			parallel_for_each(begin(processLambdas), end(processLambdas), [&](auto &func) 
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

		// process output from I2S and C2C ROIs
		processStripOutput(retVal);
	}
	catch ( BaseException& bex)
	{
		throw;
	}
	catch ( std::exception& ex)
	{
		RETHROW(CORE_ERROR::ERR_CORE_ALGO_RUNNER_THROWN_RUNTIME_EXCEPTION, "");
	}
	catch (...)
	{
		RETHROW(CORE_ERROR::ERR_CORE_ALGO_RUNNER_THROWN_RUNTIME_EXCEPTION, "");
	}
	RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_stripsHandledOk, (static_cast<double>(Utility::now_in_microseconds()) - static_cast<double>(tStart)) / 1000);
	return retVal;
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
		THROW_EX_INT(CORE_ERROR::ALGO_INIT_EDGE_FAILED);
	}
}

// ------------------------------------------------------
//				Edge processing  FUNCTION 

PARAMS_PAPEREDGE_OUTPUT_PTR baseAlgorithmRunner::processEdge(PARAMS_PAPEREDGE_INPUT_PTR input)
{
	const auto tStart = Utility::now_in_microseconds();
	auto retVal = std::make_shared<PARAMS_PAPEREDGE_OUTPUT>(input);

	if ( !_processParameters->EnableAlgorithmProcessing() )
	{
		BASE_RUNNER_SCOPED_LOG << "Edge detection [side " << input->_side << "] skipped due to process configuration";
		return retVal;
	}

	//BASE_RUNNER_SCOPED_LOG << "Edge detection [side " << input->_side << "] runs on thread #" << GetCurrentThreadId();

	try
	{
		detect_edge(input, retVal);
	}
	catch (...)
	{
		BASE_RUNNER_SCOPED_ERROR << "Function detect_edge has thrown exception";
		retVal->_result = ALG_STATUS_EXCEPTION_THROWN;
		THROW_EX_INT(CORE_ERROR::ALGO_PROCESS_EDGE_FAILED);
	}
	RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_edgeHandledOk, (static_cast<double>(Utility::now_in_microseconds()) - static_cast<double>(tStart)) / 1000);

	dumpOverlay<PARAMS_PAPEREDGE_OUTPUT_PTR>(retVal, _bAsyncWrite);
	return retVal;
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
		THROW_EX_INT(CORE_ERROR::ALGO_SHUTDOWN_EDGE_FAILED);
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
		THROW_EX_INT(CORE_ERROR::ALGO_INIT_I2S_FAILED);
	}
}

PARAMS_I2S_OUTPUT_PTR baseAlgorithmRunner::processI2S(PARAMS_I2S_INPUT_PTR input)
{
	const auto tStart = Utility::now_in_microseconds();
	auto retVal = std::make_shared<PARAMS_I2S_OUTPUT>(input);

	if ( !_processParameters->EnableAlgorithmProcessing() )
	{
		BASE_RUNNER_SCOPED_LOG << "I2S detection [side " << input->_side << "] skipped due to process configuration";
		return retVal;
	}

	//BASE_RUNNER_SCOPED_LOG << "I2S detection [side " << input->_side << "] runs on thread #" << GetCurrentThreadId();
	try
	{
		detect_i2s(input, retVal);
	}
	catch (...)
	{
		BASE_RUNNER_SCOPED_ERROR << "Function detect_i2s has thrown exception";
		retVal->_result = ALG_STATUS_EXCEPTION_THROWN;
		THROW_EX_INT(CORE_ERROR::ALGO_PROCESS_I2S_FAILED);
	}
	RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_I2SHandledOk, (static_cast<double>(Utility::now_in_microseconds()) - static_cast<double>(tStart)) / 1000);
	dumpOverlay<PARAMS_I2S_OUTPUT_PTR>(retVal, _bAsyncWrite);
	return retVal;
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
		THROW_EX_INT(CORE_ERROR::ALGO_SHUTDOWN_I2S_FAILED);
	}
}



void baseAlgorithmRunner::initC2CRoi(const INIT_PARAMETER& initParam) const
{
	C2C_ROI_INIT_PARAMETER c2cInitParam;
	c2cInitParam._roiRect = initParam._roiRect;

	const auto& buf = _processParameters->CircleTemplateBufferC2C().constData();
	const std::vector<char> data(buf, buf + _processParameters->CircleTemplateBufferC2C().size());
	c2cInitParam._templateImage = std::move(cv::imdecode(cv::Mat(data), CV_LOAD_IMAGE_GRAYSCALE));

	try
	{
		detect_c2c_roi_init(c2cInitParam);
	}
	catch (...)
	{
		BASE_RUNNER_SCOPED_ERROR << "Function detect_c2c_roi_init has thrown exception";
		THROW_EX_INT(CORE_ERROR::ALGO_INIT_C2C_FAILED);
	}
}

// ------------------------------------------------------
//				C2C 1 ROI processing  FUNCTION 

PARAMS_C2C_ROI_OUTPUT_PTR baseAlgorithmRunner::processC2CROI(PARAMS_C2C_ROI_INPUT_PTR input)
{
	const auto tStart = Utility::now_in_microseconds();
	auto retVal = std::make_shared<PARAMS_C2C_ROI_OUTPUT>(input);

	if ( !_processParameters->EnableAlgorithmProcessing() )
	{
		BASE_RUNNER_SCOPED_LOG << "C2C Detection [side : " << input->_side << "; index : " << input->_roiIndex << "] skipped due to process configuration";
		return retVal;
	}

	// allocate array of C2C outputs
	const auto& hsvCount = input->_colors.size();

	retVal->_result = ALG_STATUS_FAILED;
	retVal->_colorStatuses = { hsvCount, ALG_STATUS_FAILED };
	retVal->_colorCenters = { hsvCount, {0,0} };
	
	//BASE_RUNNER_SCOPED_LOG << "C2C Detection [side : " << input->_side << "; index : " << input->_roiIndex << "] runs on thread #" << GetCurrentThreadId();
	try
	{
		detect_c2c_roi(input, retVal);
	}
	catch (...)
	{
		BASE_RUNNER_SCOPED_ERROR << "Function detect_c2c_roi has thrown exception";
		retVal->_result = ALG_STATUS_EXCEPTION_THROWN;
		RETHROW(CORE_ERROR::ALGO_PROCESS_C2C_FAILED, "");
	}

	RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_C2CHandledOk, (static_cast<double>(Utility::now_in_microseconds()) - static_cast<double>(tStart)) / 1000);
	dumpOverlay<PARAMS_C2C_ROI_OUTPUT_PTR>(retVal, _bAsyncWrite);
	return retVal;
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
		THROW_EX_INT(CORE_ERROR::ALGO_SHUTDOWN_C2C_FAILED);
	}
}

void baseAlgorithmRunner::initWave(const INIT_PARAMETER& initParam)
{
	WAVE_INIT_PARAMETER waveInitParam;
	waveInitParam._roiRect = initParam._roiRect;

	const auto& buf = _processParameters->CircleTemplateBufferWave().constData();
	const std::vector<char> data(buf, buf + _processParameters->CircleTemplateBufferWave().size());
	waveInitParam._templateImage = std::move(cv::imdecode(cv::Mat(data), CV_LOAD_IMAGE_GRAYSCALE));

	try
	{
		detect_wave_init(waveInitParam);
	}
	catch (...)
	{
		BASE_RUNNER_SCOPED_ERROR << "Function detect_wave_init has thrown exception";
		THROW_EX_INT(CORE_ERROR::ALGO_INIT_WAVE_FAILED);
	}
}

// ------------------------------------------------------
//				Wave processing  FUNCTION 

PARAMS_WAVE_OUTPUT_PTR baseAlgorithmRunner::processWave(PARAMS_WAVE_INPUT_PTR input)
{
	auto retVal = std::make_shared<PARAMS_WAVE_OUTPUT>(input);

	if ( !_processParameters->EnableAlgorithmProcessing() )
	{
		BASE_RUNNER_SCOPED_LOG << "WAVE Detection [color : " << input->_circleColor._colorName.c_str() << "] skipped due to process configuration";
		return retVal;
	}

	// allocate array of wave outputs
	const auto& circleCount = input->_circlesCount;

	retVal->_result = ALG_STATUS_FAILED;
	//retVal->_colorDetectionResults = { static_cast<const uint64_t>(circleCount), ALG_STATUS_FAILED };
	//retVal->_colorCenters = { static_cast<const uint64_t>(circleCount), {0,0} };

	retVal->_colorDetectionResults.reserve(static_cast<const uint64_t>(circleCount));
	retVal->_colorCenters.reserve(static_cast<const uint64_t>(circleCount));
	
	BASE_RUNNER_SCOPED_LOG << "WAVE Detection [color : " << input->_circleColor._colorName.c_str() << "] runs in thread #" << GetCurrentThreadId();
	try
	{
		detect_wave(input, retVal);
	}
	catch (...)
	{
		BASE_RUNNER_SCOPED_ERROR << "Function detect_wave has thrown exception";
		retVal->_result = ALG_STATUS_EXCEPTION_THROWN;
		THROW_EX_INT(CORE_ERROR::ALGO_PROCESS_WAVE_FAILED);
	}

	dumpOverlay<PARAMS_WAVE_OUTPUT_PTR>(retVal, _bAsyncWrite);
	return retVal;
}

concurrent_vector<PARAMS_WAVE_OUTPUT_PTR> baseAlgorithmRunner::processWaves(const std::vector<PARAMS_WAVE_INPUT_PTR>& inputs)
{
	concurrent_vector<PARAMS_WAVE_OUTPUT_PTR> retVal;
	try
	{
		if (_bParallelCalc)
		{
			parallel_for_each (inputs.begin(), inputs.end(), [&](auto &in) 
			{
				retVal.push_back(processWave(in));
			});
		}
		else
		{
			std::for_each (inputs.begin(), inputs.end(), [&](auto &in) 
			{
				retVal.push_back(processWave(in));
			});
		}
	}
	catch ( BaseException& bex)
	{
		throw;
	}
	catch ( std::exception& ex)
	{
		RETHROW(CORE_ERROR::ALGO_PROCESS_WAVE_FAILED, "");
	}
	catch (...)
	{
		RETHROW(CORE_ERROR::ALGO_PROCESS_WAVE_FAILED, "");
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
		THROW_EX_INT(CORE_ERROR::ALGO_SHUTDOWN_WAVE_FAILED);
	}
}

std::string baseAlgorithmRunner::getFrameFolderName() const
{
	return fmt::format("frame_#{0}", _frameIndex);
}
	
void baseAlgorithmRunner::getSourceFrameIndexString()
{
	// get source frame ID from custom parameter passed by provider
	_sourceFrameIndexStr.clear();
	if (_frame->hasNamedParameter(NAMED_PROPERTY_SOURCE_PATH))
	{
		try
		{
			_sourceFramePath = std::any_cast<std::string>(_frame->getNamedParameter(NAMED_PROPERTY_SOURCE_PATH));
			_sourceFrameIndexStr = parseSourcePathForFrameIndex(_sourceFramePath);
		}
		catch (const std::bad_any_cast& e)
		{
			_sourceFramePath = "";
		}
	}

	if ( _sourceFrameIndexStr.empty())
		_sourceFrameIndexStr = std::to_string(_frameIndex);
	else
		_frameIndex = std::stoi(_sourceFrameIndexStr);
}
