#pragma once
#include <exception>
#include <queue>
#include <list>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>
#include <windows.h>
#include <synchapi.h>
#include "global.h"


namespace LandaJune
{
	namespace Core
	{
		enum class THREADS_EXPORT THREAD_STATE { IDLE = 0, START, BUSY };
		enum THREAD_PRIORITY { NORMAL = 0, HIGH };

		class THREADS_EXPORT NativeThreadAutoLock 
		{
			LPCRITICAL_SECTION _cs;
		public:
			NativeThreadAutoLock(NativeThreadAutoLock &&) = delete;
			NativeThreadAutoLock(const NativeThreadAutoLock &) = delete;
			explicit NativeThreadAutoLock(CRITICAL_SECTION &cs) : _cs(&cs) {
				EnterCriticalSection(_cs);
			}
			~NativeThreadAutoLock() {
				LeaveCriticalSection(_cs);
			}
			NativeThreadAutoLock operator = (NativeThreadAutoLock &&) = delete;
			const NativeThreadAutoLock & operator = (const NativeThreadAutoLock &) = delete;
		};

		template<typename... Args>
		class NativeThreadQueue {

		public:
			using arg = std::tuple<Args...>;

			NativeThreadQueue() {
				InitializeCriticalSection(&_cs);
				InitializeConditionVariable(&_cv);
			}
			NativeThreadQueue(const NativeThreadQueue &) = delete;
			NativeThreadQueue(NativeThreadQueue &&) = delete;
			const NativeThreadQueue & operator = (const NativeThreadQueue &) = delete;
			NativeThreadQueue & operator = (NativeThreadQueue &&) = delete;

			~NativeThreadQueue() {
				WakeAllConditionVariable(&_cv);
				LeaveCriticalSection(&_cs);
			}

			void push(const std::tuple<Args...> & v) {
				{
					NativeThreadAutoLock l(_cs);
					_queue.push_back(v);
				}
				WakeConditionVariable(&_cv);
			}

			bool next(std::tuple<Args...> & v) {
				NativeThreadAutoLock l(_cs);

				if (_queue.empty()) {
					SleepConditionVariableCS(&_cv, &_cs, 100);
				}
				if (_queue.empty()) {
					return false;
				}
				v = _queue.front();
				_queue.pop_front();
				return true;
			}

			bool empty(void) {
				NativeThreadAutoLock l(_cs);
				return _queue.empty();
			}

			size_t size()
			{
				NativeThreadAutoLock l(_cs);
				return _queue.size();
			}

			template <typename F>
			void enumerate(F & f) {
				NativeThreadAutoLock l(_cs);
				std::for_each(_queue.begin(), _queue.end(), f);
			}

		protected:
			CRITICAL_SECTION _cs;
			CONDITION_VARIABLE _cv;
			std::list<arg> _queue;
		};

		template<typename... Args>
		void postJob(NativeThreadQueue<Args...> &q, Args ...args) 
		{
			q.push(std::make_tuple(args...));
		}

		template<typename... Args>
		class NativeThread
		{
		    typedef void (*EXCEPTION_HANDLER)(std::exception &);
		    typedef void (*CALLBACKF_HANDLER)(std::tuple<Args...> &);

		public:
			explicit NativeThread(const THREAD_PRIORITY tP = NORMAL) 
				: _cbfhandler(nullptr), _queue(nullptr), _priority(tP) 
			{
				InitializeCriticalSection(&_cs);
			}

			NativeThread(const NativeThread &) = delete;
			NativeThread(NativeThread &&) = delete;

			~NativeThread() 
			{
			    stop(); 
				join();
			    DeleteCriticalSection(&_cs);
			}
			
			const NativeThread & operator = (const NativeThread &) = delete;
			NativeThread & operator = (NativeThread &&) = delete;

			void setErrorHandler(const EXCEPTION_HANDLER ehandler) 
			{
				NativeThreadAutoLock l(_cs); 
				_ehandler = ehandler;
			}

			auto getErrorHandler() 
			{
				NativeThreadAutoLock l(_cs); 
				return std::ref(_ehandler);
			}

			void setThreadFunction(CALLBACKF_HANDLER cbfhandler) 
			{
				NativeThreadAutoLock l(_cs); _cbfhandler = cbfhandler;
			}

