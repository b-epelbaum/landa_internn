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


			const folderReader & operator = (const folderReader &) = delete;
			folderReader & operator = (folderReader &&) = delete;

			bool warnAboutDroppedFrames() override { return false; }
			bool canContinue(CORE_ERROR lastError) override;

			int32_t getFrameLifeSpan() const override;
			int getRecommendedFramePoolSize() override { return std::thread::hardware_concurrency(); }
			CORE_ERROR prepareData(Core::FrameRef* frameRef) override;
			CORE_ERROR accessData(Core::FrameRef* frameRef) override;
			void releaseData(Core::FrameRef* frameRef) override {}


			CORE_ERROR init(BaseParametersPtr parameters, Core::ICore * coreObject, FrameProviderCallback callback) override;
			CORE_ERROR cleanup() override;

			DECLARE_NORMAL_PARAM_PROPERTY(SourceFolderPath, QString, "")
			DECLARE_NORMAL_PARAM_PROPERTY(ImageMaxCount, int, -1)

		protected:

			void sortImageFileList();
			void validateParameters(BaseParametersPtr parameters) override;

		private :

			QVector<QString>	_imagePaths;
		};
	}
}
