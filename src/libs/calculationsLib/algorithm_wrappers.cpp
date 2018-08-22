#include "algorithm_wrappers.h"
#include "TaskThreadPool.h"
#include "ProcessParameter.h"
#include "frameRef.h"
#include "common/june_exceptions.h"

#include "algo_c2c_roi_impl.h"
#include "algo_edge_impl.h"
#include "algo_i2s_impl.h"

#include <algorithm>
#include <filesystem>

#include "format.h"

///////////////
//  OPEN CV

#include <opencv/cv.h> 
#include <opencv2/imgcodecs.hpp>
#include <QDirIterator>
#ifdef _DEBUG
#pragma comment(lib, "opencv_world342d.lib")
#else
#pragma comment(lib, "opencv_world342.lib")
#endif


///////////////////////////////////////
/// change ALGO_PAR to 0 to switch to sequential calculations instead of parallel
#define ALGO_PAR 0

using namespace LandaJune::Parameters;
using namespace LandaJune::Threading;
using namespace LandaJune::Algorithms;
using namespace LandaJune::Core;

namespace fs = std::filesystem;
const std::string DEFAULT_OUT_FOLDER = "c:\\temp\\june_out";

static std::mutex _createDirmutex;

std::string getSavePath (const FrameRef* frame, const std::string& itemName )
{
	auto rootPath = frame->getProcessParams()->RootOutputFolder().toStdString();
	if (rootPath.empty())
	{
		rootPath = DEFAULT_OUT_FOLDER;
	}
	return fmt::format("{0}\\{1}\\frame_#{2}\\{3}.bmp"
		, rootPath
		, frame->getProcessParams()->JobID()
		, frame->getIndex()
		, itemName
	);
}

static void saveImage(cv::Mat image, std::string pathName)
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
		catch(fs::filesystem_error& er)
		{
			
		}
	}
	try
	{
		auto bSaved = cv::imwrite(pathName.c_str(), image);
	}
	catch(...)
	{
	
	}
}

#define TIME_STAMP std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()

template <class T>
using FUTURE_VECTOR = std::vector<TaskThreadPool::JobFuture<T>>;
#define WAIT_ALL(list) std::for_each(list.begin(), list.end(), [](auto &f) { f.wait(); });

struct COPY_REGION
{
	COPY_REGION(const FrameRef* frame, const QRect& srcRect, cv::Mat& targetMat, std::optional<std::string> ROIName, bool needSaving) : COPY_REGION(frame, qrect2cvrect(srcRect), targetMat, ROIName, needSaving) {}

	COPY_REGION(
		  const FrameRef* frame
		, const cv::Rect& srcRect
		, cv::Mat& targetMat
		, std::optional<std::string> ROIName = std::nullopt
		, bool needSaving = false)

		: _imgContainer(frame->getImageContainer())
		, _targetMat (targetMat)
		, _srcRequestedRect(srcRect)
		, _srcNorlmalizedRect(srcRect)
		, _bytesPerPixel(frame->getBitsPerPixel() / 8)
		, _bNeedSaving (needSaving)
	{
		auto const& srcWidth = _imgContainer->cols;
		auto const& srcHeight = _imgContainer->rows;

		auto const& regReqLeft = _srcRequestedRect.x;
		auto const& regReqTop = _srcRequestedRect.y;
		auto const& regReqRight = _srcRequestedRect.x + _srcRequestedRect.width;
		auto const& regReqBottom = _srcRequestedRect.y + _srcRequestedRect.height;

		// TODO : implement exception handler
		if (_srcRequestedRect.empty() || _srcRequestedRect.width == 0 || _srcRequestedRect.height == 0)
		{
			throw AlgorithmException(ALGORITHM_ERROR::ALGO_ROI_INVALID_RECT, "ROI rectangle is invalid. Batch input parameters init problem ?");
		}
		// todo : think about exceeding frame dimesions
		if (regReqLeft < 0
			|| regReqTop < 0
			|| regReqRight >srcWidth
			|| regReqBottom > srcHeight
			)
		{
			//throw AlgorithmException(ALGORITHM_ERROR::ALGO_ROI_RECT_EXCEEDS_FRAME_RECT, "ROI rectangle limits exceed frame dimensions");
		}

		// fix actual target rectangle to stay within frame limits
		_srcNorlmalizedRect.x = ((std::max)(regReqLeft, 0));
		_srcNorlmalizedRect.y = ((std::max)(regReqTop, 0));

		const auto& normRight = ((std::min)(regReqRight, srcWidth));
		const auto& normBottom = ((std::min)(regReqBottom, srcHeight));

		_srcNorlmalizedRect.width = normRight - _srcNorlmalizedRect.x;
		_srcNorlmalizedRect.height = normBottom - _srcNorlmalizedRect.y;

		// if ROI image needs to be saved, 
		if (_bNeedSaving )
		{
			if (ROIName == std::nullopt)
			{
				throw AlgorithmException(ALGORITHM_ERROR::ALGO_EMPTY_ROI_NAME_TO_SAVE, "ROI requested for saving, but no name provided");
			} 

			//generate saving path
			_savingPath = getSavePath(frame, ROIName.value());
		}
	}


