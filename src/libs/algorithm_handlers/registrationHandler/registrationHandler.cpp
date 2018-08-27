#include "registrationHandler.h"
#include "frameRef.h"
#include "functions.h"
#include "ProcessParameter.h"
#include "include/format.h"

////////////////////////////////
// algorithmic functions
#include "algo_edge_impl.h"
#include "algo_i2s_impl.h"
#include "algo_c2c_roi_impl.h"


///////////////
//  OPEN CV
#include <opencv/cv.h> 
#include <opencv2/imgcodecs.hpp>
#include "applog.h"
#include "TaskThreadPool.h"
#include <opencv2/imgcodecs/imgcodecs_c.h>
#include <opencv2/highgui.hpp>

#ifdef _DEBUG
#pragma comment(lib, "opencv_world342d.lib")
#else
#pragma comment(lib, "opencv_world342.lib")
#endif


using namespace LandaJune;
using namespace Algorithms;
using namespace Parameters;
using namespace Threading;
using namespace Core;

static const QString REGISTRATION_HANDLER_NAME = "Offline Reader Registration Handler";
static const QString REGISTRATION_HANDLER_DESC = "Processes offline registration images";

const std::string DEFAULT_OUT_FOLDER = "c:\\temp\\june_out";


#define REGISTRATION_HANDLER_SCOPED_LOG PRINT_INFO5 << "[registrationPageHandler] : "
#define REGISTRATION_HANDLER_SCOPED_ERROR PRINT_ERROR << "[registrationPageHandler] : "
#define REGISTRATION_HANDLER_SCOPED_WARNING PRINT_WARNING << "[registrationPageHandler] : "

//////////////////////////////////////////////////
////////////  HELPER FUNCTIONS
//////////////////////////////////////////////////


APOINT toAPoint(const QPoint& qpt)
{
	APOINT out;
	out._x = qpt.x();
	out._y = qpt.y();
	return std::move(out);

}
ASIZE toASize(const QSize& qsz)
{
	ASIZE out;
	out._width = qsz.width();
	out._height = qsz.height();
	return std::move(out);
}

ROIRect toROIRect(const QRect& qrc)
{
	ROIRect out;
	out._pt = toAPoint(qrc.topLeft());
	out._size = toASize(qrc.size());
	return std::move(out);
}

HSV_SINGLE colorSingle2HSVSingle(const COLOR_TRIPLET_SINGLE& color)
{
	HSV_SINGLE out;
	out._iH = color._iH;
	out._iS = color._iS;
	out._iV = color._iV;
	return std::move(out);
}

HSV color2HSV (const COLOR_TRIPLET& color)
{
	HSV out;
	out._min = colorSingle2HSVSingle(color._min);
	out._max = colorSingle2HSVSingle(color._max);
	return std::move(out);
}


cv::Rect qrect2cvrect(const QRect& rcSrc)
{
	return cv::Rect(rcSrc.left(), rcSrc.top(), rcSrc.width(), rcSrc.height());
}

cv::Rect roirect2cvrect(const ROIRect& rcSrc)
{
	return cv::Rect(rcSrc.left(), rcSrc.top(), rcSrc.width(), rcSrc.height());
}



///////////////////////////////////////////////////
/////////////  REGIONS
//////////////////////////////////////////////////

struct CV_COPY_REGION
{
	const cv::Mat&				_srcMatContainer;
	cv::Mat&					_targetMatContainer;
	cv::Rect					_srcRequestedRect;
	cv::Rect					_srcNormalizedRect;
	bool						_bNeedSaving;
	std::string					_fullSavePath;
	std::shared_ptr<ProcessParameter> _params;
	bool						_bParallelize = false;

