#pragma once

#include "fgSimulatorProvLib_global.h"
#include "BaseFrameProvider.h"

#include <QImageReader>

#include <opencv2/imgcodecs.hpp>

namespace LandaJune
{
	namespace FrameProviders
	{
		class FGSIMULATOR_EXPORT FGSimulator : public BaseFrameProvider
		{
			Q_OBJECT
			Q_PLUGIN_METADATA(IID IFrameProvider_iid)
			Q_INTERFACES(LandaJune::FrameProviders::IFrameProvider)

		public:

			FGSimulator();
			FGSimulator(const FGSimulator &) = delete;
			FGSimulator(FGSimulator &&) = delete;
			virtual ~FGSimulator();

			const FGSimulator & operator = (const FGSimulator &) = delete;
			FGSimulator & operator = (FGSimulator &&) = delete;

			bool canContinue(FRAME_PROVIDER_ERROR lastError) override;

			int getRecommendedFramePoolSize() override { return 4;  }
			FRAME_PROVIDER_ERROR prepareData(Core::FrameRef* frameRef) override;
			FRAME_PROVIDER_ERROR accessData(Core::FrameRef* frameRef) override;
			void releaseData(Core::FrameRef* frameRef) override;

			FRAME_PROVIDER_ERROR init() override;
			FRAME_PROVIDER_ERROR cleanup() override;

			DECLARE_PROVIDER_PROPERTY(SourceFolderPath, QString, "")
			DECLARE_PROVIDER_PROPERTY(SourceFilePath, QString, "")
			DECLARE_PROVIDER_PROPERTY(FrameFrequencyInMSec, int, 1000)

		protected:

			void validateParameters(std::shared_ptr<Parameters::BaseParameters> parameters) override;


		private :


			std::vector<cv::Mat> _images;
			uint64_t _next = 0ULL;
		};
	}
}
