#pragma once

#include "algorithms_global.h"
#include "algorithm_parameters.h"

using namespace LandaJune::Parameters;

namespace LandaJune
{
	namespace Core
	{
		class FrameRef;
	}
}

namespace LandaJune
{
	namespace Algorithms
	{
		void ALGORITHMS_EXPORT initAlgorithmsData(std::shared_ptr<ProcessParameter> iputParams);
		PARAMS_C2C_SHEET_OUTPUT ALGORITHMS_EXPORT calculateAll(const Core::FrameRef* frame);
		void ALGORITHMS_EXPORT clearAlgorithmsData();

		// todo : move function to functions library
		void generateRegions(const Core::FrameRef* frame, PARAMS_C2C_SHEET_INPUT& input);
		PARAMS_C2C_SHEET_OUTPUT calculateSheet(const PARAMS_C2C_SHEET_INPUT& sheetInput);
		PARAMS_C2C_STRIP_OUTPUT calculateStrip (const PARAMS_C2C_STRIP_INPUT& stripInput, bool detectEdge);

		void initEdge(const INIT_PARAMETER& initParam );
		PARAMS_PAPEREDGE_OUTPUT calculateEdge(const PARAMS_PAPEREDGE_INPUT& input);
		void shutdownEdge();

		void initI2S(const INIT_PARAMETER& initParam);
		PARAMS_I2S_OUTPUT calculateI2S(const PARAMS_I2S_INPUT& input);
		void shutdownI2S();

		void initC2CRoi(const INIT_PARAMETER& initParam);
		PARAMS_C2C_ROI_OUTPUT calculateC2CRoi(const PARAMS_C2C_ROI_INPUT& input);
		void shutdownC2CRoi();

		void initWave(const INIT_PARAMETER& initParam);
		PARAMS_WAVE_OUTPUT calculateWave(const PARAMS_WAVE_INPUT& input);
		void shutdownWave();
	}
}