	CV_COPY_REGION(
		  const cv::Mat&	srcMat							// source data
		, cv::Mat&			targetMat						// target MAT objects
		, std::shared_ptr<ProcessParameter> params
		, const cv::Rect&	srcRect							// rectangle of the source data to copy
		, int				frameIndex
		, const std::string& ROIName						// ROI Name
		, const bool		needSaving = false				// if should be dumped to disk as well
	)
		: _srcMatContainer(srcMat)
		, _targetMatContainer(targetMat)
		, _srcRequestedRect(srcRect)
		, _srcNormalizedRect(srcRect)
		, _bNeedSaving(needSaving)
	{
		_bParallelize = params->ParalellizeCalculations();
		_srcNormalizedRect = normalizeRegionRect();
		// if ROI image needs to be saved, 
		if (_bNeedSaving)
		{
			if (ROIName.empty())
			{
				throw AlgorithmException(ALGORITHM_ERROR::ALGO_EMPTY_ROI_NAME_TO_SAVE, "ROI requested for saving, but no name provided");
			}

			//generate saving path
			_fullSavePath = getSavePath(params, ROIName, frameIndex);
		}
	}

	static std::string getSavePath(std::shared_ptr<ProcessParameter> params, const std::string& itemName, int frameIndex)
	{
		auto rootPath = params->RootOutputFolder().toStdString();
		if (rootPath.empty())
		{
			rootPath = DEFAULT_OUT_FOLDER;
		}
		return fmt::format("{0}\\{1}\\frame_#{2}\\{3}.bmp"
			, rootPath
			, params->JobID()
			, frameIndex
			, itemName
		);
	}


	cv::Rect normalizeRegionRect () const
	{
		auto const& srcWidth = _srcMatContainer.cols;
		auto const& srcHeight = _srcMatContainer.rows;

		auto const& regReqLeft = _srcRequestedRect.x;
		auto const& regReqTop = _srcRequestedRect.y;
		auto const& regReqRight = _srcRequestedRect.x + _srcRequestedRect.width;
		auto const& regReqBottom = _srcRequestedRect.y + _srcRequestedRect.height;

		// TODO : implement exception handler
		if (_srcRequestedRect.empty() || _srcRequestedRect.width == 0 || _srcRequestedRect.height == 0)
		{
			throw AlgorithmException(ALGORITHM_ERROR::ALGO_ROI_INVALID_RECT, "ROI rectangle is invalid. Batch input parameters init problem ?");
		}
		// todo : think about exceeding frame dimensions
		if (   regReqLeft < 0
			|| regReqTop < 0
			|| regReqLeft >= srcWidth
			|| regReqTop >= srcHeight
			)
		{
			throw AlgorithmException(ALGORITHM_ERROR::ALGO_ROI_RECT_EXCEEDS_FRAME_RECT, "ROI rectangle limits exceed frame dimensions");
		}

		if (  regReqRight >srcWidth
			|| regReqBottom > srcHeight
			)
		{
			REGISTRATION_HANDLER_SCOPED_WARNING << "ROI boundaries exceed frame dimensions. ROI will be truncated";
		}


		cv::Rect retValRect;
		// fix actual target rectangle to stay within frame limits
		retValRect.x = ((std::max)(regReqLeft, 0));
		retValRect.y = ((std::max)(regReqTop, 0));

		const auto& normRight = ((std::min)(regReqRight, srcWidth));
		const auto& normBottom = ((std::min)(regReqBottom, srcHeight));

		retValRect.width = normRight - _srcNormalizedRect.x;
		retValRect.height = normBottom - retValRect.y;

		return retValRect;
	}

	static void performCopy(CV_COPY_REGION& rgn)
	{
		// create a new MAT object by making a deep copy from the source MAT
		rgn._targetMatContainer = std::move((rgn._srcMatContainer)(rgn._srcNormalizedRect));
		if (rgn._bNeedSaving && !rgn._fullSavePath.empty())
		{
			if (rgn._bParallelize)
			{
				TaskThreadPools::postJob(TaskThreadPools::diskDumperThreadPool(), Functions::frameSaveImage, rgn._targetMatContainer.clone(), rgn._fullSavePath);
			}
			else
			{
				Functions::frameSaveImage(rgn._targetMatContainer.clone(), rgn._fullSavePath);
			}
		}
	}
};

