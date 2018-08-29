#include "abstractalgohandler.h"

////////////////////////////////
// algorithmic functions
#include "algo_edge_impl.h"
#include "algo_i2s_impl.h"
#include "algo_c2c_roi_impl.h"
#include "util.h"
#include <filesystem>

namespace fs = std::filesystem;

using namespace LandaJune;
using namespace Algorithms;
using namespace Parameters;
using namespace Threading;
using namespace Core;

#define ABSTRACTALGO_HANDLER_SCOPED_LOG PRINT_INFO6 << "[abstractAlgoHandler] : "
#define ABSTRACTALGO_HANDLER_SCOPED_ERROR PRINT_ERROR << "[abstractAlgoHandler] : "
#define ABSTRACTALGO_HANDLER_SCOPED_WARNING PRINT_WARNING << "[abstractAlgoHandler] : "

void abstractAlgoHandler::init(std::shared_ptr<BaseParameter> parameters)
{
	createCSVFolder();
	validateProcessParameters(parameters);
}

void abstractAlgoHandler::process(const FrameRef* frame)
{
	// minimal process implementation
	_frame = frame;
	constructFrameContainer(frame, _processParameters->ScanBitDepth());

	_bParallelizeCalculations = _processParameters->ParalellizeCalculations();
	_frameIndex = frame->getIndex();
	_imageIndex = _frameIndex % _processParameters->PanelCount();
}

void abstractAlgoHandler::validateProcessParameters(std::shared_ptr<BaseParameter> parameters)
{
	_processParameters = std::dynamic_pointer_cast<ProcessParameter>(parameters);
}


void abstractAlgoHandler::constructFrameContainer(const FrameRef* frame, int bitsPerPixel)
{
	_frameContainer = std::make_unique<cv::Mat>(frame->getHeight(), frame->getWidth(),
		CV_MAKETYPE(CV_8U, bitsPerPixel / 8), (void*)frame->getBits());
}

void abstractAlgoHandler::fillCommonProcessParameters(ABSTRACT_INPUT& input)
{
	const auto& bGenerateOverlays = _processParameters->GenerateOverlays();
	input.setGenerateOverlay(bGenerateOverlays);
	input.setPixel2MM_X(_processParameters->Pixel2MM_X());
	input.setPixel2MM_Y(_processParameters->Pixel2MM_Y());
}


void abstractAlgoHandler::fillSheetProcessParameters(PARAMS_C2C_SHEET_INPUT& input)
{
	// sheet
	fillCommonProcessParameters(static_cast<ABSTRACT_INPUT&>(input));
	
	// strips 
	fillStripProcessParameters(input._stripInputParamLeft, LEFT);
	if (_processParameters->ProcessRightSide())
	{
		fillStripProcessParameters(input._stripInputParamRight, RIGHT);
	}
}

