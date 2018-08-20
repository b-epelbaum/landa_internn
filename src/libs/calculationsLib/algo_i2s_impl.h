#pragma once

#include "algorithm_parameters.h"

void detect_i2s_init(const LandaJune::Parameters::INIT_PARAMETER& initParam);
void detect_i2s(const LandaJune::Parameters::PARAMS_I2S_INPUT& input, LandaJune::Parameters::PARAMS_I2S_OUTPUT& output);
void detect_i2s_shutdown();
