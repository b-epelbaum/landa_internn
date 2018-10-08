#pragma once

#include "algorithm_parameters.h"

void detect_c2c_roi_init(const LandaJune::Algorithms::C2C_ROI_INIT_PARAMETER& initParam);
void detect_c2c_roi(LandaJune::Algorithms::PARAMS_C2C_ROI_INPUT_PTR input, LandaJune::Algorithms::PARAMS_C2C_ROI_OUTPUT_PTR output);
void detect_c2c_roi_shutdown();

