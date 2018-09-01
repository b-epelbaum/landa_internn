#pragma once
#include "registrationHandler_global.h"
#include "abstractalgohandler.h"

namespace LandaJune
{
	namespace Algorithms
	{
		class REGISTRATIONHANDLER_EXPORT registrationPageHandler : public abstractAlgoHandler
		{
			Q_OBJECT
				Q_PLUGIN_METADATA(IID IAlgorithmHandler_iid)
				Q_INTERFACES(LandaJune::Algorithms::IAlgorithmHandler)

		public:
			registrationPageHandler();
			registrationPageHandler(const registrationPageHandler & other) = default;
			registrationPageHandler(registrationPageHandler &&) = delete;
			virtual ~registrationPageHandler();

			const registrationPageHandler & operator = (const registrationPageHandler &) = delete;
			registrationPageHandler & operator = (registrationPageHandler &&) = delete;

			// IAlgorithmHandler methods
			std::unique_ptr<IAlgorithmHandler> clone() override;
			QString getName() const override;
			QString getDescription() const override;

			std::string getFrameFolderName() const override;

			void init(std::shared_ptr<Parameters::BaseParameters> parameters) override;
			void cleanup() override;
			void process(const Core::FrameRef * frame) override;

			std::shared_ptr<Parameters::BaseParameters> getParameters() const override;

		protected:

			void validateProcessParameters(std::shared_ptr<Parameters::BaseParameters> parameters) override;

		private :
		

			std::string _sourceFrameNumber;
			SHEET_SIDE _regSide = LEFT;
		};
	}
}


