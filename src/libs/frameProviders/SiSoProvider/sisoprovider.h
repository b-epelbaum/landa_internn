#pragma once

#include "sisoprovider_global.h"
#include "BaseFrameProvider.h"
#include "sisohelper.h"

namespace LandaJune
{
	namespace FrameProviders
	{
		static const QString SISO_PROVIDER_CONFIG_FILE = "sisoprovider.json";

		class SISOPROVIDER_EXPORT SiSoProvider : public BaseFrameProvider
		{
			Q_OBJECT
			Q_PLUGIN_METADATA(IID IFrameProvider_iid FILE "sisoprovider.json")
			Q_INTERFACES(LandaJune::FrameProviders::IFrameProvider)
		
		public:
			SiSoProvider();
			SiSoProvider(const SiSoProvider &) = delete;
			SiSoProvider(SiSoProvider &&) = delete;
			virtual ~SiSoProvider();

			const SiSoProvider & operator = (const SiSoProvider &) = delete;
			SiSoProvider & operator = (SiSoProvider &&) = delete;

			bool canContinue(FRAME_PROVIDER_ERROR lastError) override;
		
			int getRecommendedFramePoolSize() override { return std::thread::hardware_concurrency() / 2 + 1; }
			FRAME_PROVIDER_ERROR dataPreProcess(FrameRef* frameRef) override;
			FRAME_PROVIDER_ERROR dataAccess(FrameRef* frameRef) override;
			FRAME_PROVIDER_ERROR dataPostProcess(FrameRef* frameRef) override;

			FRAME_PROVIDER_ERROR init() override;
			FRAME_PROVIDER_ERROR clean() override;

			DECLARE_PROVIDER_PROPERTY(AppletFilePath, QString, "")
			DECLARE_PROVIDER_PROPERTY(ConfigurationFilePath, QString, "")
			DECLARE_PROVIDER_PROPERTY(OutputImageFormat, QString, "")
			DECLARE_PROVIDER_PROPERTY(BoardList, QStringList, {})
			DECLARE_PROVIDER_PROPERTY(BoardIndex, int, 0)

		protected:

			QString getDefaultConfigurationFileName() const override {
				return SISO_PROVIDER_CONFIG_FILE;
			}

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
