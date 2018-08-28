#pragma once
#include "fullpagehandler_global.h"
#include "abstractalgohandler.h"

namespace LandaJune
{
	namespace Algorithms
	{
		class FULLPAGEHANDLER_EXPORT fullPageHandler : public abstractAlgoHandler
		{
			Q_OBJECT
			Q_PLUGIN_METADATA(IID IAlgorithmHandler_iid)
			Q_INTERFACES(LandaJune::Algorithms::IAlgorithmHandler)

		public:
			fullPageHandler();
			fullPageHandler(const fullPageHandler &) = delete;
			fullPageHandler(fullPageHandler &&) = delete;
			virtual ~fullPageHandler();

			const fullPageHandler & operator = (const fullPageHandler &) = delete;
			fullPageHandler & operator = (fullPageHandler &&) = delete;

			// IAlgorithmHandler methods
			std::unique_ptr<IAlgorithmHandler> clone() override;
			QString getName() const override;
			QString getDescription() const override;

			void init(std::shared_ptr<Parameters::BaseParameter> parameters) override;
			void cleanup() override;
			void process(const Core::FrameRef * frame) override;

			std::shared_ptr<Parameters::BaseParameter> getParameters() const override;

		protected:

			void validateProcessParameters(std::shared_ptr<Parameters::BaseParameter> parameters) override;

		};
	}
}


