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

#define BASE_RUNNER_SCOPED_LOG PRINT_INFO6 << "[baseAlgorithmRunner] : "
#define BASE_RUNNER_SCOPED_ERROR PRINT_ERROR << "[baseAlgorithmRunner] : "
#define BASE_RUNNER_SCOPED_WARNING PRINT_WARNING << "[baseAlgorithmRunner] : "

baseAlgorithmRunner::~baseAlgorithmRunner()
{
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

void baseAlgorithmRunner::fillCommonProcessParameters(std::shared_ptr<ABSTRACT_INPUT> input)
{
	input->setGenerateOverlay(_processParameters->GenerateOverlays());
	input->setPixel2MM_X(_processParameters->Pixel2MM_X());
	input->setPixel2MM_Y(_processParameters->Pixel2MM_Y());
}


void baseAlgorithmRunner::fillSheetProcessParameters(std::shared_ptr<PARAMS_C2C_SHEET_INPUT> input)
{
	// sheet
	fillCommonProcessParameters(std::static_pointer_cast<ABSTRACT_INPUT>(input));
	
	// strips 
	if (_processParameters->ProcessLeftStrip())
		fillStripProcessParameters(input->_stripInputParamLeft, LEFT);
	if (_processParameters->ProcessRightStrip())
		fillStripProcessParameters(input->_stripInputParamRight, RIGHT);

	if (_processParameters->ProcessWave())
		fillWaveProcessParameters(input->_waveInputs);
}

void baseAlgorithmRunner::fillStripProcessParameters(std::shared_ptr<PARAMS_C2C_STRIP_INPUT> input, SHEET_SIDE side)
{
	// fill base class parameters
	fillCommonProcessParameters(std::static_pointer_cast<ABSTRACT_INPUT>(input));
	input->_side = side;

	// fill edge parameters
	if (    side == LEFT && _processParameters->ProcessLeftEdge() 
		 || side == RIGHT && _processParameters->ProcessRightEdge() 
		)
		fillEdgeProcessParameters(input->_paperEdgeInput, side);

	// fill I2S parameters
	if (    side == LEFT && _processParameters->ProcessLeftI2S() 
		 || side == RIGHT && _processParameters->ProcessRightI2S() 
		)
	fillI2SProcessParameters( input->_i2sInput, side);

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
				fillC2CProcessParameters(input->_c2cROIInputs[i], side );
			}
		}	
	}
}

void baseAlgorithmRunner::fillEdgeProcessParameters(std::shared_ptr<PARAMS_PAPEREDGE_INPUT> input, const SHEET_SIDE side)
{
	fillCommonProcessParameters(std::static_pointer_cast<ABSTRACT_INPUT>(input));
	input->_approxDistanceFromEdgeX = _processParameters->EdgeApproximateDistanceX_px();
	input->_triangeApproximateY = _processParameters->EdgeTriangleApproximateY_px();
	input->_side = side;
}

void baseAlgorithmRunner::fillI2SProcessParameters(std::shared_ptr<PARAMS_I2S_INPUT> input, SHEET_SIDE side)
{
	fillCommonProcessParameters(std::static_pointer_cast<ABSTRACT_INPUT>(input));
	input->_side = side;
}

void baseAlgorithmRunner::fillC2CProcessParameters(std::shared_ptr<PARAMS_C2C_ROI_INPUT> input, const SHEET_SIDE side)
{
	fillCommonProcessParameters(std::static_pointer_cast<ABSTRACT_INPUT>(input));
	input->_side = side;
}

void baseAlgorithmRunner::fillWaveProcessParameters(std::vector<std::shared_ptr<PARAMS_WAVE_INPUT>>& inputs)
{
	for (const auto& color : _processParameters->ColorArray() )
	{
		auto input = std::make_shared<PARAMS_WAVE_INPUT>(_frame);
		fillCommonProcessParameters(std::static_pointer_cast<ABSTRACT_INPUT>(input));
		input->_circleColor = color2HSV(color);
		input->_waveROI = toROIRect(_processParameters->WaveROI());
		input->_circlesCount = _processParameters->NumberOfColorDotsPerLine();
		inputs.emplace_back(std::move(input));
	}
}


