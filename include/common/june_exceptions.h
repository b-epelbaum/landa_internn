#pragma once
#include "june_errors.h"
#include "june_exceptions.h"

namespace LandaJune
{
	template <typename Er>
	class BaseException : public std::exception
	{
	public:
		BaseException() noexcept = default;
		explicit BaseException(Er err) noexcept : _error(err) {}
		explicit BaseException(Er err, std::string str) noexcept : _error(err), _msg(std::move(str)) {}
		BaseException(const BaseException& other) noexcept : _error(other._error), _msg(other._msg) {}
		BaseException(BaseException&& other) noexcept : _error(other._error), _msg(std::move(other._msg)) {}
		virtual ~BaseException() noexcept = default;
		const char * what() const noexcept override { return _msg.c_str(); }
		Er error() const noexcept { return _error; }

	private:

		Er _error = JUNE_NO_ERROR;
		std::string _msg;
	};

	namespace FrameProviders
	{
		using ProviderException = BaseException<FRAME_PROVIDER_ERROR>;
		using FrameRefException = BaseException<FRAME_REF_ERROR>;
	}

	namespace Core
	{
		using CoreEngineException = BaseException<CORE_ENGINE_ERROR>;
	}

	namespace Algorithms
	{
		using AlgorithmException = BaseException<ALGORITHM_ERROR>;
	}
}