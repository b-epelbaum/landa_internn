#pragma once

#include "utillib_global.h"
#include <string>
#include <optional>
#include <chrono>

namespace LandaJune
{
	namespace Helpers
	{
		class Utility
		{
		public:
			
			static long long now_in_microseconds()
			{
				const auto& t = std::chrono::system_clock::now().time_since_epoch();
				return std::chrono::duration_cast<std::chrono::microseconds>(t).count();
			}

			static long long now_in_millisecond()
			{
				const auto& t = std::chrono::system_clock::now().time_since_epoch();
				return std::chrono::duration_cast<std::chrono::milliseconds>(t).count();
			}

		};
	}
}