////////////////////////////////////////////////////////
////////////   Region generation

void baseAlgorithmRunner::generateSheetRegions(std::shared_ptr<PARAMS_C2C_SHEET_INPUT> input, IMAGE_REGION_LIST& regionList) const
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

void baseAlgorithmRunner::generateStripRegions(std::shared_ptr<PARAMS_C2C_STRIP_INPUT> input, IMAGE_REGION_LIST& regionList) const
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
			 _frameContainer.get()
			, input->_paperEdgeInput->_stripImageSource
			, _processParameters
			, qrect2cvrect(stripRect)
			, _frameIndex
			, generateFullPathForElement<std::shared_ptr<PARAMS_C2C_STRIP_INPUT>>(input, "bmp", _processParameters, _frameIndex, _imageIndex, getFrameFolderName())
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

void baseAlgorithmRunner::generateI2SRegion(std::shared_ptr<PARAMS_I2S_INPUT> input, IMAGE_REGION_LIST& regionList) const
{
	const auto saveSourceI2S = (input->_side == LEFT) ? 
		_processParameters->SaveSourceLeftI2S() 
	  : _processParameters->SaveSourceRightI2S();

	regionList.push_back(std::move(ImageRegion::createRegion(_frameContainer.get()
			, input->_triangleImageSource
			, _processParameters
			, roirect2cvrect(input->_approxTriangeROI)
			, _frameIndex
			, generateFullPathForElement<std::shared_ptr<PARAMS_I2S_INPUT>>(input, "bmp", _processParameters, _frameIndex, _imageIndex, getFrameFolderName())
			, saveSourceI2S)));

}

void baseAlgorithmRunner::generateC2CRegion(std::shared_ptr<PARAMS_C2C_ROI_INPUT> input, IMAGE_REGION_LIST& regionList) const
{
	const auto saveSourceC2C = (input->_side == LEFT) ? 
		_processParameters->SaveSourceLeftC2C() 
	  : _processParameters->SaveSourceRightC2C();

	auto& ROI = input;
	ROI->setGenerateOverlay(_processParameters->GenerateOverlays());
	
	regionList.push_back(
		std::move(ImageRegion::createRegion
		(
			_frameContainer.get()
			, ROI->_ROIImageSource
			, _processParameters
			, roirect2cvrect(ROI->_ROI)
			, _frameIndex
			, generateFullPathForElement<std::shared_ptr<PARAMS_C2C_ROI_INPUT>>(input, "bmp", _processParameters, _frameIndex, _imageIndex, getFrameFolderName())
			, saveSourceC2C
		))
	);
}

