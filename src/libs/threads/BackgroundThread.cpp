#include "stdafx.h"
#include "BackgroundThread.h"

using namespace LandaJune::Threading;

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
				auto res = static_cast<int>(fut.get());
				if (	res > 0 && res < 50 )
				{
					pThis->stop();
				}
			}
		}
		// just for sanity, if thread function has missed the thrown exception
		catch (BaseException& e) 
		{
			auto wrapper = pThis->getErrorHandler();
			const auto handler = wrapper.get(); 
			handler(pThis->_coreObject, e);
		}
		catch (std::exception& ex) 
		{
			auto wrapper = pThis->getErrorHandler();
			const auto handler = wrapper.get();
			BaseException bex(CORE_ERROR{CORE_ERROR::ERR_PROVIDER_EXCEPTION, ex.what()}, __FILE__, __LINE__);
			handler(pThis->_coreObject, bex);
		}
		taskbg_ptr.reset();
	}

	auto taskExitFunc = pThis->getThreadExitFunction();
	auto & taskexit_ptr = taskExitFunc.get();

	if (taskexit_ptr.valid())
	{
		taskexit_ptr();
	}
	
	std::cout << "----------------------------------------------------------------" << std::endl
			  << "--------  >>> Thread \"" << pThis->_name << " [ID : " << GetCurrentThreadId() << " ] has exited" << std::endl
			  << "----------------------------------------------------------------" << std::endl;
}

void BackgroundThread::setThreadPriority(const std::thread::native_handle_type handle) const
{
	if (_priority == HIGH ) 
		SetThreadPriority(static_cast<HANDLE>(handle), THREAD_PRIORITY_ABOVE_NORMAL);
}