using CV_COPY_REGION_LIST = std::vector<CV_COPY_REGION>;


registrationPageHandler::registrationPageHandler()
{
	REGISTRATION_HANDLER_SCOPED_LOG << "created";
}

registrationPageHandler::~registrationPageHandler()
{
	REGISTRATION_HANDLER_SCOPED_LOG << "destroyed";
}

std::unique_ptr<IAlgorithmHandler> registrationPageHandler::clone()
{
	auto retVal = std::make_unique<registrationPageHandler>();
	retVal->_processParameters = _processParameters;
	return std::move(retVal);
}

QString registrationPageHandler::getName() const
{
	return REGISTRATION_HANDLER_NAME;
}

QString registrationPageHandler::getDescription() const
{
	return REGISTRATION_HANDLER_DESC;
}

std::shared_ptr<BaseParameter> registrationPageHandler::getParameters() const
{
	return std::static_pointer_cast<BaseParameter>(_processParameters);
}

void registrationPageHandler::init(std::shared_ptr<BaseParameter> parameters)
{
	validateProcessParameters(parameters);
	// template image ( temporary solution )
	QFile templateTif(":/templates/Resources/Template1.tif");
	
	if (templateTif.open(QFile::ReadOnly))
		_processParameters->setCircleTemplateBuffer(templateTif.readAll());

	const INIT_PARAMETER edgeInitParam{ toROIRect(_processParameters->LeftStripRect()) };
	initEdge(edgeInitParam);

	const INIT_PARAMETER i2sInitParam{ toROIRect(_processParameters->I2SApproximateTriangleRectLeft()) };
	initI2S(i2sInitParam);

	if (!_processParameters->C2CROIArrayLeft().empty())
	{
		const INIT_PARAMETER c2croiInitParam{ toROIRect(_processParameters->C2CROIArrayLeft()[0]) };
		initC2CRoi(c2croiInitParam);
	}
	else
	{
		REGISTRATION_HANDLER_SCOPED_WARNING << "No C2C ROI defined !";
	}
}

void registrationPageHandler::cleanup()
{
	shutdownEdge();
	shutdownI2S();
	shutdownC2CRoi();
}

void registrationPageHandler::validateProcessParameters(std::shared_ptr<BaseParameter> parameters)
{
	_processParameters = std::dynamic_pointer_cast<ProcessParameter>(parameters);
}

void registrationPageHandler::constructFrameContainer(const FrameRef* frame, int bitsPerPixel)
{
	_frameContainer = std::make_unique<cv::Mat>(frame->getHeight(), frame->getWidth(),
	                                            CV_MAKETYPE(CV_8U, bitsPerPixel / 8), (void*)frame->getBits());
}



void registrationPageHandler::fillProcessParameters(const FrameRef* frame, PARAMS_C2C_STRIP_INPUT& input)
{
	_bParallelizeCalculations = _processParameters->ParalellizeCalculations();
	_frameIndex = frame->getIndex();

	try
	{
		_sourceFrameImageName = std::any_cast<std::string>(frame->getNamedParameter("srcPath"));
	}
	catch (const std::bad_any_cast& e)
	{
		REGISTRATION_HANDLER_SCOPED_ERROR << "Cannot retrieve source image file name. Error : " << e.what() << "; Resetting to default...";
		_sourceFrameImageName = fmt::format("source_frame_#{0}", _frameIndex);
	}

	// TODO : think about more effective way of common parameters propagation. Remove inheritance ? weak_ptr to parent class
	const auto& bGenerateOverlays = _processParameters->GenerateOverlays();

	input.setGenerateOverlay(bGenerateOverlays);
	input.setGenerateOverlay(bGenerateOverlays);
	input._i2sInput.setGenerateOverlay(bGenerateOverlays);

	input._paperEdgeInput.setGenerateOverlay(bGenerateOverlays);

	input._paperEdgeInput._approxDistanceFromEdgeX = _processParameters->EdgeApproximateDistanceX_px();
	input._paperEdgeInput._triangeApproximateY = _processParameters->I2SOffsetFromPaperEdgeY_mm();
	input.setPixel2MM_X(_processParameters->Pixel2MM_X());
	input.setPixel2MM_Y(_processParameters->Pixel2MM_Y());
}

