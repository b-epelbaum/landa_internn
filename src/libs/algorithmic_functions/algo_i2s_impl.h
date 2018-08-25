#pragma once

#include "algorithm_parameters.h"

void detect_i2s_init(const LandaJune::Algorithms::INIT_PARAMETER& initParam);
void detect_i2s(const LandaJune::Algorithms::PARAMS_I2S_INPUT& input, LandaJune::Algorithms::PARAMS_I2S_OUTPUT& output);
void detect_i2s_shutdown();