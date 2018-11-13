#pragma once

#include "fullFrameCycler_global.h"
#include "BaseFrameProvider.h"

#include <QImageReader>

#include <opencv2/imgcodecs.hpp>
#include <thread>

namespace LandaJune
{
	namespace FrameProviders
	{
		class FULL_FRAME_CYCLER_EXPORT fullFrameCycler : public BaseFrameProvider
		{
			Q_OBJECT
			Q_PLUGIN_METADATA(IID IFrameProvider_iid)
			Q_INTERFACES(LandaJune::FrameProviders::IFrameProvider)

		public:

			fullFrameCycler();
			fullFrameCycler(const fullFrameCycler &) = delete;
			fullFrameCycler(fullFrameCycler &&) = delete;
			virtual ~fullFrameCycler();

			const fullFrameCycler & operator = (const fullFrameCycler &) = delete;
			fullFrameCycler & operator = (fullFrameCycler &&) = delete;

			int getRecommendedFramePoolSize() override { return std::thread::hardware_concurrency();}
			bool shouldReportSkippedFrame() const override { return true; }

			CORE_ERROR prepareData(Core::FrameRef* frameRef) override;
			CORE_ERROR accessData(Core::FrameRef* frameRef) override;
			void releaseData(Core::FrameRef* frameRef) override;

			void init(BaseParametersPtr parameters, Core::ICore * coreObject, CoreEventCallback callback) override;
			void cleanup() override;

			int32_t getFrameLifeSpan() const override;

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
