#include "frameRef.h"
#include <chrono>
#include <map>
#include <opencv2/imgcodecs.hpp>

using namespace LandaJune::Parameters;
using namespace LandaJune::Core;

using OPENCV_COLOR_MAP = std::map<int, int>;

static OPENCV_COLOR_MAP __colorMap = 
{
	  {CV_8U, 8}
	, {CV_16U, 16}
	, {CV_8UC3, 24}
	, {CV_32S, 32}
};

FrameRef::FrameRef(std::shared_ptr<ProcessParameter> processParams, int openCVImgFormat)
	: _imgCVFormat(openCVImgFormat)
	, _processParameters (processParams)
	
{
	const auto allOk = (
		processParams
		);

	if (!allOk)
	{
		throw FrameRefException(FRAME_REF_ERROR::ERR_FRAME_INVALID_INIT_DATA, "Cannot initialize FrameRef object : ProcessParameter value is invalid");
	}

	if ( const auto bitsPerPixel = __colorMap.find(openCVImgFormat); bitsPerPixel != __colorMap.end() )
	{
		_bitsPerPixel = bitsPerPixel->second;
	}

	_processParameters = processParams;
	_bInited = true;
}

void FrameRef::reset()
{
	_index = -1;
	if (_imgContainer)
		_imgContainer->release();
}

const uint8_t* FrameRef::getBits() const
{
	return static_cast<const uint8_t*>(_imgContainer->data);
}

uint8_t* FrameRef::getBits()
{
	return static_cast<uint8_t*>(_imgContainer->data);
}

void FrameRef::setBits(const int32_t imageIdx
	, const int32_t width
	, const int32_t height
	, const size_t receivedSize
	, uint8_t* bits)
{
	const auto& t = std::chrono::system_clock::now().time_since_epoch();
	_frameTimeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(t).count();
	_index = imageIdx;

	_frameWidth = width;
	_frameHeight = height;
	_sizeInBytes = receivedSize;
	_imgContainer = std::make_shared<cv::Mat>(_frameHeight, _frameWidth, _imgCVFormat, bits );
}