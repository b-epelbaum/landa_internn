#pragma once

#include "utillib_global.h"
#include <string>
#include <optional>
#include <chrono>
#include <vector>
#include <algorithm>

#define TIME_STAMP std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()

namespace LandaJune
{
	namespace Helpers
	{
		class Utility
		{
		public:

			static unsigned long threadId();
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

			static std::vector<std::string>	split_string(const std::string& str, const std::string& delims = " ")
			{
			    std::vector<std::string> output;
			    auto first = std::cbegin(str);

			    while (first != std::cend(str))
			    {
			        const auto second = std::find_first_of(first, std::cend(str), 
			                  std::cbegin(delims), std::cend(delims));

			        if (first != second)
			            output.emplace_back(first, second);

			        if (second == std::cend(str))
			            break;

			        first = std::next(second);
			    }
			    return output;
			}
		};
	}
}
