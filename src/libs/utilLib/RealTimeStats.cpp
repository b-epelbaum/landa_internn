#include "RealTimeStats.h"
#include <string>

using namespace LandaJune::Helpers;
using autolock = std::lock_guard<std::mutex>;

std::shared_ptr<RealTimeStats> RealTimeStats::_this;

RealTimeStats::RealTimeStats()
{
	for (auto i = 0; i < statsNumber; ++i) 
	{
		_values[i] = 0; _times[i] = 0;
	}
}


RealTimeStats::~RealTimeStats()
= default;

void RealTimeStats::reset()
{
	for (auto i = 0; i < statsNumber; ++i)
	{
		_values[i] = 0; _times[i] = 0;
	}
}

void RealTimeStats::increment(const StatName stat, const double delta, const double v) 
{
	autolock l(_mutex);
	if (stat < statsNumber) 
	{
		_values[stat] += v;
		_times[stat] += delta;
	}
}

std::string RealTimeStats::to_string() 
{
	static const char *names[statsNumber] = {
		"Generated Images Ok",
		"Performed Algo Ok",
		"Performed Algo Result Ok",
		"Created Regions Ok",
		"Saved Bitmaps Ok",
		"Generated Images Drop",
		"Generated Images Fail",
		"Performed Algo Fail",
		"Performed Algo Result Fail",
		"Created Regions Fail",
		"Saved Bitmaps Fail"
	};
	autolock l(_mutex);
	std::string txt;
	for (auto i = 0; i < statsNumber; ++i) 
	{
		if (!_values[i]) { continue; }
		txt.append(names[i]).append(":\t").append(std::to_string(_values[i]))
			.append("\t").append(std::to_string(_values[i] / _times[i]))
			.append("\t").append(std::to_string(_times[i] / _values[i])).append("\n");
	}
	return std::move(txt);
}
