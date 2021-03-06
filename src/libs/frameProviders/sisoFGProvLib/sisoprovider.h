#pragma once

#include "sisoFGProvLib_global.h"
#include "BaseFrameProvider.h"
#include "sisohelper.h"

#include <thread>

namespace LandaJune
{
	namespace FrameProviders
	{
		class SISOPROVIDER_EXPORT SiSoProvider : public BaseFrameProvider
		{
			Q_OBJECT
			Q_PLUGIN_METADATA(IID IFrameProvider_iid)
			Q_INTERFACES(LandaJune::FrameProviders::IFrameProvider)
		
		public:
			SiSoProvider();
			SiSoProvider(const SiSoProvider &) = delete;
			SiSoProvider(SiSoProvider &&) = delete;
			virtual ~SiSoProvider();

			const SiSoProvider & operator = (const SiSoProvider &) = delete;
			SiSoProvider & operator = (SiSoProvider &&) = delete;

			int32_t getFrameLifeSpan() const override;
			bool shouldReportSkippedFrame() const override { return true;}

			int getRecommendedFramePoolSize() override { return std::thread::hardware_concurrency() / 2 + 1; }
			CORE_ERROR prepareData(Core::FrameRef* frameRef) override;
			CORE_ERROR accessData(Core::FrameRef* frameRef) override;
			void releaseData(Core::FrameRef* frameRef) override;


			void init(BaseParametersPtr parameters, Core::ICore * coreObject, CoreEventCallback callback) override;
			void cleanup() override;

			DECLARE_NORMAL_PARAM_PROPERTY(AppletFilePath, QString, "")
			DECLARE_NORMAL_PARAM_PROPERTY(ConfigurationFilePath, QString, "")
			DECLARE_NORMAL_PARAM_PROPERTY(OutputImageFormat, QString, "")
			DECLARE_NORMAL_PARAM_PROPERTY(BoardList, QStringList, {})
			DECLARE_NORMAL_PARAM_PROPERTY(BoardIndex, int, 0)

		protected:

			void validateParameters(BaseParametersPtr parameters) override;

		private:

			void detectBoards();
			void createDisplay(int imageFormat, int width, int height);
			void dumpTestImageSync(uint8_t * imageBits, int width, int height, int imageCVFormat) const;

#ifdef ENABLE_FGRAB
			Fg_Struct * _fg = nullptr;
			int _camPort1 = PORT_A;
			int _camPort2 = PORT_B;
			int _camPort3 = PORT_C;

			frameindex_t _nrOfPicturesToGrab = 1000;
			frameindex_t _nbBuffers = 16;
#endif
			unsigned int _width1 = 0;
			unsigned int _height1 = 0;

			unsigned int _width2 = 0;
			unsigned int _height2 = 0;

			unsigned int _width3 = 0;
			unsigned int _height3 = 0;


			int _samplePerPixel = 1;
			size_t _bytePerSample = 1;

			int _format1 = 0, _format2 = 0, _format3 = 0;

			void * _imgBuf1 = nullptr;
			void * _imgBuf2 = nullptr;
			void * _imgBuf3 = nullptr;

			int _displayId = -1;
		};
	}
}
