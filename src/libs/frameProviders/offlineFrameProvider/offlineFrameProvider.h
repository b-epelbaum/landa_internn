#pragma once

#include "offlineFrameProvider_global.h"
#include "BaseFrameProvider.h"

#include <QImageReader>

#include <opencv2/imgcodecs.hpp>

namespace LandaJune
{
	namespace FrameProviders
	{
		class OFFLINE_FRAME_PROV_EXPORT offlineFrameProvider : public BaseFrameProvider
		{
			Q_OBJECT
			Q_PLUGIN_METADATA(IID IFrameProvider_iid)
			Q_INTERFACES(LandaJune::FrameProviders::IFrameProvider)

		public:

			offlineFrameProvider();
			offlineFrameProvider(const offlineFrameProvider &) = delete;
			offlineFrameProvider(offlineFrameProvider &&) = delete;
			virtual ~offlineFrameProvider();

			const offlineFrameProvider & operator = (const offlineFrameProvider &) = delete;
			offlineFrameProvider & operator = (offlineFrameProvider &&) = delete;

			bool canContinue(CORE_ERROR lastError) override;

			int getRecommendedFramePoolSize() override { return 4;  }
			CORE_ERROR prepareData(Core::FrameRef* frameRef) override;
			CORE_ERROR accessData(Core::FrameRef* frameRef) override;
			void releaseData(Core::FrameRef* frameRef) override;

			CORE_ERROR init(std::shared_ptr<Parameters::BaseParameters> parameters) override;
			CORE_ERROR cleanup() override;

			DECLARE_PARAM_PROPERTY(SourceFolderPath, QString, "", true)
			DECLARE_PARAM_PROPERTY(SourceFilePath, QString, "", true)
			DECLARE_PARAM_PROPERTY(FrameFrequencyInMSec, int, 1000, true)
			DECLARE_PARAM_PROPERTY(ImageMaxCount, int, -1, true)
			DECLARE_PARAM_PROPERTY(CyclicGeneration, bool, true, true)

		protected:

			void validateParameters(std::shared_ptr<Parameters::BaseParameters> parameters) override;
			
			std::shared_ptr<BaseFrameProvider> _currentOfflineProvider;

		private :

			std::vector<cv::Mat> _images;
			uint64_t _next = 0ULL;
		};
	}
}
