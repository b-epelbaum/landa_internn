#pragma once
#include "baseparam.h"
#include <QRect>

namespace LandaJune
{
	namespace Parameters
	{
		class ProcessParameters : public BaseParameters
		{
			Q_OBJECT

		public:
			ProcessParameters();
			virtual ~ProcessParameters() = default;
			explicit ProcessParameters(const QJsonObject& obj );
			ProcessParameters(const ProcessParameters& other) = default;

			DECLARE_PARAM_PROPERTY(OffLineProvider, PARAM_GROUP_HEADER, { "Offline Frame Generator parameters" }, true)
			DECLARE_PARAM_PROPERTY(SourceFolderPath, QString, "c:\\temp", true)
			DECLARE_PARAM_PROPERTY(SourceFilePath, QString, "", true)
			DECLARE_PARAM_PROPERTY(FrameFrequencyInMSec, int, 500, true)
			DECLARE_PARAM_PROPERTY(CycleImage, bool, true, true)
			DECLARE_PARAM_PROPERTY(ImageMaxCount, int, 1000, true)

			DECLARE_PARAM_PROPERTY(SisoProvider, PARAM_GROUP_HEADER, { "Silicon Software Frame grabber parameters" }, true)
			DECLARE_PARAM_PROPERTY(SISO_AppletFilePath, QString, "", true)
			DECLARE_PARAM_PROPERTY(SISO_ConfigurationFilePath, QString, "", true)
			DECLARE_PARAM_PROPERTY(SISO_OutputImageFormat, QString, "", true)
			DECLARE_PARAM_PROPERTY(SISO_BoardList, QStringList, {}, true)
			DECLARE_PARAM_PROPERTY(SISO_BoardIndex, int, 0, true)


			DECLARE_PARAM_PROPERTY(GeneralParams, PARAM_GROUP_HEADER, {"Process General parameters"}, true)
			DECLARE_PARAM_PROPERTY(JobID, int, 0, true)
			DECLARE_PARAM_PROPERTY(SheetID, int, 0, true)
			DECLARE_PARAM_PROPERTY(PanelCount, int, 11, true)
			DECLARE_PARAM_PROPERTY(ScanBitDepth, int, 24, true)
			DECLARE_PARAM_PROPERTY(RootOutputFolder, QString, "C:\\temp\\june_out", true)

			DECLARE_PARAM_PROPERTY(ImageProcessing, PARAM_GROUP_HEADER, { "Image processing parameters" }, true)
			DECLARE_PARAM_PROPERTY(ParalellizeCalculations, bool, true, true)
			DECLARE_PARAM_PROPERTY(EnableProcessing, bool, true, true)
			DECLARE_PARAM_PROPERTY(EnableAlgorithmProcessing, bool, true, true)
			DECLARE_PARAM_PROPERTY(ProcessLeftStrip, bool, true, true)
			DECLARE_PARAM_PROPERTY(ProcessLeftEdge, bool, true, true)
			DECLARE_PARAM_PROPERTY(ProcessLeftI2S, bool, true, true)
			DECLARE_PARAM_PROPERTY(ProcessLeftC2C, bool, true, true)
			DECLARE_PARAM_PROPERTY(ProcessRightStrip, bool, true, true)
			DECLARE_PARAM_PROPERTY(ProcessRightEdge, bool, false, true)
			DECLARE_PARAM_PROPERTY(ProcessRightI2S, bool, true, true)
			DECLARE_PARAM_PROPERTY(ProcessRightC2C, bool, true, true)
			DECLARE_PARAM_PROPERTY(ProcessWaveI2S, bool, true, true)
			DECLARE_PARAM_PROPERTY(ProcessWave, bool, true, true)
			DECLARE_PARAM_PROPERTY(GenerateOverlays, bool, true, true)
			DECLARE_PARAM_PROPERTY(CircleTemplateResourceC2C, QString, "C2CCircleTemplate.tif", true)
			DECLARE_PARAM_PROPERTY(CircleTemplateResourceWave, QString, "WaveCircleTemplate.tif", true)
			DECLARE_PARAM_PROPERTY(CircleTemplateBufferC2C, QByteArray, {}, false)
			DECLARE_PARAM_PROPERTY(CircleTemplateBufferWave, QByteArray, {}, false)

			DECLARE_PARAM_PROPERTY(ImageSaving, PARAM_GROUP_HEADER, { "Image/CSV saving parameters" }, true)
			DECLARE_PARAM_PROPERTY(EnableAnyDataSaving, bool, true, true)
			DECLARE_PARAM_PROPERTY(EnableImageSaving, bool, true, true)
			DECLARE_PARAM_PROPERTY(EnableCSVSaving, bool, true, true)
			DECLARE_PARAM_PROPERTY(SaveC2CRegistrationCSV, bool, true, true)
			DECLARE_PARAM_PROPERTY(SaveI2SPlacementCSV, bool, true, true)
			DECLARE_PARAM_PROPERTY(SaveWaveCSV, bool, true, true)

