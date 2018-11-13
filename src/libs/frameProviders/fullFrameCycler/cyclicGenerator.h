#pragma once

#include "BaseFrameProvider.h"
#include <thread>

#include <opencv2/imgcodecs.hpp>

namespace LandaJune
{
	namespace FrameProviders
	{
		class cyclicGenerator : public BaseFrameProvider
		{
			Q_OBJECT

			friend class offlineFrameProvider;

		public:

			cyclicGenerator();
			cyclicGenerator(const cyclicGenerator &) = delete;
			cyclicGenerator(cyclicGenerator &&) = delete;
			virtual ~cyclicGenerator();

			void init(BaseParametersPtr parameters, Core::ICore * coreObject, CoreEventCallback callback) override;
			void cleanup() override;


			const cyclicGenerator & operator = (const cyclicGenerator &) = delete;
			cyclicGenerator & operator = (cyclicGenerator &&) = delete;

			int32_t getFrameLifeSpan() const override;
			int getRecommendedFramePoolSize() override { return std::thread::hardware_concurrency();}
			bool shouldReportSkippedFrame() const override { return true;}

			CORE_ERROR prepareData(Core::FrameRef* frameRef) override;
			CORE_ERROR accessData(Core::FrameRef* frameRef) override;
			void releaseData(Core::FrameRef* frameRef) override;

			DECLARE_NORMAL_PARAM_PROPERTY(SourceFilePath, QString, "")
			DECLARE_NORMAL_PARAM_PROPERTY(FrameFrequencyInMSec, int, 1000)
			DECLARE_NORMAL_PARAM_PROPERTY(ImageMaxCount, int, -1)

		protected:

			void validateParameters(BaseParametersPtr parameters) override;


		private :

			cv::Mat _sourceTemplateImage;
		};
	}
}