			void setMaxQueueSize(const uint64_t maxSize) 
			{
				NativeThreadAutoLock l(_cs); 
				_queueMaxSize = maxSize;
			}

			uint64_t getMaxQueueSize() const
			{
				uint64_t retVal;
				InterlockedExchange64 (reinterpret_cast<LONG64*>(&retVal), static_cast<LONG64>(_queueMaxSize)); 
				return retVal;
			}

			auto getThreadFunction() 
			{
				NativeThreadAutoLock l(_cs); 
				return std::ref(_cbfhandler);
			}

			void setQueue(NativeThreadQueue<Args...> *queue) 
			{
				NativeThreadAutoLock l(_cs); 
				_queue = queue;
			}

			auto getQueue() 
			{
				NativeThreadAutoLock l(_cs); 
				return _queue;
			}

			void setThreadPriority(THREAD_PRIORITY priority) 
			{
    			NativeThreadAutoLock l(_cs); 
			    if (_handle) 
				{
    			    SetThreadPriority(_handle, _priority);
			    }
			    _priority = priority;
			}

			bool start()
			{
    			NativeThreadAutoLock l(_cs);
				if (_handle)
				{
					return false;
				}
				_state = THREAD_STATE::START;
				_handle = CreateThread(nullptr, 0, threadFunction, this, 0, nullptr);
				if (_handle != INVALID_HANDLE_VALUE)
				{
					_state = THREAD_STATE::IDLE; return false;
				}

    			SetThreadPriority(_handle, _priority);
    			return true;
			}

			void stop() 
			{
				InterlockedCompareExchange(reinterpret_cast<unsigned *>(&_state), static_cast<unsigned>(THREAD_STATE::IDLE), static_cast<unsigned>(THREAD_STATE::BUSY) );
			}

			void join() 
			{
				while ( WaitForSingleObject(_handle, 0) == WAIT_TIMEOUT ) 
				{
					Sleep(10);
				}

				CloseHandle(_handle);
				_handle = nullptr;
			}

		protected:
			THREAD_STATE getState() const
			{
				THREAD_STATE retVal;
				InterlockedExchange (reinterpret_cast<unsigned*>(&retVal), static_cast<unsigned>(_state)); 
				return retVal;
			}

			void setState(THREAD_STATE state) 
			{
				InterlockedExchange(reinterpret_cast<unsigned*>(&_state), static_cast<unsigned>(state));
			}

			static DWORD WINAPI threadFunction(LPVOID pParam) 
			{

				NativeThread *pThis = static_cast<NativeThread*>(pParam);
				pThis->setState(THREAD_STATE::BUSY);

				for (;;)
				{
					try {
						auto q = pThis->getQueue();
						if (!q) { throw std::runtime_error("unknown message queue"); }

						if (q->empty() && pThis->getState() != THREAD_STATE::BUSY) {
							break;
						}

						std::tuple<Args...> params;
						if (!q->next(params)) { continue; }

						auto cbf = pThis->getThreadFunction();
						if (cbf) { cbf(params); }

					}
					catch (std::exception & e) {
						auto ehandler = pThis->getErrorHandler();
						if (ehandler) { ehandler(e); }
					}
				}
				return 0;
			}

		    CRITICAL_SECTION  _cs;

			HANDLE _handle = nullptr;
			EXCEPTION_HANDLER _ehandler = nullptr;
		    CALLBACKF_HANDLER _cbfhandler = nullptr;
			NativeThreadQueue<Args...> *_queue;
			THREAD_STATE _state = THREAD_STATE::IDLE;
			THREAD_PRIORITY _priority = NORMAL;
			uint64_t _queueMaxSize = 1024 * 1024 * 1024;
		};

		//
		// static functions
		//

		using shared_char_vector = std::shared_ptr<std::vector<unsigned char>>;

		THREADS_EXPORT NativeThreadQueue<shared_char_vector, std::string>& fileDumpThreadQueue();
		THREADS_EXPORT NativeThread<shared_char_vector, std::string>& fileDumpThread();
		THREADS_EXPORT bool dumpThreadPostJob(shared_char_vector img, std::string path, bool postAsync );
		THREADS_EXPORT uint64_t fileDumpThreadQueueSize();
	}
}

