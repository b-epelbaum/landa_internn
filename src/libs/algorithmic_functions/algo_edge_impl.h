#pragma once

#include "algorithm_parameters.h"

void detect_edge_init(const LandaJune::Algorithms::INIT_PARAMETER& initParam);
void detect_edge(LandaJune::Algorithms::PARAMS_PAPEREDGE_INPUT_PTR input, LandaJune::Algorithms::PARAMS_PAPEREDGE_OUTPUT_PTR output);
void detect_edge_shutdown();