void baseAlgorithmRunner::generateWaveRegion(std::shared_ptr<PARAMS_WAVE_INPUT> input, IMAGE_REGION_LIST& regionList, bool bDumpWave ) const
{
	auto& ROI = input;
	ROI->setGenerateOverlay(_processParameters->GenerateOverlays());

	regionList.push_back(
		std::move(ImageRegion::createRegion
		(
			_frameContainer.get()
			, ROI->_waveImageSource
			, _processParameters
			, roirect2cvrect(ROI->_waveROI)
			, _frameIndex
			, generateFullPathForElement<std::shared_ptr<PARAMS_WAVE_INPUT>>(input, "bmp", _processParameters, _frameIndex, _imageIndex, getFrameFolderName())
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

std::shared_ptr<PARAMS_C2C_SHEET_OUTPUT> baseAlgorithmRunner::processSheet(std::shared_ptr<PARAMS_C2C_SHEET_INPUT> sheetInput)
{
	auto retVal = std::make_shared<PARAMS_C2C_SHEET_OUTPUT>(sheetInput);
	retVal->_result = ALG_STATUS_SUCCESS;

	const auto leftStripLambda = [&] { retVal->_stripOutputParameterLeft = processStrip(sheetInput->_stripInputParamLeft); };
	const auto rightStripLambda = [&] { retVal->_stripOutputParameterRight = processStrip(sheetInput->_stripInputParamRight); };
	const auto waveLambda = [&] { retVal->_waveOutputs = processWaves(sheetInput->_waveInputs); };
	
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

	processWaveOutputs(retVal->_waveOutputs);
	return retVal;
}

// ------------------------------------------------------
//				Strip processing  FUNCTION 

std::shared_ptr<PARAMS_C2C_STRIP_OUTPUT> baseAlgorithmRunner::processStrip(std::shared_ptr<PARAMS_C2C_STRIP_INPUT> stripInput)
{
	auto tStart = Utility::now_in_microseconds();
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


	// update C2C centers with I2S coordinates offsets
	auto const offsetX = retVal->_i2sOutput->_triangeCorner._x;
	auto const offsetY = retVal->_i2sOutput->_triangeCorner._y;
	if (_bParallelCalc)
	{
 		parallel_for_each(retVal->_c2cROIOutputs.begin(), retVal->_c2cROIOutputs.end(), [&](auto &c2cOut) 
		{
			std::for_each(c2cOut->_colorCenters.begin(), c2cOut->_colorCenters.end(), [&](auto &colorCenter)
			{
				colorCenter._x -= offsetX;
				colorCenter._y -= offsetY;
			});
		});
	}
	else
	{
		std::for_each(retVal->_c2cROIOutputs.begin(), retVal->_c2cROIOutputs.end(), [&](auto &c2cOut) 
		{
			std::for_each(c2cOut->_colorCenters.begin(), c2cOut->_colorCenters.end(), [&](auto &colorCenter)
			{
				colorCenter._x -= offsetX;
				colorCenter._y -= offsetY;
			});
		});
	}

	retVal->_result = 
		std::all_of(retVal->_c2cROIOutputs.begin(), retVal->_c2cROIOutputs.end(), [](auto& out) { return out->_result == ALG_STATUS_SUCCESS; } )
		? ALG_STATUS_SUCCESS
		: ALG_STATUS_FAILED;

	processStripOutput(retVal);
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
	}
}

// ------------------------------------------------------
//				Edge processing  FUNCTION 

std::shared_ptr<PARAMS_PAPEREDGE_OUTPUT> baseAlgorithmRunner::processEdge(std::shared_ptr<PARAMS_PAPEREDGE_INPUT> input)
{
	auto tStart = Utility::now_in_microseconds();
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
	}
	RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_edgeHandledOk, (static_cast<double>(Utility::now_in_microseconds()) - static_cast<double>(tStart)) / 1000);

	dumpOverlay<std::shared_ptr<PARAMS_PAPEREDGE_OUTPUT>>(retVal);
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

std::shared_ptr<PARAMS_I2S_OUTPUT> baseAlgorithmRunner::processI2S(std::shared_ptr<PARAMS_I2S_INPUT> input)
{
	auto tStart = Utility::now_in_microseconds();
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
	}
	RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_I2SHandledOk, (static_cast<double>(Utility::now_in_microseconds()) - static_cast<double>(tStart)) / 1000);
	dumpOverlay<std::shared_ptr<PARAMS_I2S_OUTPUT>>(retVal);
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
	}
}

// ------------------------------------------------------
//				C2C 1 ROI processing  FUNCTION 

std::shared_ptr<PARAMS_C2C_ROI_OUTPUT> baseAlgorithmRunner::processC2CROI(std::shared_ptr<PARAMS_C2C_ROI_INPUT> input)
{
	auto tStart = Utility::now_in_microseconds();
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
	}

	RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_C2CHandledOk, (static_cast<double>(Utility::now_in_microseconds()) - static_cast<double>(tStart)) / 1000);
	dumpOverlay<std::shared_ptr<PARAMS_C2C_ROI_OUTPUT>>(retVal);
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
	}
}

// ------------------------------------------------------
//				Wave processing  FUNCTION 

std::shared_ptr<PARAMS_WAVE_OUTPUT> baseAlgorithmRunner::processWave(std::shared_ptr<PARAMS_WAVE_INPUT> input)
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
	}

	dumpOverlay<std::shared_ptr<PARAMS_WAVE_OUTPUT>>(retVal);
	return retVal;
}

