#pragma once
namespace LandaJune
{
	namespace Algorithms
	{
		enum class ALGO_ERROR
		{
			  ERR_NO_ERROR = 0
			, ALG_STATUS_FAILED
			, ALG_STATUS_CIRCLE_NOT_FOUND
			, ALG_STATUS_CORRUPT_CIRCLE
			, ALG_STATUS_TOO_MANY_CIRCLES
			, ALG_STATUS_NOT_ENOUGH_CIRCLES
			, ALG_STATUS_NUM
			, ALG_ERROR_LAST
		};
	}
}