	static cv::Rect qrect2cvrect(const QRect& rcSrc)
	{
		return cv::Rect(rcSrc.left(), rcSrc.top(), rcSrc.width(), rcSrc.height());
	}

	static void makeCopy(COPY_REGION& rgn)
	{
		rgn._targetMat = std::move((*rgn._imgContainer)(rgn._srcNorlmalizedRect));  // .copyTo(rgn._targetMat);
		if (rgn._bNeedSaving && !rgn._savingPath.empty() )
		{
#if ALGO_PAR == 1
			TaskThreadPools::postJob(TaskThreadPools::diskDumperThreadPool(), saveImage, rgn._targetMat.clone(), rgn._savingPath);
#else
			saveImage (rgn._targetMat.clone(), rgn._savingPath);
#endif
		}
	}

	std::shared_ptr<cv::Mat>	_imgContainer;
	cv::Mat&					_targetMat;
	cv::Rect					_srcRequestedRect;
	cv::Rect					_srcNorlmalizedRect;
	int32_t						_bytesPerPixel;
	bool						_bNeedSaving;
	std::string					_savingPath;
};

using COPY_REGION_LIST = std::vector<COPY_REGION>;


#define ALGO_SCOPED_LOG PRINT_INFO7 << "[ALGO] : "
#define ALGO_SCOPED_ERROR PRINT_ERROR << "[ALGO : "
#define ALGO_SCOPED_WARNING PRINT_WARNING << "[ALGO] : "


/////////////////////////////////////////////////////
///         COMMON ALGORITHM FUNCTIONS

void LandaJune::Algorithms::initAlgorithmsData(std::shared_ptr<ProcessParameter> iputParams)
{
	const INIT_PARAMETER edgeInitParam{ ROIRect(iputParams->LeftStripRect()) };
	initEdge(edgeInitParam);

	const INIT_PARAMETER i2sInitParam{ ROIRect(iputParams->I2SApproximateTriangleRectLeft()) };
	initI2S(i2sInitParam);

	if (!iputParams->C2CROIArrayLeft().empty())
	{
		const INIT_PARAMETER c2croiInitParam{ ROIRect(iputParams->C2CROIArrayLeft()[0]) };
		initC2CRoi(c2croiInitParam);
	}
	else
	{
		initC2CRoi(INIT_PARAMETER{});
	}
	initWave(INIT_PARAMETER{});
}


void LandaJune::Algorithms::fillProcessParameters(const FrameRef* frame, PARAMS_C2C_SHEET_INPUT& input)
{
	input.setGenerateOverlay(frame->getProcessParams()->GenerateOverlays());
	input._stripInputParamLeft._paperEdgeInput._approxDistanceFromEdgeX = frame->getProcessParams()->EdgeApproximateDistanceX_px();
	input._stripInputParamLeft._paperEdgeInput._triangeApproximateY = frame->getProcessParams()->I2SOffsetFromPaperEdgeY_mm();
	input._stripInputParamLeft.setPixel2MM_X(frame->getProcessParams()->Pixel2MM_X());
	input._stripInputParamLeft.setPixel2MM_Y(frame->getProcessParams()->Pixel2MM_Y());

}

