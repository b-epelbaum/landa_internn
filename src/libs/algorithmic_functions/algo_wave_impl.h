#pragma once

#include "algorithm_parameters.h"

const int MAX_CIRCLES_IN_WAVE = 1000;


void detect_wave_init(const LandaJune::Algorithms::WAVE_INIT_PARAMETER& initParam);
void detect_wave(LandaJune::Algorithms::PARAMS_WAVE_INPUT_PTR input, LandaJune::Algorithms::PARAMS_I2S_OUTPUT_PTR waveTriangleOut, LandaJune::Algorithms::PARAMS_WAVE_OUTPUT_PTR output);
void detect_wave_shutdown();


struct Circle_Pos
{
	float x, y;
};



struct Circle_Pos_Int
{
	int x, y;
};

