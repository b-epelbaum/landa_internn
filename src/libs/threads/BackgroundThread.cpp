#include "stdafx.h"
#include "BackgroundThread.h"
//#include "applog.h"

using namespace LandaJune::Threading;

//#define THREAD_SCOPED_LOG PRINT_INFO7 << " -- [BackgroundThread : " << _name.c_str() << "] : "
//#define THREAD_SCOPED_ERROR PRINT_ERROR << " -- [BackgroundThread : " << _name.c_str() << "] : "

BackgroundThread::BackgroundThread(std::string name, const int index, const THREAD_PRIORITY tP ) 
	: _error_handler(default_error_handler)
	, _priority(tP)
	, _name(std::move(name))
	, _index(index)
{
	
}

BackgroundThread::~BackgroundThread()
{
	stop();
	join();
}

bool BackgroundThread::start()
{
	autolock l(_mutex);
	if (_thread)
	{
		return false;
	}
	_thread = std::make_unique<std::thread>(threadFunction, this);
	setThreadPriority(_thread->native_handle());
	return !!_thread.get();
}

void BackgroundThread::stop()
{
	setState(THREAD_STATE::IDLE);
}

bool BackgroundThread::join()
{
	std::unique_ptr<std::thread> tempThread;
	{
		autolock l(_mutex);
		if (!_thread.get())
		{
			return false;
		}
		tempThread = std::move(_thread);
		_state = THREAD_STATE::IDLE;
	}
	if (tempThread->joinable()) 
	{
		tempThread->join();
	}
	return true;
}

void BackgroundThread::threadFunction(BackgroundThread *pThis)
{
	//PRINT_INFO7 << "[BackgroundThread: " << pThis->_name.c_str() << "; IDX: " << pThis->_index << "(" << GetCurrentThreadId() << ")] started on CPU #" << GetCurrentProcessorNumber();
	pThis->setState(THREAD_STATE::BUSY);

	// TODO : think about removing sleep
	
	static const std::chrono::milliseconds timeout(0);
	while (pThis->getState() == THREAD_STATE::BUSY)
	{
		std::this_thread::sleep_for(timeout);
		auto taskbg = pThis->getThreadFunction();
		auto & taskbg_ptr = taskbg.get();
		try 
		{
			if (taskbg_ptr.valid()) 
			{
				taskbg_ptr();
				auto fut = taskbg_ptr.get_future();
				fut.get();
			}
		} 
		catch (BaseException& e) 
		{
			auto wrapper = pThis->getErrorHandler();
			const auto handler = wrapper.get(); 
			handler(pThis->_userObject, e);
		}
		catch (std::runtime_error& re) 
		{
			auto wrapper = pThis->getErrorHandler();
			const auto handler = wrapper.get();
			BaseException ex(CORE_ERROR{CORE_ERROR::ERR_CORE_ALGO_RUNNER_THROWN_RUNTIME_EXCEPTION, re.what()}, __FILE__, __LINE__);
			handler(pThis->_userObject, ex);
		}
		taskbg_ptr.reset();
	}
}

void BackgroundThread::setThreadPriority(const std::thread::native_handle_type handle) const
{
	if (_priority == HIGH ) 
		SetThreadPriority(static_cast<HANDLE>(handle), THREAD_PRIORITY_ABOVE_NORMAL);
}
