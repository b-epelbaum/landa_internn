#pragma once

#include "fgSimulatorProvLib_global.h"
#include "BaseFrameProvider.h"

#include <QImageReader>

namespace LandaJune
{
	namespace FrameProviders
	{
		static const QString FG_SIMULATOR_CONFIG_FILE = "FGSimulator.json";

		class FGSIMULATOR_EXPORT FGSimulator : public BaseFrameProvider
		{
			Q_OBJECT
			Q_PLUGIN_METADATA(IID IFrameProvider_iid FILE "FGSimulator.json")
			Q_INTERFACES(LandaJune::FrameProviders::IFrameProvider)

		public:

			FGSimulator();
			FGSimulator(const FGSimulator &) = delete;
			FGSimulator(FGSimulator &&) = delete;
			virtual ~FGSimulator();

			const FGSimulator & operator = (const FGSimulator &) = delete;
			FGSimulator & operator = (FGSimulator &&) = delete;

			bool canContinue(FRAME_PROVIDER_ERROR lastError) override;

			int getRecommendedFramePoolSize() override { return 4;  }
			FRAME_PROVIDER_ERROR dataPreProcess(Core::FrameRef* frameRef) override;
			FRAME_PROVIDER_ERROR dataAccess(Core::FrameRef* frameRef) override;
			FRAME_PROVIDER_ERROR dataPostProcess(Core::FrameRef* frameRef) override;

			FRAME_PROVIDER_ERROR init() override;
			FRAME_PROVIDER_ERROR clean() override;

			DECLARE_PROVIDER_PROPERTY(SourceFolderPath, QString, "")
			DECLARE_PROVIDER_PROPERTY(SourceFilePath, QString, "")
			DECLARE_PROVIDER_PROPERTY(FrameFrequencyInMSec, int, 1000)

		protected:

			QString getDefaultConfigurationFileName() const override {
				return FG_SIMULATOR_CONFIG_FILE;
			}
		private :

			std::vector<QImage> _images;
			uint64_t _next = 0ULL;
		};
	}
}
