#include "frameRef.h"
#include <chrono>

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
	_bits = nullptr;
	_postDataFunc = nullptr;
	_paramsMap.clear();
}


const uint8_t* FrameRef::getBits() const
{
	return _bits;
}


uint8_t* FrameRef::getBits()
{
	return _bits;
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
	_bits = bits;
}