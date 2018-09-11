#pragma once

#include "algorithm_parameters.h"

void detect_edge_init(const LandaJune::Algorithms::INIT_PARAMETER& initParam);
void detect_edge(std::shared_ptr<LandaJune::Algorithms::PARAMS_PAPEREDGE_INPUT> input, std::shared_ptr<LandaJune::Algorithms::PARAMS_PAPEREDGE_OUTPUT> output);
void detect_edge_shutdown();