void registrationPageHandler::process(const FrameRef * frame)
{
	_frame = frame;
	constructFrameContainer(frame, _processParameters->ScanBitDepth());
	PARAMS_C2C_STRIP_INPUT input(_frame, LEFT);
	fillProcessParameters(_frame, input);
	generateRegions(input);
	auto output = processStrip(input, true);
}


void registrationPageHandler::generateRegions(PARAMS_C2C_STRIP_INPUT& input)
{
	
	CV_COPY_REGION_LIST _regionsToCopy;

	// add region of left strip
	_regionsToCopy.emplace_back(
		CV_COPY_REGION
		( 
			*_frameContainer
		  , input._paperEdgeInput._stripImageSource
		  , _processParameters
		  , qrect2cvrect(_processParameters->LeftStripRect())
		  , _frameIndex
		  , "strip_[LEFT]"
		  , _processParameters->DumpLeftStrip()
		)
	);
	
	// add region of I2S
	input._i2sInput._approxTriangeROI = toROIRect(_processParameters->I2SApproximateTriangleRectLeft());

	_regionsToCopy.emplace_back(
			CV_COPY_REGION
			(
				*_frameContainer
			  , input._i2sInput._triangleImageSource
			  , _processParameters
			  , qrect2cvrect(_processParameters->I2SApproximateTriangleRectLeft())
			  , _frameIndex
			  , "I2S_[LEFT]"
			  , _processParameters->DumpI2S()
			)
		);


	// ROIs
	if (_processParameters->C2CROISetsCount() != 0)
	{
		std::vector<HSV> hsv;
		for (auto i = 0; i < _processParameters->ColorArray().size(); i++)
		{
			hsv.emplace_back(color2HSV( _processParameters->ColorArray()[i]));
		}

		// crate array of PARAMS_C2C_ROI_INPUT objects accordingly to C2C ROIs number
		for (auto i = 0; i < _processParameters->C2CROISetsCount(); i++)
		{
			input._c2cROIInputs.emplace_back(
				PARAMS_C2C_ROI_INPUT
				(
					_frame
				  , LEFT
				  , hsv
				  , toROIRect(_processParameters->C2CROIArrayLeft()[i])
				  , i
				)
			);
		}

		// and create regions for every ROI set
		for (auto i = 0; i < _processParameters->C2CROISetsCount(); i++)
		{
			// array of LEFT C2C ROIs
			auto& leftROI = input._c2cROIInputs[i];
			leftROI.setGenerateOverlay(input.GenerateOverlay());
			_regionsToCopy.emplace_back(
				CV_COPY_REGION
				(
					*_frameContainer
					, leftROI._ROIImageSource
					, _processParameters
					, roirect2cvrect(leftROI._ROI)
					, _frameIndex
					, fmt::format("C2C_[LEFT]_#{0}", i)
					, _processParameters->DumpC2CROIs()
				)
			);
		}
	}
	if (_bParallelizeCalculations)
	{
		FUTURE_VECTOR<void> futureCopyRegionsList;
		auto& pool = TaskThreadPools::algorithmsThreadPool();
		
		// run first 1 - n ROIs on thread pool
		std::for_each(
			_regionsToCopy.begin() + 1, 
			_regionsToCopy.end(),
			[&futureCopyRegionsList, &pool](auto &in)
			{
				futureCopyRegionsList.emplace_back(TaskThreadPools::postJob(pool, CV_COPY_REGION::performCopy, std::ref(in)));
			}
			);

		// run first copy regions on this thread
		CV_COPY_REGION::performCopy(_regionsToCopy.front());

		// wait for previous ROIs to complete
		WAIT_ALL(futureCopyRegionsList);
	}
	else
	{
		std::for_each(_regionsToCopy.begin(), _regionsToCopy.end(), CV_COPY_REGION::performCopy);
	}
}

