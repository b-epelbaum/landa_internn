#pragma once

#include "algorithm_parameters.h"

void detect_wave_init(const LandaJune::Algorithms::INIT_PARAMETER& initParam);
void detect_wave(const LandaJune::Algorithms::PARAMS_WAVE_INPUT& input, LandaJune::Algorithms::PARAMS_WAVE_OUTPUT& output);
void detect_wave_shutdown();

