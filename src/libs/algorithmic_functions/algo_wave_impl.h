#pragma once

#include "algorithm_parameters.h"

const int MAX_CIRCLES_IN_WAVE = 1000;


void detect_wave_init(const LandaJune::Algorithms::WAVE_INIT_PARAMETER& initParam);
void detect_wave(std::shared_ptr<LandaJune::Algorithms::PARAMS_WAVE_INPUT> input, std::shared_ptr<LandaJune::Algorithms::PARAMS_WAVE_OUTPUT> output);
void detect_wave_shutdown();


struct Circle_Pos
{
	float x, y;
};