///////////////////////////////////////////////////
/////////////// SIDE STRIP FUNCTION ///////////////
///////////////////////////////////////////////////

PARAMS_C2C_STRIP_OUTPUT registrationPageHandler::processStrip(const PARAMS_C2C_STRIP_INPUT& stripInput, const bool detectEdge)
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
			_futureEdgeOutputList.emplace_back(TaskThreadPools::postJob(TaskThreadPools::algorithmsThreadPool(), &registrationPageHandler::processEdge, this, std::ref(stripInput._paperEdgeInput)));
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
		REGISTRATION_HANDLER_SCOPED_LOG << "No C2C ROI defined in input parameters.";
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
			_futureROIList.emplace_back(TaskThreadPools::postJob(TaskThreadPools::algorithmsThreadPool(), &registrationPageHandler::processC2CROI, this, std::ref(in)));
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

void registrationPageHandler::initEdge(const INIT_PARAMETER& initParam) const
{
	try
	{
		detect_edge_init(initParam);
	}
	catch (...)
	{
		REGISTRATION_HANDLER_SCOPED_ERROR << "Function detect_edge_init has thrown exception";
	}
}

PARAMS_PAPEREDGE_OUTPUT registrationPageHandler::processEdge(const PARAMS_PAPEREDGE_INPUT& input)
{
	PARAMS_PAPEREDGE_OUTPUT retVal;
	retVal._input = input;
	REGISTRATION_HANDLER_SCOPED_LOG << "Edge detection [side " << input._side << "] runs on thread #" << GetCurrentThreadId();

	try
	{
		detect_edge(input, retVal);
	}
	catch (...)
	{
		REGISTRATION_HANDLER_SCOPED_ERROR << "Function detect_edge has thrown exception";
		retVal._result = ALG_STATUS_EXCEPTION_THROWN;
	}

	if (input.GenerateOverlay())
	{
		const auto& savePath = CV_COPY_REGION::getSavePath(_processParameters, fmt::format("edge_overlay_{0}.bmp", input._side), _frame->getIndex());
		if (_bParallelizeCalculations)
		{
			TaskThreadPools::postJob(TaskThreadPools::diskDumperThreadPool(), Functions::frameSaveImage, retVal._edgeOverlay.clone(), savePath);
		}
		else
		{
			Functions::frameSaveImage(retVal._edgeOverlay.clone(), savePath);
		}
	}

	return std::move(retVal);
}

void registrationPageHandler::shutdownEdge() const
{
	try
	{
		detect_edge_shutdown();
	}
	catch (...)
	{
		REGISTRATION_HANDLER_SCOPED_ERROR << "Function detect_edge_shutdown has thrown exception";
	}
}

///////////////////////////////////////////////////
////////////////// I2S FUNCTION ///////////////////
///////////////////////////////////////////////////

void registrationPageHandler::initI2S(const INIT_PARAMETER& initParam) const
{
	try
	{
		detect_i2s_init(initParam);
	}
	catch (...)
	{
		REGISTRATION_HANDLER_SCOPED_ERROR << "Function detect_i2s_init has thrown exception";
	}
}

