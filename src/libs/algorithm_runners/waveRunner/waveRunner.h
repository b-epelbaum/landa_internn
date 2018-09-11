#pragma once
#include "waveRunner_global.h"
#include "baseAlgorithmRunner.h"

namespace LandaJune
{
	namespace Algorithms
	{
		class WAVEHANDLER_EXPORT wavePageHandler : public baseAlgorithmRunner
		{
			Q_OBJECT
				Q_PLUGIN_METADATA(IID IAlgorithmRunner_iid)
				Q_INTERFACES(LandaJune::Algorithms::IAlgorithmRunner)

		public:
			wavePageHandler();
			wavePageHandler(const wavePageHandler & other) = default;
			wavePageHandler(wavePageHandler &&) = delete;
			virtual ~wavePageHandler();

			const wavePageHandler & operator = (const wavePageHandler &) = delete;
			wavePageHandler & operator = (wavePageHandler &&) = delete;

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


