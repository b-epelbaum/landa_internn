#pragma once
#include <string>
#include <sstream>
#include "june_errors.h"
#include "util.h"
#include <iomanip>

namespace LandaJune
{
	#define THROW_EX_ERR(x) throw BaseException(x, __FILE__, __LINE__);
	#define THROW_EX_INT(x) throw BaseException(CORE_ERROR{x}, __FILE__, __LINE__);
	#define THROW_EX_ERR_STR(x,y) throw BaseException(CORE_ERROR{x,y}, __FILE__, __LINE__);
	
	#define RETHROW(x,y) std::throw_with_nested(BaseException(CORE_ERROR{x, y }, __FILE__, __LINE__));

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
			ss << std::string(level, ' ') << "\t\tInternal exception: " << e.what() << std::endl;
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

	inline void print_exception(const BaseException& ex, std::ostringstream& ss)
	{
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(ex.timeStamp().time_since_epoch()) % 1000;
		auto in_time_t = std::chrono::system_clock::to_time_t(ex.timeStamp());
		std::tm bt = *((std::localtime)(&in_time_t));
		std::ostringstream st;
		st <<  std::put_time(&bt, "%Y-%m-%d %H:%M:%S");
		st << '.' << std::setfill('0') << std::setw(3) << ms.count();

		ss << "--------------  Core exception caught -----------------\r\n\t\t\t\tException details : \r\n"
					<< "\t\t\t\t\tError ID  : \t" << ex.error()			<< "\r\n"
					<< "\t\t\t\t\tError msg : \t" << ex.what()			<< "\r\n"
					<< "\t\t\t\t\tThread#   : \t" << ex.thread()		<< "\r\n"
					<< "\t\t\t\t\tTimestamp : \t" << st.str().c_str()	<< "\r\n"
					<< "\t\t\t\t\tSource    : \t" << ex.file()			<< "\r\n"
					<< "\t\t\t\t\tLine      : \t" << ex.line()			<< "\r\n";
		print_base_exception(static_cast<const std::exception&>(ex), ss, 0 );
		ss << "------------------------------------------------------------------------------";  
	}

}