void abstractAlgoHandler::fillStripProcessParameters(PARAMS_C2C_STRIP_INPUT& input, SHEET_SIDE side)
{
	// fill base class parameters
	fillCommonProcessParameters(static_cast<ABSTRACT_INPUT&>(input));
	input._side = side;

	// fill edge parameters
	fillEdgeProcessParameters(input._paperEdgeInput, side);

	// fill I2S parameters
	fillI2SProcessParameters( input._i2sInput, side);

	// get rectangles of correspondent C2C ROIs
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

void abstractAlgoHandler::fillEdgeProcessParameters(PARAMS_PAPEREDGE_INPUT& input, SHEET_SIDE side)
{
	fillCommonProcessParameters(static_cast<ABSTRACT_INPUT&>(input));
	input._approxDistanceFromEdgeX = _processParameters->EdgeApproximateDistanceX_px();
	input._triangeApproximateY = _processParameters->EdgeTriangleApproximateY_px();
	input._side = side;
}

void abstractAlgoHandler::fillI2SProcessParameters(PARAMS_I2S_INPUT& input, SHEET_SIDE side)
{
	fillCommonProcessParameters(static_cast<ABSTRACT_INPUT&>(input));
	input._side = side;
}

void abstractAlgoHandler::fillC2CProcessParameters(PARAMS_C2C_ROI_INPUT& input, SHEET_SIDE side)
{
	fillCommonProcessParameters(static_cast<ABSTRACT_INPUT&>(input));
	input._side = side;
}

void abstractAlgoHandler::fillWaveProcessParameters(PARAMS_WAVE_INPUT& input)
{
}

void abstractAlgoHandler::dumpOverlays(const cv::Mat& img, const std::string& fileName)
{
	if (!_processParameters->DisableAllROISaving() && _processParameters->GenerateOverlays())
	{
		dumpMatFile(img, CV_COPY_REGION::getSavePath(_processParameters, fileName, _frameIndex), false, _bParallelizeCalculations);
	}
}

void abstractAlgoHandler::dumpFrameCSV(const PARAMS_C2C_STRIP_OUTPUT& stripOut)
{
	std::ostringstream ss;
	ss << "Pattern Type :,Registration\r\n\r\n"
	<< "Job Id :," << _processParameters->JobID() << "\r\n"
	<< "Flat Id :," << _frameIndex << "\r\n"
	<< "ImageIndex ID :," << _imageIndex << "\r\n"
	<< "Registration Side :" << SIDE_NAMES[stripOut._input->_side] << "\r\n"
	<< "Registration Overall Status :," << (stripOut._result == ALG_STATUS_SUCCESS ) ? "Success" : "Fail";
	ss << "\r\nInk\\Sets,";

	for (const auto& out : stripOut._c2cROIOutputs)
	{
		ss << "Set #" << out._input->_roiIndex << " :," << (out._result == ALG_STATUS_SUCCESS ) ? "Success" : "Fail";
	}
	ss << "\r\n";

	for ( int i = 0; i  <  stripOut._c2cROIOutputs[0]._input->_colors.size(); i++)
	{
		ss << stripOut._c2cROIOutputs[0]._input->_colors[i]._colorName;
		for (const auto& out : stripOut._c2cROIOutputs)
		{
			ss << "," << out._colorCenters[i]._x << "," << out._colorCenters[i]._y;
		}
		ss << "\r\n";
	}

	auto const& fPath = QString::fromStdString(fmt::format("{0}\\{1}.csv"
	, _csvFolder
	, _frameIndex ));

	QFile fout (fPath);

	const auto & res = fout.open(QFile::WriteOnly | QFile::Text);

	const std::string& textOut = ss.str();
	fout.write(textOut.c_str(), textOut.size());

/*
Cyan,8563,-124,160654,-70,312784,-75,464889,-274,617626,-332
Black,8561,2987,160641,3042,312759,3038,464889,2851,617576,2792
Yellow,11614,-293,163704,-247,315777,-253,467938,-439,620625,-492
Magenta,11611,2906,163720,2956,315809,2954,467939,2767,620661,2717
*/
}

std::string abstractAlgoHandler::getBatchRootFolder()
{
	return fmt::format("{0}", _processParameters->JobID());
}

std::string abstractAlgoHandler::getFrameFolderName()
{
	return fmt::format("frame_#{2}", _frameIndex);
}

void abstractAlgoHandler::createCSVFolder()
{
	auto rootPath = _processParameters->RootOutputFolder().toStdString();
	if (rootPath.empty())
	{
		rootPath = DEFAULT_OUT_FOLDER;
	}

	const fs::path p{ 
		fmt::format("{0}\\{1}\\RawResults"
		, rootPath
		, _processParameters->JobID() )
	};
	
	if (!is_directory(p) || !exists(p))
	{
		try
		{
			create_directories(p); // create CSV folder
		}
		catch (fs::filesystem_error& er)
		{

		}
	}

	_csvFolder = p.string();
}

void abstractAlgoHandler::copyRegions(CV_COPY_REGION_LIST& regionList)
{
	// performs actual deep copy of selected regions
	if (_bParallelizeCalculations)
	{
		FUTURE_VECTOR<void> futureCopyRegionsList;
		auto& pool = TaskThreadPools::algorithmsThreadPool();

		// run first 1 - n ROIs on thread pool
		std::for_each(
			regionList.begin() + 1,
			regionList.end(),
			[&futureCopyRegionsList, &pool](auto &in)
		{
			futureCopyRegionsList.emplace_back(TaskThreadPools::postJob(pool, CV_COPY_REGION::performCopy, std::ref(in)));
		}
		);

		// run first copy regions on this thread
		CV_COPY_REGION::performCopy(regionList.front());

		// wait for previous ROIs to complete
		WAIT_ALL(futureCopyRegionsList);
	}
	else
	{
		std::for_each(regionList.begin(), regionList.end(), CV_COPY_REGION::performCopy);
	}
}


void abstractAlgoHandler::generateSheetRegions(PARAMS_C2C_SHEET_INPUT& input, CV_COPY_REGION_LIST& regionList) const
{
	auto& inputStripLeft = input._stripInputParamLeft;
	generateStripRegions(inputStripLeft, regionList);

	if (_processParameters->ProcessRightSide())
	{
		auto& inputStripRight = input._stripInputParamRight;
		generateStripRegions(inputStripRight, regionList);
	}
}

void abstractAlgoHandler::generateStripRegions(PARAMS_C2C_STRIP_INPUT& input, CV_COPY_REGION_LIST& regionList) const
{
	const auto dumpStrip = (input._side == LEFT) ? 
		_processParameters->DumpLeftStrip() 
	  : _processParameters->DumpRightStrip();

	const std::string name = (input._side == LEFT) 
		? "strip_[LEFT]" 
		: "strip_[RIGHT]";

	const auto& stripRect = (input._side == LEFT) 
		? _processParameters->LeftStripRect() 
		: _processParameters->RightStripRect();

	// add strip region
	regionList.emplace_back(
		CV_COPY_REGION
		(
			*_frameContainer
			, input._paperEdgeInput._stripImageSource
			, _processParameters
			, qrect2cvrect(stripRect)
			, _frameIndex
			, name
			, dumpStrip
		)
	);

	// get I2S region
	const auto& approxRect = (input._side == LEFT)
		? toROIRect(_processParameters->I2SApproximateTriangleRectLeft())
		: toROIRect(_processParameters->I2SApproximateTriangleRectRight());

	input._i2sInput._approxTriangeROI = approxRect;
	auto& inputI2S = input._i2sInput;
	generateI2SRegions(inputI2S, regionList);



	///////////////////////
	////////// C2C ROIs
	for (auto i = 0; i < input._c2cROIInputs.size(); i++)
	{
		generateC2CRegions(input._c2cROIInputs[i], regionList);
	}
}

void abstractAlgoHandler::generateI2SRegions(PARAMS_I2S_INPUT& input, CV_COPY_REGION_LIST& regionList) const
{
	const std::string name = (input._side == LEFT)
		? "I2S_[LEFT]"
		: "I2S_[RIGHT]";

	regionList.emplace_back(
		CV_COPY_REGION
		(
			*_frameContainer
			, input._triangleImageSource
			, _processParameters
			, roirect2cvrect(input._approxTriangeROI)
			, _frameIndex
			, name
			, _processParameters->DumpI2S()
		)
	);
}

void abstractAlgoHandler::generateC2CRegions(PARAMS_C2C_ROI_INPUT& input, CV_COPY_REGION_LIST& regionList) const
{
	const std::string name = (input._side == LEFT)
		? "C2C_[LEFT]_{0}"
		: "C2C_[RIGHT]_{0}";

	auto& ROI = input;
	ROI.setGenerateOverlay(input.GenerateOverlay());
	regionList.emplace_back(
		CV_COPY_REGION
		(
			*_frameContainer
			, ROI._ROIImageSource
			, _processParameters
			, roirect2cvrect(ROI._ROI)
			, _frameIndex
			, fmt::format(name, ROI._roiIndex)
			, _processParameters->DumpC2CROIs()
		)
	);
}

void abstractAlgoHandler::generateWaveRegions(PARAMS_WAVE_INPUT& input, CV_COPY_REGION_LIST& regionList)
{

}

///////////////////////////////////////////////////
////////////// WHOLE SHEET  FUNCTION //////////////
///////////////////////////////////////////////////


PARAMS_C2C_SHEET_OUTPUT abstractAlgoHandler::processSheet(const PARAMS_C2C_SHEET_INPUT& sheetInput)
{
	PARAMS_C2C_SHEET_OUTPUT retVal;
	retVal._input = sheetInput;
	retVal._result = ALG_STATUS_SUCCESS;

	if (_processParameters->DisableAllAlgorithmProcessing())
	{
		ABSTRACTALGO_HANDLER_SCOPED_LOG << " ---- All algorithm processing is disabled. Skipping calculations";
		return retVal;
	}

	if (_bParallelizeCalculations)
	{
		/////////////////////////////////////
		/// calculate strip regions in parallel
		FUTURE_VECTOR<PARAMS_C2C_STRIP_OUTPUT> _futureStripOutputList;
		if (_processParameters->ProcessRightSide())
		{
			// post right strip function to the thread pool
			_futureStripOutputList.emplace_back(TaskThreadPools::postJob(TaskThreadPools::algorithmsThreadPool(), &abstractAlgoHandler::processStrip, this, std::ref(sheetInput._stripInputParamRight), false));
		}

		// run left strip calculations directly in current thread and get outputs
		retVal._stripOutputParameterLeft = processStrip(sheetInput._stripInputParamLeft, true);

		// get outputs from futures
		if (!_futureStripOutputList.empty())
			retVal._stripOutputParameterRight = _futureStripOutputList[0].get();
	}
	else
	{
		/////////////////////////////////////
		/// calculate strip regions sequentially

		retVal._stripOutputParameterLeft = processStrip(sheetInput._stripInputParamLeft, true);
		if (_processParameters->ProcessRightSide())
			retVal._stripOutputParameterRight = processStrip(sheetInput._stripInputParamRight, false);
	}

	return retVal;
}

///////////////////////////////////////////////////
/////////////// SIDE STRIP FUNCTION ///////////////
///////////////////////////////////////////////////

PARAMS_C2C_STRIP_OUTPUT abstractAlgoHandler::processStrip(const PARAMS_C2C_STRIP_INPUT& stripInput, const bool detectEdge)
{
	PARAMS_C2C_STRIP_OUTPUT retVal;
	retVal._input = stripInput;

	if (_bParallelizeCalculations)
	{
		/////////////////////////////////////
		/// calculate I2S and C2C regions in parallel

		FUTURE_VECTOR<PARAMS_PAPEREDGE_OUTPUT> _futureEdgeOutputList;

		// post edge detection function to the thread pool
		if (detectEdge)
		{
			_futureEdgeOutputList.emplace_back(TaskThreadPools::postJob(TaskThreadPools::algorithmsThreadPool(), &abstractAlgoHandler::processEdge, this, std::ref(stripInput._paperEdgeInput)));
		}

		/// calculate I2S directly
		retVal._i2sOutput = processI2S(stripInput._i2sInput);

		// get output from pooled function
		if (!_futureEdgeOutputList.empty())
			retVal._paperEdgeOutput = _futureEdgeOutputList[0].get();
	}
	else
	{
		if (detectEdge)
		{
			retVal._paperEdgeOutput = processEdge(stripInput._paperEdgeInput);
		}
		retVal._i2sOutput = processI2S(stripInput._i2sInput);
	}

	/////////////////////////////////////
	/// calculate ROIs in parallel

	auto& roiInputs = stripInput._c2cROIInputs;
	if (roiInputs.empty())
	{
		ABSTRACTALGO_HANDLER_SCOPED_LOG << "No C2C ROI defined in input parameters.";
		return retVal;
	}

	if (_bParallelizeCalculations)
	{
		// prepare vector of futures
		FUTURE_VECTOR<PARAMS_C2C_ROI_OUTPUT> _futureROIList;

		// run first 1 - n ROIs on thread pool
		std::for_each(roiInputs.begin() + 1, roiInputs.end(),
			[&_futureROIList, this](auto &in)
		{
			_futureROIList.emplace_back(TaskThreadPools::postJob(TaskThreadPools::algorithmsThreadPool(), &abstractAlgoHandler::processC2CROI, this, std::ref(in)));
		}
		);

		// calculate first ROI on this thread
		retVal._c2cROIOutputs.push_back(processC2CROI(roiInputs.front()));

		// wait for previous ROIs to complete
		WAIT_ALL(_futureROIList);

		// now get results from futures
		for (auto& i : _futureROIList)
			retVal._c2cROIOutputs.push_back(i.get());
	}
	else
	{
		std::for_each(
			roiInputs.begin(),  // start from first element
			roiInputs.end(),	// end element
			[&retVal, this](auto &in) // lambda function which calculates ROI and adds output to a vector of outputs
		{
			retVal._c2cROIOutputs.push_back(this->processC2CROI(in));
		}
		);
	}
	return retVal;
}

void abstractAlgoHandler::initEdge(const INIT_PARAMETER& initParam) const
{
	try
	{
		detect_edge_init(initParam);
	}
	catch (...)
	{
		ABSTRACTALGO_HANDLER_SCOPED_ERROR << "Function detect_edge_init has thrown exception";
	}
}

PARAMS_PAPEREDGE_OUTPUT abstractAlgoHandler::processEdge(const PARAMS_PAPEREDGE_INPUT& input)
{
	PARAMS_PAPEREDGE_OUTPUT retVal;
	retVal._input = input;
	ABSTRACTALGO_HANDLER_SCOPED_LOG << "Edge detection [side " << input._side << "] runs on thread #" << GetCurrentThreadId();

	try
	{
		detect_edge(input, retVal);
	}
	catch (...)
	{
		ABSTRACTALGO_HANDLER_SCOPED_ERROR << "Function detect_edge has thrown exception";
		retVal._result = ALG_STATUS_EXCEPTION_THROWN;
	}

	dumpOverlays(retVal._edgeOverlay, fmt::format("strip_[{0}]_OVERLAY.bmp", SIDE_NAMES[input._side]));
	return std::move(retVal);
}

void abstractAlgoHandler::shutdownEdge() const
{
	try
	{
		detect_edge_shutdown();
	}
	catch (...)
	{
		ABSTRACTALGO_HANDLER_SCOPED_ERROR << "Function detect_edge_shutdown has thrown exception";
	}
}

///////////////////////////////////////////////////
////////////////// I2S FUNCTION ///////////////////
///////////////////////////////////////////////////

void abstractAlgoHandler::initI2S(const INIT_PARAMETER& initParam) const
{
	try
	{
		detect_i2s_init(initParam);
	}
	catch (...)
	{
		ABSTRACTALGO_HANDLER_SCOPED_ERROR << "Function detect_i2s_init has thrown exception";
	}
}

PARAMS_I2S_OUTPUT abstractAlgoHandler::processI2S(const PARAMS_I2S_INPUT& input)
{
	PARAMS_I2S_OUTPUT retVal;
	retVal._input = input;

	ABSTRACTALGO_HANDLER_SCOPED_LOG << "I2S detection [side " << input._side << "] runs on thread #" << GetCurrentThreadId();
	try
	{
		detect_i2s(input, retVal);
	}
	catch (...)
	{
		ABSTRACTALGO_HANDLER_SCOPED_ERROR << "Function detect_i2s has thrown exception";
		retVal._result = ALG_STATUS_EXCEPTION_THROWN;
	}

	dumpOverlays(retVal._triangleOverlay, fmt::format("I2S_[{0}]_OVERLAY.bmp", SIDE_NAMES[input._side]));
	return std::move(retVal);
}

void abstractAlgoHandler::shutdownI2S() const
{
	try
	{
		detect_i2s_shutdown();
	}
	catch (...)
	{
		ABSTRACTALGO_HANDLER_SCOPED_ERROR << "Function detect_i2s_shutdown has thrown exception";
	}
}


///////////////////////////////////////////////////
/////////////// C2C ONE ROI FUNCTION //////////////
///////////////////////////////////////////////////


void abstractAlgoHandler::initC2CRoi(const INIT_PARAMETER& initParam) const
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
		ABSTRACTALGO_HANDLER_SCOPED_ERROR << "Function detect_c2c_roi_init has thrown exception";
	}
}

