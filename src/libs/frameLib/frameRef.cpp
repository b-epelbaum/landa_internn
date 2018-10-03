#include "frameRef.h"
#include <chrono>
#include <opencv2/imgcodecs.hpp>

using namespace LandaJune::Parameters;
using namespace LandaJune::Core;

FrameRef::FrameRef(uint64_t frameRefIndex) : _frameRefIndex(frameRefIndex)
{}


void FrameRef::reset()
{
	if (_postDataFunc)
	{
		_postDataFunc(this);
	}

	_index = -1;
	_sizeInBytes = 0;
	//_bits = nullptr;
	_postDataFunc = nullptr;
	_paramsMap.clear();
}


/*
const uint8_t* FrameRef::getBits() const
{
	return _bits;
}


uint8_t* FrameRef::getBits()
{
	return _bits;
}
*/

void FrameRef::setBits(const int32_t imageIdx
				, std::shared_ptr<cv::Mat> mat, bool bOfflineSource)
{
	const auto& t = std::chrono::system_clock::now().time_since_epoch();
	_frameTimeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(t).count();
	_index = imageIdx;
	_offlineSource = bOfflineSource;

	_frameWidth = mat->cols;
	_frameHeight = mat->rows;

	switch (mat->depth()) 
	{
		case CV_8UC3:	_bitsPerPixel = 24; break;
		case CV_8UC4:	_bitsPerPixel = 32; break;
		case CV_8UC1:	_bitsPerPixel = 8; break;
		default: _bitsPerPixel = 24;
	}
	_sizeInBytes = mat->step[0] * mat->rows;
	_img = mat;
}

void FrameRef::setBits(const int32_t imageIdx
	, const int32_t width
	, const int32_t height
	, const int32_t bitsPerPixel
	, const size_t receivedSize
	, uint8_t* bits
	, bool bOfflineSource)
{
	const auto& t = std::chrono::system_clock::now().time_since_epoch();
	_frameTimeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(t).count();
	_index = imageIdx;

	_frameWidth = width;
	_frameHeight = height;
	_bitsPerPixel = bitsPerPixel;
	_sizeInBytes = receivedSize;
	_offlineSource = bOfflineSource;

	_img = std::make_shared<cv::Mat>(_frameHeight, _frameWidth,
		CV_MAKETYPE(CV_8U, _bitsPerPixel / 8), static_cast<void*>(bits));
}