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

			const cyclicGenerator & operator = (const cyclicGenerator &) = delete;
			cyclicGenerator & operator = (cyclicGenerator &&) = delete;

			bool canContinue(CORE_ERROR lastError) override;

			int32_t getFrameLifeSpan() const override;
			int getRecommendedFramePoolSize() override { return std::thread::hardware_concurrency();  }
			CORE_ERROR prepareData(Core::FrameRef* frameRef) override;
			CORE_ERROR accessData(Core::FrameRef* frameRef) override;
			void releaseData(Core::FrameRef* frameRef) override;

			CORE_ERROR init(std::shared_ptr<Parameters::BaseParameters> parameters) override;
			CORE_ERROR cleanup() override;

			DECLARE_NORMAL_PARAM_PROPERTY(SourceFilePath, QString, "")
			DECLARE_NORMAL_PARAM_PROPERTY(FrameFrequencyInMSec, int, 1000)
			DECLARE_NORMAL_PARAM_PROPERTY(ImageMaxCount, int, -1)

		protected:

			void validateParameters(std::shared_ptr<Parameters::BaseParameters> parameters) override;


		private :

			cv::Mat _sourceTemplateImage;
		};
	}
}
