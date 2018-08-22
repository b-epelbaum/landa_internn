#pragma once

#include "offlineCalcProvLib_global.h"
#include "BaseFrameProvider.h"

#include <opencv2/imgcodecs.hpp>


namespace LandaJune
{
	namespace FrameProviders
	{
		static const QString OFFLINE_READER_CONFIG_FILE = "offreader.json";

		class OFFREADER_EXPORT OfflineReader : public BaseFrameProvider
		{
			Q_OBJECT
			Q_PLUGIN_METADATA(IID IFrameProvider_iid FILE "offreader.json")
			Q_INTERFACES(LandaJune::FrameProviders::IFrameProvider)

		public:

			OfflineReader();
			OfflineReader(const OfflineReader &) = delete;
			OfflineReader(OfflineReader &&) = delete;
			virtual ~OfflineReader();

			const OfflineReader & operator = (const OfflineReader &) = delete;
			OfflineReader & operator = (OfflineReader &&) = delete;

			bool canContinue(FRAME_PROVIDER_ERROR lastError) override;

			int getRecommendedFramePoolSize() override { return 1; }
			FRAME_PROVIDER_ERROR dataPreProcess(Core::FrameRef* frameRef) override;
			FRAME_PROVIDER_ERROR dataAccess(Core::FrameRef* frameRef) override;
			FRAME_PROVIDER_ERROR dataPostProcess(Core::FrameRef* frameRef) override;

			FRAME_PROVIDER_ERROR init() override;
			FRAME_PROVIDER_ERROR clean() override;

			DECLARE_PROVIDER_PROPERTY(SourceFolderPath, QString, "")
			DECLARE_PROVIDER_PROPERTY(ImageMaxCount, int, 1000)

		protected:

			QString getDefaultConfigurationFileName() const override {
				return OFFLINE_READER_CONFIG_FILE;
			}
		private :

			QVector<QString>	_imagePaths;
			cv::Mat				_currentImage;
		};
	}
}