			DECLARE_PARAM_PROPERTY(SourceImages, PARAM_GROUP_HEADER, { "Source Region saving parameters" }, true)
			DECLARE_PARAM_PROPERTY(SaveSourceImages, bool, true, true)
			DECLARE_PARAM_PROPERTY(SaveSourceLeftStrip, bool, true, true)
			DECLARE_PARAM_PROPERTY(SaveSourceLeftI2S, bool, true, true)
			DECLARE_PARAM_PROPERTY(SaveSourceLeftC2C, bool, true, true)
			DECLARE_PARAM_PROPERTY(SaveSourceRightStrip, bool, true, true)
			DECLARE_PARAM_PROPERTY(SaveSourceRightI2S, bool, true, true)
			DECLARE_PARAM_PROPERTY(SaveSourceRightC2C, bool, true, true)
			DECLARE_PARAM_PROPERTY(SaveSourceWave, bool, true, true)

			DECLARE_PARAM_PROPERTY(OverlayParameters, PARAM_GROUP_HEADER, { "Overlay saving parameters" }, true)
			DECLARE_PARAM_PROPERTY(SaveOverlayImages, bool, true, true)
			DECLARE_PARAM_PROPERTY(SaveOverlayLeftEdge, bool, true, true)
			DECLARE_PARAM_PROPERTY(SaveOverlayLeftI2S, bool, true, true)
			DECLARE_PARAM_PROPERTY(SaveOverlayLeftC2C, bool, true, true)
			DECLARE_PARAM_PROPERTY(SaveOverlayRightEdge, bool, true, true)
			DECLARE_PARAM_PROPERTY(SaveOverlayRightI2S, bool, true, true)
			DECLARE_PARAM_PROPERTY(SaveOverlayRightC2C, bool, true, true)
			DECLARE_PARAM_PROPERTY(SaveOverlayWave, bool, true, true)
			
			DECLARE_PARAM_PROPERTY(Substrate, PARAM_GROUP_HEADER, {"Sheet parameters"}, true)
			DECLARE_PARAM_PROPERTY(SubstrateWidth_mm, double, 1000, true)
			DECLARE_PARAM_PROPERTY(SubstrateHeight_mm, double, 700, true)
			DECLARE_PARAM_PROPERTY(SubstrateImageMargin_mm, double, 2, true)

			DECLARE_PARAM_PROPERTY(Pixel2MM_X, double, 0.08466683, true)
			DECLARE_PARAM_PROPERTY(Pixel2MM_Y, double, 0.08660258, true)
			
			DECLARE_PARAM_PROPERTY(ReferenceColorTriplet, COLOR_TRIPLET_SINGLE, {}, true)
			
			/// strip properties
			DECLARE_PARAM_PROPERTY(Strip, PARAM_GROUP_HEADER, { "Strip offsets" }, true)
			DECLARE_PARAM_PROPERTY(OffsetFromLeftEdge_mm, double, 2.94, true)
			DECLARE_PARAM_PROPERTY(OffsetBetweenTriangles_mm, double, 990, true)
			DECLARE_PARAM_PROPERTY(StripWidth_mm, double, 13.7, true)

			// I2S Triangle
			DECLARE_PARAM_PROPERTY(I2S, PARAM_GROUP_HEADER, { "I2S parameters" }, true)
			DECLARE_PARAM_PROPERTY(I2SOffsetFromPaperEdgeX_mm, double, 3.19, true)
			DECLARE_PARAM_PROPERTY(I2SOffsetFromPaperEdgeY_mm, double, 10.5, true)
			DECLARE_PARAM_PROPERTY(I2SMarginX_mm, double, 2.5, true)
			DECLARE_PARAM_PROPERTY(I2SMarginY_mm, double, 2.5, true)
			DECLARE_PARAM_PROPERTY(I2SROIWidth_mm, double, 8.5, true)
			DECLARE_PARAM_PROPERTY(I2SROIHeight_mm, double, 8.5, true)

			// C2C ROIs
			DECLARE_PARAM_PROPERTY(C2CROI, PARAM_GROUP_HEADER, { "C2C ROI Parameters" }, true)
			DECLARE_PARAM_PROPERTY(C2CROISetsCount, int, 5, true)
			DECLARE_PARAM_PROPERTY(C2CDistanceBetweenDots_um, double, 3048, true)
			DECLARE_PARAM_PROPERTY(C2CDistanceBetweenSets_um, double, 159300, true)
			DECLARE_PARAM_PROPERTY(C2CDistanceFromTriangle2FirstSet_um, double, 9600, true)

