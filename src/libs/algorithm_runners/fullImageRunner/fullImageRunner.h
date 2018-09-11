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
			virtual ~fullImageRunner();

			const fullImageRunner & operator = (const fullImageRunner &) = delete;
			fullImageRunner & operator = (fullImageRunner &&) = delete;

			// IAlgorithmRunner methods
			std::unique_ptr<IAlgorithmRunner> clone() override;
			QString getName() const override;
			QString getDescription() const override;
			std::string parseSourceFrameIndexString(const std::string& strPath) override;

			std::string getFrameFolderName() const override;

			void init(std::shared_ptr<Parameters::BaseParameters> parameters) override;
			void cleanup() override;
			void process(const Core::FrameRef * frame) override;

			std::shared_ptr<Parameters::BaseParameters> getParameters() const override;

		protected:

			void validateProcessParameters(std::shared_ptr<Parameters::BaseParameters> parameters) override;

		};
	}
}


