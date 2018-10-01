#include "stdafx.h"
#include "writequeue.h"
#include "RealTimeStats.h"


using QueueType = LandaJune::Core::NativeThreadQueue<LandaJune::Core::shared_char_vector, std::string>;
using ThreadType = LandaJune::Core::NativeThread<LandaJune::Core::shared_char_vector, std::string>;

QueueType	__dumpQueue;
ThreadType	__dumpThread;


QueueType& LandaJune::Core::fileDumpThreadQueue()
{
	return __dumpQueue;
}

ThreadType& LandaJune::Core::fileDumpThread()
{
	if (!__dumpThread.getQueue()) 
	{
		auto & q = fileDumpThreadQueue();
		__dumpThread.setQueue(&q);
	}
	return __dumpThread;
}

bool LandaJune::Core::dumpThreadPostJob(shared_char_vector img, std::string path, bool postAsync )
{
	if ( postAsync )
	{
		const auto currentQueueSize = fileDumpThreadQueueSize();
		if ( img->size() + currentQueueSize > __dumpThread.getMaxQueueSize() )
			return false;

		
		Helpers::RealTimeStats::rtStats()->increment(
			Helpers::RealTimeStats::objects_saveQueueLength, 1, currentQueueSize);

		
		postJob(fileDumpThreadQueue(), img, path);
	}
	else
	{
		const auto func = __dumpThread.getThreadFunction();
		auto paramTuple = std::make_tuple<std::shared_ptr<std::vector<unsigned char>>, std::string>(std::move(img), std::move(path));
		func(paramTuple);
	}
	return true;
}

uint64_t LandaJune::Core::fileDumpThreadQueueSize() 
{
	uint64_t counter = 0;
	auto f = [&counter](const QueueType::arg & v) 
	{
		const auto & ref = std::get<0>(v);
		counter += ref->size();
	};
	__dumpQueue.enumerate(f);
	return counter;
}
