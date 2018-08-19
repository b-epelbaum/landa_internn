#include "FrameRefPool.h"
#include "frameRef.h"

using namespace LandaJune::Core;


FrameRefPool & LandaJune::Core::frameRefPool()
{
	static FrameRefPool __fpool;
	return __fpool;
}



void FrameRefPool::reset(const uint64_t qlen)
{
	clear();
	init(qlen);
}

void FrameRefPool::clear()
{
	autolock lock(_mutex);
	_framesFree.clear();
	_frameUsed.clear();
}

void FrameRefPool::init(const uint64_t size)
{
	autolock lock(_mutex);
	static auto frameDeleter = [](FrameRef *p) {delete p; };
	for (uint64_t i = 0; i < size; ++i)
	{
		_framesFree.push_back(std::make_unique<FrameRef>());
	}
	_size = size;
}

uint64_t FrameRefPool::size()
{
	autolock lock(_mutex);
	return _size;
}

std::unique_ptr<FrameRef> FrameRefPool::pullFirstFree()
{
	autolock lock(_mutex);
	if (_framesFree.empty()) 
	{
		return std::unique_ptr<FrameRef>();
	}
	auto frame = std::move(_framesFree.front());
	_framesFree.pop_front(); 
	return frame;
}

void FrameRefPool::release(std::unique_ptr<FrameRef> && frame)
{
	autolock lock(_mutex);
	frame->reset();
	_framesFree.push_back(std::move(frame));
}

std::unique_ptr<FrameRef> FrameRefPool::pullFirstLoaded()
{
	autolock lock(_mutex);
	if (_frameUsed.empty()) 
	{
		return std::unique_ptr<FrameRef>();
	}
	auto frame = std::move(_frameUsed.front());
	_frameUsed.pop_front(); return frame;
}

void FrameRefPool::pushNextLoaded(std::unique_ptr<FrameRef> && frame)
{
	autolock lock(_mutex);
	_frameUsed.push_back(std::move(frame));
}
