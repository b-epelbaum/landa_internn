#include "frameRef.h"
#include <chrono>

#include "ProcessParameter.h"
#include <opencv2/imgcodecs.hpp>

using namespace LandaJune::Parameters;
using namespace LandaJune::Core;


FrameRef::GLOBAL_FRAME_DATA::GLOBAL_FRAME_DATA(std::shared_ptr<ProcessParameter> globalParams)
	: _cvImageFormat(globalParams->OpenCVImageFormat())
	, _params(globalParams)
{
}

bool FrameRef::GLOBAL_FRAME_DATA::operator == (const GLOBAL_FRAME_DATA& other) const
{
	return (
		_cvImageFormat == other._cvImageFormat
		&& _params == other._params);
}

bool FrameRef::GLOBAL_FRAME_DATA::isValid(FRAME_REF_ERROR& err) const
{
	err = FRAME_REF_ERROR::ERR_NO_ERROR;
	const auto allOk = (
		_params
		);
	if (allOk)
		return true;
	err = (!_params) ? FRAME_REF_ERROR::ERR_FRAME_INVALID_BATCH_PARAMS
		: FRAME_REF_ERROR::ERR_FRAME_INVALID_INIT_DATA;

	return false;
}



FrameRef::FrameRef(const GLOBAL_FRAME_DATA& frameData)
{
	FRAME_REF_ERROR err;
	if (!frameData.isValid(err))
	{
		throw FrameRefException(err, "Cannot initialize FrameRef object");
	}

	_imgCVFormat = frameData._cvImageFormat;
	_bitsPerPixel = frameData._params->ScanBitDepth();

	// TODO : think about padding
	_processParameters = std::move(frameData._params);
	_bInited = true;
}

void FrameRef::reset()
{
	_index = -1;
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