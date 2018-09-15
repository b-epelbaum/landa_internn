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
			bool canContinue(FRAME_PROVIDER_ERROR lastError) override;

			int getRecommendedFramePoolSize() override { return std::thread::hardware_concurrency(); }
			FRAME_PROVIDER_ERROR prepareData(Core::FrameRef* frameRef) override;
			FRAME_PROVIDER_ERROR accessData(Core::FrameRef* frameRef) override;
			void releaseData(Core::FrameRef* frameRef) override;


			FRAME_PROVIDER_ERROR init() override;
			FRAME_PROVIDER_ERROR cleanup() override;

			DECLARE_PARAM_PROPERTY(SourceFolderPath, QString, "", true)
			DECLARE_PARAM_PROPERTY(ImageMaxCount, int, -1, true)

		protected:

			void validateParameters(std::shared_ptr<Parameters::BaseParameters> parameters) override;

		private :

			QVector<QString>	_imagePaths;
		};
	}
}