void LandaJune::Algorithms::generateRegions(const FrameRef* frame, PARAMS_C2C_SHEET_INPUT& input)
{
	const auto processParameters = frame->getProcessParams();
	COPY_REGION_LIST _regionsToCopy;

	// left strip
	_regionsToCopy.emplace_back(
		COPY_REGION
		{ frame, processParameters->LeftStripRect(), input._stripInputParamLeft._paperEdgeInput._stripImageSource, "strip_[LEFT]", true }
	);

	// right strip
	_regionsToCopy.emplace_back(
		COPY_REGION
		{ frame, processParameters->RightStripRect(), input._stripInputParamRight._paperEdgeInput._stripImageSource, "strip_[RIGHT]", true }
	);


	// I2S
	input._stripInputParamLeft._i2sInput._approxTriangeROI = processParameters->I2SApproximateTriangleRectLeft();
	input._stripInputParamRight._i2sInput._approxTriangeROI = processParameters->I2SApproximateTriangleRectRight();

	// I2S Left
	_regionsToCopy.emplace_back(
		COPY_REGION
		{ frame,processParameters->I2SApproximateTriangleRectLeft(), input._stripInputParamLeft._i2sInput._triangleImageSource, "I2S_[LEFT]",  true }
	);

	// I2S Right
	_regionsToCopy.emplace_back(
		COPY_REGION
		{ frame,processParameters->I2SApproximateTriangleRectRight(), input._stripInputParamRight._i2sInput._triangleImageSource, "I2S_[RIGHT]", true }
	);

	// ROIs
	if (processParameters->C2CROISetsCount() != 0)
	{
		// TODO : translate colors from batch parameters to real HSVs

		std::vector<HSV> hsv;

		for (auto i = 0; i < processParameters->ColorArray().size(); i++)
		{
			hsv.emplace_back(HSV{ processParameters->ColorArray()[i] });
		}

		for (auto i = 0; i < processParameters->C2CROISetsCount(); i++)
		{
			input._stripInputParamLeft._c2cROIInputs.emplace_back(
					PARAMS_C2C_ROI_INPUT
						{ frame, LEFT, hsv , ROIRect(processParameters->C2CROIArrayLeft()[i]), i }
			);

			input._stripInputParamRight._c2cROIInputs.emplace_back(
				PARAMS_C2C_ROI_INPUT
				{ frame, RIGHT, hsv , ROIRect(processParameters->C2CROIArrayRight()[i]), i }
			);
		}

		for ( auto i = 0; i < processParameters->C2CROISetsCount(); i++)
		{
			auto& leftROI = input._stripInputParamLeft._c2cROIInputs[i];
			auto& RightROI = input._stripInputParamRight._c2cROIInputs[i];

			_regionsToCopy.emplace_back(
				COPY_REGION
				{ frame
				  , static_cast<QRect>(leftROI._ROI)
				  , leftROI._ROIImageSource
				  , fmt::format("C2C_[LEFT]_#{0}", i)
				  , true }
			);

			_regionsToCopy.emplace_back(
				COPY_REGION
				{ frame
				  , static_cast<QRect>(RightROI._ROI)
				  , RightROI._ROIImageSource
				  , fmt::format("C2C_[RIGHT]_#{0}", i)
				  , true }
			);
		}
	}
#if ALGO_PAR == 1
	FUTURE_VECTOR<void> futureCopyRegionsList;
	auto& pool = TaskThreadPools::algorithmsThreadPool();
	// run first 1 - n ROIs on thread pool
	std::for_each(_regionsToCopy.begin() + 1, _regionsToCopy.end(),
		[&futureCopyRegionsList, &pool](auto &in)
	{
		futureCopyRegionsList.emplace_back(TaskThreadPools::postJob(pool, COPY_REGION::makeCopy, std::ref(in)));
	}
	);
	
	// run first copy regions on this thread
	COPY_REGION::makeCopy(_regionsToCopy.front());
	// wait for previous ROIs to complete
	WAIT_ALL(futureCopyRegionsList);
#else
	std::for_each(_regionsToCopy.begin(), _regionsToCopy.end(), COPY_REGION::makeCopy);
#endif
}

PARAMS_C2C_SHEET_OUTPUT LandaJune::Algorithms::calculateAll(const FrameRef* frame)
{
	PARAMS_C2C_SHEET_INPUT input(frame);
	fillProcessParameters(frame, input);
	generateRegions(frame, input);
	return calculateSheet(input);
}

void LandaJune::Algorithms::clearAlgorithmsData()
{
	shutdownEdge();
	shutdownI2S();
	shutdownC2CRoi();
	shutdownWave();
}



///////////////////////////////////////////////////
////////////// WHOLE SHEET  FUNCTION //////////////
///////////////////////////////////////////////////

PARAMS_C2C_SHEET_OUTPUT LandaJune::Algorithms::calculateSheet(const PARAMS_C2C_SHEET_INPUT& sheetInput)
{
	PARAMS_C2C_SHEET_OUTPUT retVal;
	retVal._input = sheetInput;
	retVal._result = ALG_STATUS_SUCCESS;

#if ALGO_PAR == 1
	/////////////////////////////////////
	/// calculate strip regioons in parallel

	// get algorithm tasks pool
	auto& pool = TaskThreadPools::algorithmsThreadPool();

	// post right strip function to the thread pool
	auto& rightStripFuture = TaskThreadPools::postJob(pool, calculateStrip, std::ref(sheetInput._stripInputParamRight), false);

	// run left strip calculations directly in current thread and get outputs
	retVal._stripOutputParameterLeft = calculateStrip(sheetInput._stripInputParamLeft, true);

	// get outputs from futures
	retVal._stripOutputParameterRight = rightStripFuture.get();
#else
	/////////////////////////////////////
	/// calculate strip regioons sequentially

	retVal._stripOutputParameterLeft = calculateStrip(sheetInput._stripInputParamLeft, true);
	retVal._stripOutputParameterRight = calculateStrip(sheetInput._stripInputParamRight, false);
#endif
	return retVal;
}