			// HSV
			DECLARE_PARAM_PROPERTY(Colors, PARAM_GROUP_HEADER, { "Color Parameters" }, true)
			DECLARE_PARAM_PROPERTY(ColorArray, QVector<COLOR_TRIPLET>, {}, true)

			DECLARE_PARAM_PROPERTY(Wave, PARAM_GROUP_HEADER, { "Wave Parameters" }, true)
			DECLARE_PARAM_PROPERTY(WaveTriangleApproximateX_um, double, 510230, true)
			DECLARE_PARAM_PROPERTY(WaveTriangleApproximateY_um, double, 4223, true)
			DECLARE_PARAM_PROPERTY(WaveImageMarginX_um, double, 2000, true)
			DECLARE_PARAM_PROPERTY(WaveCircleDiameter_um, double, 1016, true)
			DECLARE_PARAM_PROPERTY(WaveDistanceBetweenDotsX_um, double, 2709.3, true)
			DECLARE_PARAM_PROPERTY(WaveDistanceBetweenDotsY_um, double, 3048, true)
			DECLARE_PARAM_PROPERTY(WaveDistanceBetweenTriangleAndFirstRow_um, double, 9687, true)


			//-------------------------------------------------------
			// calculated values
			
			// Substrate values
			DECLARE_PARAM_PROPERTY(GeneralParamsCalc, PARAM_GROUP_HEADER, { "Substrate values" }, false)
			DECLARE_PARAM_PROPERTY(SubstrateWidth_px, int, 0, false)
			DECLARE_PARAM_PROPERTY(SubstrateHeight_px, int, 0, false)
			DECLARE_PARAM_PROPERTY(OpenCVImageFormat, int, 0, false)

			// Strip values
			DECLARE_PARAM_PROPERTY(StripParamsCalc, PARAM_GROUP_HEADER, { "Strip values" }, false)
			DECLARE_PARAM_PROPERTY(LeftStripRect, QRect, {}, false)
			DECLARE_PARAM_PROPERTY(RightStripRect, QRect, {}, false)


			// edge values
			DECLARE_PARAM_PROPERTY(OffsetParamsCalc, PARAM_GROUP_HEADER, { "Offset values" }, false)
			DECLARE_PARAM_PROPERTY(OffsetBetweenTriangles_px, int, 0, false)
			DECLARE_PARAM_PROPERTY(EdgeApproximateDistanceX_px, int, 0, false)
			DECLARE_PARAM_PROPERTY(EdgeTriangleApproximateY_px, int, 0, false)

						
			// I2S values
			DECLARE_PARAM_PROPERTY(I2SParamsCalc, PARAM_GROUP_HEADER, { "I2S values" }, false)
			DECLARE_PARAM_PROPERTY(I2SApproximateTriangleRectLeft, QRect, {}, false)
			DECLARE_PARAM_PROPERTY(I2SApproximateTriangleRectRight, QRect, {}, false)

			DECLARE_PARAM_PROPERTY(I2SMarginX_px, int, 0, false)
			DECLARE_PARAM_PROPERTY(I2SMarginY_px, int, 0, false)
			DECLARE_PARAM_PROPERTY(I2SROIWidth_px, int, 0, false)
			DECLARE_PARAM_PROPERTY(I2SROIHeight_px, int, 0, false)
			
			// C2C ROIs values
			DECLARE_PARAM_PROPERTY(C2CROIParamsCalc, PARAM_GROUP_HEADER, { "C2C ROI values" }, false)
			DECLARE_PARAM_PROPERTY(C2CDistanceBetweenDots_px, int, 0, false)
			DECLARE_PARAM_PROPERTY(C2CDistanceBetweenSets_px, int, 0, false)
			DECLARE_PARAM_PROPERTY(C2CDistanceFromTriangle2FirstSet_px, int, 0, false)
			DECLARE_PARAM_PROPERTY(C2CROIArrayLeft, QVector<QRect>, {}, false)
			DECLARE_PARAM_PROPERTY(C2CROIArrayRight, QVector<QRect>, {}, false)

			DECLARE_PARAM_PROPERTY(WaveCalc, PARAM_GROUP_HEADER, { "Wave Parameters" }, true)
			DECLARE_PARAM_PROPERTY(WaveROI, QRect, {}, false) 
			DECLARE_PARAM_PROPERTY(NumberOfColorDotsPerLine, int, 0, false) // 367
			DECLARE_PARAM_PROPERTY(WaveTriangleROIRect, QRect, {}, false) 

		protected:

			void recalculate() override {_recalculate();};
	
		
		private:

			void _recalculate();
			int toPixelsX(const double val_mmx ) const { return val_mmx / _Pixel2MM_X;}
			int toPixelsY(const double val_mmy ) const { return val_mmy / _Pixel2MM_Y;}
			
		};
	}
}


