#pragma once
#include "fullImageRunner_global.h"
#include "baseAlgorithmRunner.h"

namespace LandaJune
{
	namespace Algorithms
	{

		class FULLIMAGERUNNER_EXPORT fullImageRunner : public baseAlgorithmRunner
		{
			Q_OBJECT
			Q_PLUGIN_METADATA(IID IAlgorithmRunner_iid)
			Q_INTERFACES(LandaJune::Algorithms::IAlgorithmRunner)

		public:
			fullImageRunner();
			fullImageRunner(const fullImageRunner &) = default;
			fullImageRunner(fullImageRunner &&) = delete;
			virtual ~fullImageRunner() = default;

			const fullImageRunner & operator = (const fullImageRunner &) = delete;
			fullImageRunner & operator = (fullImageRunner &&) = delete;

			void init (BaseParametersPtr parameters, Core::ICore* coreObject, CoreEventCallback callback) override;
			void cleanup() override;

			// IAlgorithmRunner methods
			std::unique_ptr<IAlgorithmRunner> clone() override;
			QString getName() const override;
			QString getDescription() const override;

			std::string getFrameFolderName() override;

			BaseParametersPtr getParameters() const override;

		protected:

			CORE_ERROR processInternal() override;
			void validateProcessParameters(BaseParametersPtr parameters) override;

			// general sheet output processing
			CORE_ERROR processSheetOutput(PARAMS_C2C_SHEET_OUTPUT_PTR sheetOutput) override;

			// general strip output processing
			void processStripOutput(PARAMS_C2C_STRIP_OUTPUT_PTR stripOutput) override;
			
			// general wave output processing
			void processWaveOutputs(concurrent_vector<PARAMS_WAVE_OUTPUT_PTR> & waveOutputs, PARAMS_I2S_OUTPUT_PTR waveTriangleOutput  ) override;

			virtual void sortWaveOutputs(concurrent_vector<PARAMS_WAVE_OUTPUT_PTR> & waveOutputs );
			virtual void logFailedStrip(PARAMS_C2C_STRIP_OUTPUT_PTR stripOutput);
			virtual void logFailedWaves(concurrent_vector<PARAMS_WAVE_OUTPUT_PTR> & waveOutputs);

			// saving strip CSV files
			virtual void processStripOutputCSV(PARAMS_C2C_STRIP_OUTPUT_PTR stripOutput);
			virtual void processWaveOutputsCSV(concurrent_vector<PARAMS_WAVE_OUTPUT_PTR> & waveOutputs );

			bool shouldProcessLeftStrip() const override;
			bool shouldProcessRightStrip() const override;

			bool isLeftStripInOfflineMode() const;

		};
	}
}