PARAMS_I2S_OUTPUT registrationPageHandler::processI2S(const PARAMS_I2S_INPUT& input)
{
	PARAMS_I2S_OUTPUT retVal;
	retVal._input = input;

	REGISTRATION_HANDLER_SCOPED_LOG << "I2S detection [side " << input._side << "] runs on thread #" << GetCurrentThreadId();
	try
	{
		detect_i2s(input, retVal);
	}
	catch (...)
	{
		REGISTRATION_HANDLER_SCOPED_ERROR << "Function detect_i2s has thrown exception";
		retVal._result = ALG_STATUS_EXCEPTION_THROWN;
	}

	if (input.GenerateOverlay())
	{
		const auto& savePath = CV_COPY_REGION::getSavePath(_processParameters, fmt::format("I2S_overlay_{0}.bmp", input._side), _frame->getIndex());
		if (_bParallelizeCalculations)
		{
			TaskThreadPools::postJob(TaskThreadPools::diskDumperThreadPool(), Functions::frameSaveImage, retVal._triangleOverlay.clone(), savePath);
		}
		else
		{
			Functions::frameSaveImage(retVal._triangleOverlay.clone(), savePath);
		}
	}
	return std::move(retVal);
}

void registrationPageHandler::shutdownI2S() const
{
	try
	{
		detect_i2s_shutdown();
	}
	catch (...)
	{
		REGISTRATION_HANDLER_SCOPED_ERROR << "Function detect_i2s_shutdown has thrown exception";
	}
}


///////////////////////////////////////////////////
/////////////// C2C ONE ROI FUNCTION //////////////
///////////////////////////////////////////////////


void registrationPageHandler::initC2CRoi(const INIT_PARAMETER& initParam) const
{
	C2C_ROI_INIT_PARAMETER c2cInitParam(initParam);
	const auto& buf = _processParameters->CircleTemplateBuffer().constData();
	std::vector<char> data(buf, buf + _processParameters->CircleTemplateBuffer().size());
	c2cInitParam._templateImage = std::move(cv::imdecode(cv::Mat(data), CV_LOAD_IMAGE_GRAYSCALE));

	try
	{
		detect_c2c_roi_init(c2cInitParam);
	}
	catch (...)
	{
		REGISTRATION_HANDLER_SCOPED_ERROR << "Function detect_c2c_roi_init has thrown exception";
	}
}

PARAMS_C2C_ROI_OUTPUT registrationPageHandler::processC2CROI(const PARAMS_C2C_ROI_INPUT& input)
{
	PARAMS_C2C_ROI_OUTPUT retVal;
	retVal._input = input;

	REGISTRATION_HANDLER_SCOPED_LOG << "C2C Detection [side : " << input._side << "; index : " << input._roiIndex << "] runs in thread #" << GetCurrentThreadId();
	try
	{
		detect_c2c_roi(input, retVal);
	}
	catch (...)
	{
		REGISTRATION_HANDLER_SCOPED_ERROR << "Function detect_c2c_roi has thrown exception";
		retVal._result = ALG_STATUS_EXCEPTION_THROWN;
	}

	if (input.GenerateOverlay())
	{
		int iIndex = 0;
		std::for_each(retVal._colorOverlays.begin(), retVal._colorOverlays.end()
			, [&iIndex, &input, this](auto overlay)
		{
			const auto& savePath = CV_COPY_REGION::getSavePath(_processParameters, fmt::format("c2c_overlay_[{0}]_{1}.bmp", input._side, iIndex),  this->_frame->getIndex());
			if (this->_bParallelizeCalculations)
			{
				TaskThreadPools::postJob(TaskThreadPools::diskDumperThreadPool(), Functions::frameSaveImage, overlay.clone(), savePath);
			}
			else
			{
				Functions::frameSaveImage(overlay.clone(), savePath);
			}
			++iIndex;
		}
		);
	}
	return retVal;
}

void registrationPageHandler::shutdownC2CRoi() const
{
	try
	{
		detect_c2c_roi_shutdown();
	}
	catch (...)
	{
		REGISTRATION_HANDLER_SCOPED_ERROR << "Function detect_c2c_roi_shutdown has thrown exception";
	}
}