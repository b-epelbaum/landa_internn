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
		_current[i] = 0; _values[i] = 0; _times[i] = 0;
	}
}


RealTimeStats::~RealTimeStats()
= default;

void RealTimeStats::reset()
{
	for (auto i = 0; i < statsNumber; ++i)
	{
		_current[i] = 0; _values[i] = 0; _times[i] = 0;
	}
}

void RealTimeStats::increment(const StatName stat, const double delta, const double v) 
{
	autolock l(_mutex);
	if (stat < statsNumber) 
	{
		_current[stat] = v;
		_values[stat] += v;
		_times[stat] += delta;
	}
}

static const char *__names[RealTimeStats::statsNumber] = {
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
	"Saved Bitmaps Fail",
	"Save Queue Length"
};

std::string RealTimeStats::name(StatName id) const {
	return std::move(std::string(id < statsNumber ? __names[id] : ""));
}

RealTimeStats::StatInfo RealTimeStats::info(StatName id) const {
	StatInfo si{};
	if (id < statsNumber) {
		si._total = _values[id];
		si._current = _current[id];
		if (_times[id]) si._average = _values[id] / _times[id];
		if (_values[id]) si._raverage = _times[id] /  _values[id];
	}
	return si;
}

std::string RealTimeStats::to_string(bool bBreakLines) 
{
	autolock l(_mutex);
	std::string txt;

	std::string breaker =  bBreakLines ? "\r\n" : "\n";
	for (auto i = 0; i < statsNumber; ++i) 
	{
		if (_values[i] < 0.0001)
		{
			continue;
		}

		txt.append(__names[i]).append(":\t").append(std::to_string(_values[i]))
			.append("\t").append(std::to_string(_current[i]))
			.append("\t").append(std::to_string(_values[i] / _times[i]))
			.append("\t").append(std::to_string(_times[i] / _values[i])).append(breaker);
		
	}
	return std::move(txt);
}

RealTimeStats* RealTimeStats::rtStats()
{
	return _this ? _this : _this = new RealTimeStats();
}
