#include "stdafx.h"
#include "RealTimeStats.h"
#include <string>

using namespace LandaJune::Helpers;
using autolock = std::lock_guard<std::mutex>;

STATLIB_EXPORT RealTimeStats * RealTimeStats::_this = nullptr;

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
		"Frames generated",
		"Frames handled",
		"Regions generated",
		"Regions copied",
		"Handled strips",
		"Handled edges",
		"Handled I2S",
		"Handled C2C",
		"Handled Waves",
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
		if (_values[i] < 0.0001)
		{
			continue;
		}

		txt.append(names[i]).append(":\t").append(std::to_string(_values[i]))
			.append("\t").append(std::to_string(_values[i] / _times[i]))
			.append("\t").append(std::to_string(_times[i] / _values[i])).append("\n");
	}
	return std::move(txt);
}

RealTimeStats* RealTimeStats::rtStats()
{
	return _this ? _this : _this = new RealTimeStats();
}
