#pragma once
#include <string>
#include <sstream>
#include "june_errors.h"
#include "util.h"

namespace LandaJune
{
	#define THROW_EX_ERR(x) throw BaseException(x, __FILE__, __LINE__);
	#define THROW_EX_INT(x) throw BaseException(CORE_ERROR{x}, __FILE__, __LINE__);
	#define THROW_EX_ERR_STR(x,y) throw BaseException(CORE_ERROR{x,y}, __FILE__, __LINE__);

	class BaseException;

	class BaseException : public std::exception
	{
	public:

		BaseException() 
			: _error()
			, _threadId(Helpers::Utility::threadId())
			, _timeStamp(std::chrono::system_clock::now()) {}

		BaseException(const char* sfile, const int iline) noexcept
			  : _error()
			  , _threadId(Helpers::Utility::threadId())
			  , _timeStamp(std::chrono::system_clock::now())
			  , _file(sfile)
			  , _line(iline)
		{
		}

		BaseException(const CORE_ERROR& err, const char* sfile, const int iline) noexcept 
			: _error(err)
			, _threadId(Helpers::Utility::threadId())
			, _timeStamp(std::chrono::system_clock::now())
			, _file(sfile)
			, _line(iline)
		{
		}
		

		BaseException(const BaseException& other) noexcept 
			: _error(other._error)
			, _threadId(other._threadId)
			, _timeStamp(other._timeStamp)
			, _file(other._file)
			, _line(other._line) 
		{}


		BaseException(BaseException&& other) noexcept 
			: _error(std::move(other._error))
			, _threadId(other._threadId)
			, _timeStamp(other._timeStamp)
			, _file(std::move(other._file))
			, _line(other._line) 
			{}

		virtual ~BaseException() noexcept = default;
		const char * what() const noexcept override { return _error._errorString.c_str(); }
		const char * file() const noexcept { return _file.c_str(); }
		int  line() const noexcept { return _line; }
		unsigned long thread() const noexcept { return _threadId; }
		int errorID() const noexcept { return _error._error; }
		const CORE_ERROR& error() const noexcept { return _error; }
		std::chrono::system_clock::time_point timeStamp() const noexcept { return _timeStamp; }
	private:

		CORE_ERROR _error;
		unsigned long _threadId = 0;
		std::chrono::system_clock::time_point _timeStamp;
		std::string _file;
		int _line =-1;
	};

	inline void print_base_exception(const std::exception& e, std::ostringstream& ss, int level =  0)
	{
		if ( level != 0 )
			ss << std::string(level, ' ') << "internal exception: " << e.what() << std::endl;
		try 
		{
			std::rethrow_if_nested(e);
		} 
		catch(const std::exception& e) 
		{
			print_base_exception(e, ss, level+1);
		}
		catch(...) {}
	}

	inline void print_exception(const BaseException& e, std::ostringstream& ss)
	{
		ss << "Base exception caught : \r\n\t" << e.error() << std::endl;
		print_base_exception(static_cast<const std::exception&>(e), ss, 0 );
	}

}
