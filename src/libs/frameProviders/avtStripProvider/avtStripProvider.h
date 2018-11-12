#pragma once

#include "avtStripProvider_global.h"
#include "BaseFrameProvider.h"

#include <QImageReader>

#include <opencv2/imgcodecs.hpp>
#include <thread>

namespace LandaJune
{
	namespace FrameProviders
	{
		class AVT_STRIP_PROV_EXPORT avtStripProvider : public BaseFrameProvider
		{
			Q_OBJECT
			Q_PLUGIN_METADATA(IID IFrameProvider_iid)
			Q_INTERFACES(LandaJune::FrameProviders::IFrameProvider)

		public:

			avtStripProvider();
			avtStripProvider(const avtStripProvider &) = delete;
			avtStripProvider(avtStripProvider &&) = delete;
			virtual ~avtStripProvider();

			const avtStripProvider & operator = (const avtStripProvider &) = delete;
			avtStripProvider & operator = (avtStripProvider &&) = delete;

			void init(BaseParametersPtr parameters, Core::ICore * coreObject, CoreEventCallback callback) override;
			void cleanup() override;

			int32_t getFrameLifeSpan() const override;
			int getRecommendedFramePoolSize() override { return 2;}
			bool shouldReportSkippedFrame() const override { return false;}
					
			CORE_ERROR prepareData(Core::FrameRef* frameRef) override;
			CORE_ERROR accessData(Core::FrameRef* frameRef) override;
			void releaseData(Core::FrameRef* frameRef) override {}

			DECLARE_NORMAL_PARAM_PROPERTY(SourceFolderPath, QString, "")
			DECLARE_NORMAL_PARAM_PROPERTY(ImageMaxCount, int, -1)

		protected:

			void sortImageFileList( QStringList& pathList);
			void splitToLeftAndRight();
			void validateParameters(BaseParametersPtr parameters) override;

		private :

			QVector<QString>	_imagePathsLeft;
			QVector<QString>	_imagePathsRight;

			bool _bCurrentLeftSide = true;
		};
	}
}
