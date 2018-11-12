#pragma once
#include "avtStripRunner_global.h"
#include "baseAlgorithmRunner.h"

namespace LandaJune
{
	namespace Algorithms
	{
		class AVTSTRIPRUNNER_EXPORT avtStripRunner : public baseAlgorithmRunner
		{
			Q_OBJECT
			Q_PLUGIN_METADATA(IID IAlgorithmRunner_iid)
			Q_INTERFACES(LandaJune::Algorithms::IAlgorithmRunner)

		public:
			avtStripRunner();
			avtStripRunner(const avtStripRunner &) = default;
			avtStripRunner(avtStripRunner &&) = delete;
			virtual ~avtStripRunner() = default;

			const avtStripRunner & operator = (const avtStripRunner &) = delete;
			avtStripRunner & operator = (avtStripRunner &&) = delete;

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
			bool shouldProcessLeftStrip() const override;
			bool shouldProcessRightStrip() const override;

			// parse frame index
			void parseFrameAndImageIndex();
			
			// saving strip CSV files
			virtual void processStripOutputCSV(PARAMS_C2C_STRIP_OUTPUT_PTR stripOutput);

			virtual void logFailedStrip(PARAMS_C2C_STRIP_OUTPUT_PTR stripOutput);

			bool isLeftStrip();

			bool _bIsLeftStrip = true;
		};
	}
}


