#include "ThreadPool.h"
#include "applog.h"

using namespace LandaJune::Core;

#define TPOOL_SCOPED_LOG PRINT_INFO4 << "[ThreadPool] : "
#define TPOOL_SCOPED_ERROR PRINT_ERROR << "[ThreadPool] : "

bool ThreadPool::start(const uint64_t len )
{
	TPOOL_SCOPED_LOG << "starting " << _alias.c_str() << " thread pool...";
	autolock l(_mutex);
	if (!_pool.empty())
	{
		TPOOL_SCOPED_ERROR << _alias.c_str() << " thread pool has no threads";
		return false;
	}
	size_t i, length = len;
	for (i = 0; i < length; ++i)
	{
		_pool.push_back(std::make_unique<Thread>(_alias, static_cast<int>(i), _priority));
	}
	length = _pool.size();
	for (i = 0; i < length; ++i)
	{
		_pool.at(i)->start();
	}
	return true;
}

void ThreadPool::stop()
{
	TPOOL_SCOPED_LOG << "stopping " << _alias.c_str() << " thread pool...";
	autolock l(_mutex);
	size_t i;
	const auto length = _pool.size();
	for (i = 0; i < length; ++i)
	{
		_pool.at(i)->stop();
	}
	for (i = 0; i < length; ++i)
	{
		_pool.at(i)->join();
	}
	_pool.clear();
	_next = 0;
	TPOOL_SCOPED_LOG << _alias.c_str() << " thread pool stopped";
}

void ThreadPool::setErrorHandler(std::function<void(std::exception &)> handler)
{
	autolock l(_mutex);
	const auto length = _pool.size();
	for (size_t i = 0; i < length; ++i)
	{
		_pool.at(i)->setErrorHandler(handler);
	}
}

ThreadPool&  LandaJune::Core::processingPool() 
{
	static ThreadPool __tpool(NORMAL, "Frame processor");
	return __tpool;
}

ThreadPool& LandaJune::Core::copyRegionsPool() 
{
	static ThreadPool __tpool(NORMAL, "Region copier");
	return __tpool;
}

ThreadPool& LandaJune::Core::saveBitmapPool() 
{
	static ThreadPool __tpool(NORMAL, "Image dumper");
	return __tpool;
}

Thread& LandaJune::Core::imageGenerationThread()
{
	static Thread __th("Image Producer", 100);
	return __th;
}

Thread& LandaJune::Core::imageProcessingThread()
{
	static Thread __th("Frame Processor", 200);
	return __th;
}


