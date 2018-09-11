#pragma once

#include "algorithm_parameters.h"

void detect_i2s_init(const LandaJune::Algorithms::INIT_PARAMETER& initParam);
void detect_i2s(std::shared_ptr<LandaJune::Algorithms::PARAMS_I2S_INPUT> input, std::shared_ptr<LandaJune::Algorithms::PARAMS_I2S_OUTPUT> output);
void detect_i2s_shutdown();