///////////////////////////////////////////////////
/////////////// SIDE STRIP FUNCTION ///////////////
///////////////////////////////////////////////////

PARAMS_C2C_STRIP_OUTPUT LandaJune::Algorithms::calculateStrip(const PARAMS_C2C_STRIP_INPUT& stripInput, bool detectEdge)
{
	PARAMS_C2C_STRIP_OUTPUT retVal;
	retVal._input = stripInput;

#if ALGO_PAR == 1
	/////////////////////////////////////
	/// calculate I2S and C2C regions in parallel
	// get algorithm tasks pool

	// post edge detection function to the thread pool
	if (detectEdge)
	{
		auto& edgeFuture = TaskThreadPools::postJob(TaskThreadPools::algorithmsThreadPool(), calculateEdge, std::ref(stripInput._paperEdgeInput));
	}

	/// calculate I2S directly
	retVal._i2sOutput = calculateI2S(stripInput._i2sInput);

	// get output from pooled function
	retVal._paperEdgeOutput = edgeFuture.get();
#else
	if (detectEdge)
	{
		retVal._paperEdgeOutput = calculateEdge(stripInput._paperEdgeInput);
	}
	retVal._i2sOutput = calculateI2S(stripInput._i2sInput);
#endif

	/////////////////////////////////////
	/// calculate ROIs in parallel

	auto& roiInputs = stripInput._c2cROIInputs;
	if (roiInputs.empty())
	{
		ALGO_SCOPED_WARNING << "No C2C ROI defined in input parameters.";
		return retVal;
	}

#if ALGO_PAR == 1	

	// prepare vector of futures
	FUTURE_VECTOR<PARAMS_C2C_ROI_OUTPUT> _futureROIList;

	// run first 1 - n ROIs on thread pool
	std::for_each(roiInputs.begin() + 1, roiInputs.end(),
		[&_futureROIList](auto &in)
	{
		_futureROIList.push_back(TaskThreadPools::postJob(TaskThreadPools::algorithmsThreadPool(), calculateC2CRoi, std::ref(in)));
	}
	);

	// calculate first ROI on this thread
	retVal._c2cROIOutputs.push_back(calculateC2CRoi(roiInputs.front()));

	// wait for previous ROIs to complete
	WAIT_ALL(_futureROIList);

	// now get results from futures
	for (auto& i : _futureROIList)
		retVal._c2cROIOutputs.push_back(i.get());
#else
	std::for_each(
		roiInputs.begin(),  // start from first element
		roiInputs.end(),	// end element
		[&retVal](auto &in) // lambda function which calculates ROI and adds output to a vector of outputs
	{
		retVal._c2cROIOutputs.push_back(calculateC2CRoi(in));
	}
	);

#endif
	return retVal;
}



///////////////////////////////////////////////////
//////////////// PAPER EDGE FUNCTION //////////////
///////////////////////////////////////////////////

void LandaJune::Algorithms::initEdge(const INIT_PARAMETER& initParam)
{
	try
	{
		detect_edge_init(initParam);
	}
	catch (...)
	{
		ALGO_SCOPED_ERROR << "Function detect_edge_init has thrown exception";
	}
}

PARAMS_PAPEREDGE_OUTPUT LandaJune::Algorithms::calculateEdge(const PARAMS_PAPEREDGE_INPUT& input)
{
	PARAMS_PAPEREDGE_OUTPUT retVal;
	retVal._input = input;
	PRINT_INFO7 << "C2C_EDGE [side " << input._side << "] runs on thread #" << GetCurrentThreadId();

	try
	{
		detect_edge(input, retVal);
	}
	catch(...)
	{
		ALGO_SCOPED_ERROR << "Function detect_edge has thrown exception";
		retVal._outStatus = ALG_STATUS_FAILED;
	}
	
	if ( input.GenerateOverlay() )
	{
		auto savePath = getSavePath(input._frame, fmt::format("edge_overlay_{0}.bmp", input._side));
#if ALGO_PAR == 1
		TaskThreadPools::postJob(TaskThreadPools::diskDumperThreadPool(), saveImage, retVal._edgeOverlay.clone(), savePath);
#else
		saveImage(retVal._edgeOverlay.clone(), savePath);
#endif
	}
	return std::move(retVal);
}


