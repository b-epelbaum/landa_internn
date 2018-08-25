#pragma once

#include "algorithm_parameters.h"

void detect_edge_init(const LandaJune::Algorithms::INIT_PARAMETER& initParam);
void detect_edge(const LandaJune::Algorithms::PARAMS_PAPEREDGE_INPUT& input, LandaJune::Algorithms::PARAMS_PAPEREDGE_OUTPUT& output);
void detect_edge_shutdown();
