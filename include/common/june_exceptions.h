#pragma once
#include <string>
#include "june_errors.h"

namespace LandaJune
{
	class BaseException : public std::exception
	{
	public:
		BaseException() noexcept = default;
		explicit BaseException(const int err) noexcept : _error(err) {}
		explicit BaseException(const int err, std::string str) noexcept : _error(err), _msg(std::move(str)) {}
		BaseException(const BaseException& other) noexcept : _error(other._error), _msg(other._msg) {}
		BaseException(BaseException&& other) noexcept : _error(other._error), _msg(std::move(other._msg)) {}
		virtual ~BaseException() noexcept = default;
		const char * what() const noexcept override { return _msg.c_str(); }
		int error() const noexcept { return _error; }

	private:

		int _error = JUNE_NO_ERROR;
		std::string _msg;
	};
}