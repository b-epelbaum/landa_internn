#pragma once

#include "algorithm_parameters.h"

void detect_i2s_init(const LandaJune::Algorithms::I2S_ROI_INIT_PARAMETER& initParam);
void detect_i2s(LandaJune::Algorithms::PARAMS_I2S_INPUT_PTR input, LandaJune::Algorithms::PARAMS_I2S_OUTPUT_PTR output);
void detect_i2s_shutdown();