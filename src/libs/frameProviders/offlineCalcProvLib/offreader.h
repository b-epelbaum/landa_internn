#pragma once

#include "offlineCalcProvLib_global.h"
#include "BaseFrameProvider.h"

#include <opencv2/imgcodecs.hpp>
#include <thread>


namespace LandaJune
{
	namespace FrameProviders
	{
		class OFFREADER_EXPORT OfflineReader : public BaseFrameProvider
		{
			Q_OBJECT
			Q_PLUGIN_METADATA(IID IFrameProvider_iid)
			Q_INTERFACES(LandaJune::FrameProviders::IFrameProvider)

		public:

			OfflineReader();
			OfflineReader(const OfflineReader &) = delete;
			OfflineReader(OfflineReader &&) = delete;
			virtual ~OfflineReader();


			const OfflineReader & operator = (const OfflineReader &) = delete;
			OfflineReader & operator = (OfflineReader &&) = delete;

			bool warnAboutDroppedFrames() override { return false; }
			bool canContinue(FRAME_PROVIDER_ERROR lastError) override;

			int getRecommendedFramePoolSize() override { return std::thread::hardware_concurrency(); }
			FRAME_PROVIDER_ERROR prepareData(Core::FrameRef* frameRef) override;
			FRAME_PROVIDER_ERROR accessData(Core::FrameRef* frameRef) override;
			void releaseData(Core::FrameRef* frameRef) override;


			FRAME_PROVIDER_ERROR init() override;
			FRAME_PROVIDER_ERROR cleanup() override;

			DECLARE_PROVIDER_PROPERTY(SourceFolderPath, QString, "")
			DECLARE_PROVIDER_PROPERTY(ImageMaxCount, int, -1)

		protected:

			void validateParameters(std::shared_ptr<Parameters::BaseParameters> parameters) override;

		private :

			QVector<QString>	_imagePaths;
		};
	}
}
