#pragma once
#include "waveHandler_global.h"
#include "abstractalgohandler.h"

namespace LandaJune
{
	namespace Algorithms
	{
		class WAVEHANDLER_EXPORT wavePageHandler : public abstractAlgoHandler
		{
			Q_OBJECT
				Q_PLUGIN_METADATA(IID IAlgorithmHandler_iid)
				Q_INTERFACES(LandaJune::Algorithms::IAlgorithmHandler)

		public:
			wavePageHandler();
			wavePageHandler(const wavePageHandler & other) = default;
			wavePageHandler(wavePageHandler &&) = delete;
			virtual ~wavePageHandler();

			const wavePageHandler & operator = (const wavePageHandler &) = delete;
			wavePageHandler & operator = (wavePageHandler &&) = delete;

			// IAlgorithmHandler methods
			std::unique_ptr<IAlgorithmHandler> clone() override;
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