void LandaJune::Algorithms::shutdownEdge()
{
	try
	{
		detect_edge_shutdown();
	}
	catch (...)
	{
		ALGO_SCOPED_ERROR << "Function detect_edge_shutdown has thrown exception";
	}
}


///////////////////////////////////////////////////
////////////////// I2S FUNCTION ///////////////////
///////////////////////////////////////////////////

void LandaJune::Algorithms::initI2S(const INIT_PARAMETER& initParam)
{
	try
	{
		detect_i2s_init(initParam);
	}
	catch (...)
	{
		ALGO_SCOPED_ERROR << "Function detect_i2s_init has thrown exception";
	}
}

PARAMS_I2S_OUTPUT LandaJune::Algorithms::calculateI2S(const PARAMS_I2S_INPUT& input)
{
	PARAMS_I2S_OUTPUT retVal;
	retVal._input = input;

	try
	{
		detect_i2s(input, retVal);
	}
	catch (...)
	{
		ALGO_SCOPED_ERROR << "Function detect_i2s has thrown exception";
		retVal._outStatus = ALG_STATUS_FAILED;
	}

	if (input.GenerateOverlay())
	{
		auto savePath = getSavePath(input._frame, fmt::format("i2s_overlay_{0}.bmp", input._side));
#if ALGO_PAR == 1
		TaskThreadPools::postJob(TaskThreadPools::diskDumperThreadPool(), saveImage, retVal._triangleOverlay.clone(), savePath);
#else
		saveImage(retVal._triangleOverlay.clone(), savePath);
#endif
	}

	PRINT_INFO8 << "I2S [side " << input._side << "] runs on thread #" << GetCurrentThreadId();
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	return std::move(retVal);
}


void LandaJune::Algorithms::shutdownI2S()
{
	try
	{
		detect_i2s_shutdown();
	}
	catch (...)
	{
		ALGO_SCOPED_ERROR << "Function detect_i2s_shutdown has thrown exception";
	}
}


///////////////////////////////////////////////////
/////////////// C2C ONE ROI FUNCTION //////////////
///////////////////////////////////////////////////

void LandaJune::Algorithms::initC2CRoi(const INIT_PARAMETER& initParam)
{
	try
	{
		detect_c2c_roi_init(initParam);
	}
	catch (...)
	{
		ALGO_SCOPED_ERROR << "Function detect_c2c_roi_init has thrown exception";
	}
}

PARAMS_C2C_ROI_OUTPUT LandaJune::Algorithms::calculateC2CRoi(const PARAMS_C2C_ROI_INPUT& input)
{
	PARAMS_C2C_ROI_OUTPUT retVal;
	retVal._input = input;

	PRINT_INFO8 << "C2C_ROI [" << input._roiIndex << "] runs in thread #" << GetCurrentThreadId();
	try
	{
		detect_c2c_roi(input, retVal);
	}
	catch (...)
	{
		ALGO_SCOPED_ERROR << "Function detect_c2c_roi has thrown exception";
		retVal._outStatus = ALG_STATUS_FAILED;
	}

	if (input.GenerateOverlay())
	{
		int iIndex = 0;
		std::for_each(retVal._colorOverlays.begin(), retVal._colorOverlays.end()
			, [&input, &iIndex](auto overlay)
		{
			auto savePath = getSavePath(input._frame, fmt::format("c2c_overlay_{0}.bmp", iIndex));
#if ALGO_PAR == 1
			TaskThreadPools::postJob(TaskThreadPools::diskDumperThreadPool(), saveImage, retVal.overlay.clone(), savePath);
#else
			saveImage(overlay.clone(), savePath);
#endif
			++iIndex;
		}
		);
	}
	return retVal;
}


void LandaJune::Algorithms::shutdownC2CRoi()
{
	try
	{
		detect_c2c_roi_shutdown();
	}
	catch (...)
	{
		ALGO_SCOPED_ERROR << "Function detect_c2c_roi_shutdown has thrown exception";
	}
}

///////////////////////////////////////////////////
/////////////// C2C ONE ROI FUNCTION //////////////
///////////////////////////////////////////////////

void LandaJune::Algorithms::initWave(const INIT_PARAMETER& initParam)
{

}

PARAMS_WAVE_OUTPUT LandaJune::Algorithms::calculateWave(const PARAMS_WAVE_INPUT& input)
{
	PARAMS_WAVE_OUTPUT out;
	return out;
}

void LandaJune::Algorithms::shutdownWave()
{

}
