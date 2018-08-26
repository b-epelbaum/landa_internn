#pragma once
#include "registrationHandler_global.h"
#include "../interfaces/IAlgorithmHandler.h"

#include "algorithm_parameters.h"

namespace LandaJune {
	namespace Core {
		class FrameRef;
	}
}

namespace LandaJune {
	namespace Core {
		class FrameRef;
	}
}

namespace LandaJune {
	namespace Parameters {
		class ProcessParameter;
	}
}


namespace LandaJune
{
	namespace Algorithms
	{
		class REGISTRATIONHANDLER_EXPORT registrationPageHandler : public QObject, public IAlgorithmHandler
		{
			Q_OBJECT
			Q_PLUGIN_METADATA(IID IAlgorithmHandler_iid)
			Q_INTERFACES(LandaJune::Algorithms::IAlgorithmHandler)

		public:
			registrationPageHandler();
			registrationPageHandler(const registrationPageHandler &) = delete;
			registrationPageHandler(registrationPageHandler &&) = delete;
			virtual ~registrationPageHandler();

			std::unique_ptr<IAlgorithmHandler> clone() override;

			const registrationPageHandler & operator = (const registrationPageHandler &) = delete;
			registrationPageHandler & operator = (registrationPageHandler &&) = delete;

			QString getName() const override;
			QString getDescription() const override;

			void init(std::shared_ptr<Parameters::BaseParameter> parameters) override;
			void cleanup() override;

			void process(const Core::FrameRef * frame) override;

			std::shared_ptr<Parameters::BaseParameter> getParameters() const override;

		protected:

			void validateProcessParameters(std::shared_ptr<Parameters::BaseParameter> parameters) override;

		private:

			void fillProcessParameters(const Core::FrameRef* frame, PARAMS_C2C_STRIP_INPUT& input);
			void generateRegions(PARAMS_C2C_STRIP_INPUT& input);

			PARAMS_C2C_STRIP_OUTPUT processStrip(const PARAMS_C2C_STRIP_INPUT& stripInput, bool detectEdge);

			void initEdge(const INIT_PARAMETER& initParam) const;
			PARAMS_PAPEREDGE_OUTPUT processEdge(const PARAMS_PAPEREDGE_INPUT& input);
			void shutdownEdge() const;

			void initI2S(const INIT_PARAMETER& initParam) const;
			PARAMS_I2S_OUTPUT processI2S(const PARAMS_I2S_INPUT& input);
			void shutdownI2S() const;

			void initC2CRoi(const INIT_PARAMETER& initParam) const;
			PARAMS_C2C_ROI_OUTPUT processC2CROI(const PARAMS_C2C_ROI_INPUT& input);
			void shutdownC2CRoi() const;
		
		private :

			void constructFrameContainer(const Core::FrameRef* frame, int bitsPerPixel );

			const Core::FrameRef* _frame = nullptr;;
			std::unique_ptr<cv::Mat> _frameContainer;
			int _frameIndex = 0;
			std::shared_ptr<Parameters::ProcessParameter> _processParameters;
			bool _bParallelizeCalculations = false;
		};
	}
}


