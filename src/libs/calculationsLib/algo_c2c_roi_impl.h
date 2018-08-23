#pragma once

#include "algorithm_parameters.h"

void detect_c2c_roi_init(const LandaJune::Parameters::INIT_PARAMETER& initParam);
void detect_c2c_roi(const LandaJune::Parameters::PARAMS_C2C_ROI_INPUT& input, LandaJune::Parameters::PARAMS_C2C_ROI_OUTPUT& output);
void detect_c2c_roi_shutdown();

