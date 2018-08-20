#pragma once

#include "algorithm_parameters.h"

void detect_edge_init(const LandaJune::Parameters::INIT_PARAMETER& initParam);
void detect_edge(const LandaJune::Parameters::PARAMS_PAPEREDGE_INPUT& input, LandaJune::Parameters::PARAMS_PAPEREDGE_OUTPUT& output);
void detect_edge_shutdown();
