#include "stdafx.h"
#include "writequeue.h"

LandaJune::Core::NativeThreadQueue<LandaJune::Core::shared_char_vector, std::string> __dumpQueue;
LandaJune::Core::NativeThread<LandaJune::Core::shared_char_vector, std::string> __dumpThread;

LandaJune::Core::NativeThreadQueue<std::shared_ptr<std::vector<unsigned char>>, std::basic_string<char>>& LandaJune::
Core::fileDumpThreadQueue()
{
	return __dumpQueue;
}

LandaJune::Core::NativeThread<std::shared_ptr<std::vector<unsigned char>>, std::basic_string<char>>& LandaJune::Core::
fileDumpThread()
{
	if (!__dumpThread.getQueue()) 
	{
		auto & q = fileDumpThreadQueue();
		__dumpThread.setQueue(&q);
	}
	return __dumpThread;
}

void LandaJune::Core::dumpThreadPostJob(shared_char_vector img, std::string path)
{
	postJob(fileDumpThreadQueue(), img, path);
}