concurrent_vector<std::shared_ptr<PARAMS_WAVE_OUTPUT>> baseAlgorithmRunner::processWaves(const std::vector<std::shared_ptr<PARAMS_WAVE_INPUT>>& inputs)
{
	concurrent_vector<std::shared_ptr<PARAMS_WAVE_OUTPUT>> retVal;
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

std::string baseAlgorithmRunner::getFrameFolderName() const
{
	return fmt::format("frame_#{0}", _frameIndex);
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

void baseAlgorithmRunner::processStripOutput(std::shared_ptr<PARAMS_C2C_STRIP_OUTPUT> stripOutput )
{
	if ( _processParameters->EnableAnyDataSaving() &&  _processParameters->EnableCSVSaving())
	{
		const auto csvFolder = _csvFolder;
		const auto imgIndex = _imageIndex;
		const auto frameIndex = _frameIndex;
		const auto jobID = _processParameters->JobID();

		if (_bParallelCalc)
		{
			if (_processParameters->SaveC2CRegistrationCSV())
			{
				task<void> sLeft([csvFolder, imgIndex, frameIndex, jobID, stripOutput]()
				{
					dumpRegistrationCSV(stripOutput, jobID, frameIndex, imgIndex, csvFolder );
				});

				if (_processParameters->ProcessRightStrip())
				{
					task<void> sRight([csvFolder, imgIndex, frameIndex, jobID, stripOutput]()
					{
						dumpRegistrationCSV(stripOutput, jobID, frameIndex, imgIndex, csvFolder  );
					});
				}
			}

			if (_processParameters->SaveI2SPlacementCSV())
			{
				task<void> iLeft([csvFolder, imgIndex, frameIndex, stripOutput]()
				{
					dumpPlacementCSV(stripOutput, frameIndex, imgIndex, csvFolder  );
				});

				if (_processParameters->ProcessRightStrip())
				{
					task<void> iRight([csvFolder, imgIndex, frameIndex, stripOutput]()
					{
						dumpPlacementCSV(stripOutput, frameIndex, imgIndex, csvFolder  );
					});
				}
			}
		}
		else
		{
			if (_processParameters->SaveC2CRegistrationCSV())
			{
				dumpRegistrationCSV(stripOutput, _processParameters->JobID(), _frameIndex, _imageIndex, _csvFolder  );
				if (_processParameters->ProcessRightStrip())
				{
					dumpRegistrationCSV(stripOutput, _processParameters->JobID(), _frameIndex, _imageIndex, _csvFolder  );
				}
			}
			
			if (_processParameters->SaveI2SPlacementCSV())
			{
				dumpPlacementCSV(stripOutput, _frameIndex, _imageIndex, _csvFolder  );
				if (_processParameters->ProcessRightStrip())
				{
					dumpPlacementCSV(stripOutput, _frameIndex, _imageIndex, _csvFolder  );
				}
			}
		}
	}
}


void baseAlgorithmRunner::processWaveOutputs(const concurrent_vector<std::shared_ptr<PARAMS_WAVE_OUTPUT>> & waveOutputs )
{
	if ( _processParameters->EnableAnyDataSaving() &&  _processParameters->EnableCSVSaving())
	{
		const auto csvFolder = _csvFolder;
		const auto imgIndex = _imageIndex;
		const auto frameIndex = _frameIndex;
		const auto jobID = _processParameters->JobID();

		if (_bParallelCalc)
		{
			if (_processParameters->SaveWaveCSV())
			{
				task<void> tWaveCSV([csvFolder, imgIndex, frameIndex, jobID, waveOutputs]()
				{
					dumpWaveCSV(waveOutputs, jobID, frameIndex, imgIndex, csvFolder );
				});
			}
		}
		else
		{
			if (_processParameters->SaveWaveCSV())
			{
				dumpWaveCSV(waveOutputs, jobID, frameIndex, imgIndex, csvFolder );
			}
		}
	}
}