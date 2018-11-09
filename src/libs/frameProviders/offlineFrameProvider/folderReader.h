#pragma once

#include "BaseFrameProvider.h"
#include <thread>

namespace LandaJune
{
	namespace FrameProviders
	{
		class folderReader : public BaseFrameProvider
		{
			Q_OBJECT

			friend class offlineFrameProvider;

		public:

			folderReader();
			folderReader(const folderReader &) = delete;
			folderReader(folderReader &&) = delete;
			virtual ~folderReader();

			void init(BaseParametersPtr parameters, Core::ICore * coreObject, CoreEventCallback callback) override;
			void cleanup() override;

			const folderReader & operator = (const folderReader &) = delete;
			folderReader & operator = (folderReader &&) = delete;

			int32_t getFrameLifeSpan() const override;
			int getRecommendedFramePoolSize() override { return 2;}
			bool shouldReportSkippedFrame() const override { return false;}
					
			CORE_ERROR prepareData(Core::FrameRef* frameRef) override;
			CORE_ERROR accessData(Core::FrameRef* frameRef) override;
			void releaseData(Core::FrameRef* frameRef) override {}

			DECLARE_NORMAL_PARAM_PROPERTY(SourceFolderPath, QString, "")
			DECLARE_NORMAL_PARAM_PROPERTY(ImageMaxCount, int, -1)
			DECLARE_NORMAL_PARAM_PROPERTY(OfflineRegStripOnly, bool, false)
			DECLARE_NORMAL_PARAM_PROPERTY(OfflineRegStripsLeftAndRight, bool, false)
			DECLARE_NORMAL_PARAM_PROPERTY(OfflineRightStripIsEven, bool, true)

		protected:

			void sortImageFileList();
			void splitToLeftAndRight();
			void validateParameters(BaseParametersPtr parameters) override;

		private :

			QVector<QString>	_imagePaths;

			QVector<QString>	_imagePathsLeft;
			QVector<QString>	_imagePathsRight;

			bool _bAccessLeft = true;
		};
	}
}