PARAMS_C2C_ROI_OUTPUT abstractAlgoHandler::processC2CROI(const PARAMS_C2C_ROI_INPUT& input)
{
	PARAMS_C2C_ROI_OUTPUT retVal;
	retVal._input = input;

	// allocate array of C2C outputs
	const auto& hsvCount = input._colors.size();

	retVal._result = ALG_STATUS_FAILED;
	retVal._colorStatuses = { hsvCount, ALG_STATUS_FAILED };
	retVal._colorCenters = { hsvCount, {0,0} };
	
	ABSTRACTALGO_HANDLER_SCOPED_LOG << "C2C Detection [side : " << input._side << "; index : " << input._roiIndex << "] runs in thread #" << GetCurrentThreadId();
	try
	{
		detect_c2c_roi(input, retVal);
	}
	catch (...)
	{
		ABSTRACTALGO_HANDLER_SCOPED_ERROR << "Function detect_c2c_roi has thrown exception";
		retVal._result = ALG_STATUS_EXCEPTION_THROWN;
	}

	// double check for saving overlays to avoid unnecessary array iteration
	if (!_processParameters->DisableAllROISaving() && input.GenerateOverlay())
	{
		dumpOverlays(retVal._colorOverlay, fmt::format("C2C_[{0}]_{1}_OVERLAY.bmp", SIDE_NAMES[input._side], input._roiIndex));
	}
	return retVal;
}

void abstractAlgoHandler::shutdownC2CRoi() const
{
	try
	{
		detect_c2c_roi_shutdown();
	}
	catch (...)
	{
		ABSTRACTALGO_HANDLER_SCOPED_ERROR << "Function detect_c2c_roi_shutdown has thrown exception";
	}
}

void abstractAlgoHandler::initWave(const INIT_PARAMETER& initParam)
{
}

PARAMS_WAVE_OUTPUT abstractAlgoHandler::processWave(const PARAMS_WAVE_INPUT& input)
{
	PARAMS_WAVE_OUTPUT out;
	return out;
}

void abstractAlgoHandler::shutdownWave()
{
}


