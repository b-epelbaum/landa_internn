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

			int getRecommendedFramePoolSize() override { return std::thread::hardware_concurrency();  }
			CORE_ERROR prepareData(Core::FrameRef* frameRef) override;
			CORE_ERROR accessData(Core::FrameRef* frameRef) override;
			void releaseData(Core::FrameRef* frameRef) override;

			CORE_ERROR init() override;
			CORE_ERROR cleanup() override;

			DECLARE_PARAM_PROPERTY(SourceFilePath, QString, "", true)
			DECLARE_PARAM_PROPERTY(FrameFrequencyInMSec, int, 1000, true)
			DECLARE_PARAM_PROPERTY(ImageMaxCount, int, -1, true)

		protected:

			void validateParameters(std::shared_ptr<Parameters::BaseParameters> parameters) override;


		private :

			cv::Mat _sourceImage;
		};
	}
}
