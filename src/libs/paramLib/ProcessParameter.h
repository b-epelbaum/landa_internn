#pragma once
#include "baseparam.h"

#include <QRect>

namespace LandaJune
{
	namespace Parameters
	{
		class ProcessParameter : public BaseParameter
		{
			Q_OBJECT

		public:
			ProcessParameter();
			virtual ~ProcessParameter() = default;

			DECLARE_PARAM_PROPERTY(GeneralParams, PARAM_GROUP_HEADER, {"General parameters"}, true)
			DECLARE_PARAM_PROPERTY(JobID, int, 0, true)
			DECLARE_PARAM_PROPERTY(SheetID, int, 0, true)
			DECLARE_PARAM_PROPERTY(PanelCount, int, 11, true)
			DECLARE_PARAM_PROPERTY(RootOutputFolder, QString, "C:\\temp\\june_out", true)

			DECLARE_PARAM_PROPERTY(Debug, PARAM_GROUP_HEADER, { "Debug parameters" }, true)
			DECLARE_PARAM_PROPERTY(ParalellizeCalculations, bool, false, true)
			DECLARE_PARAM_PROPERTY(DisableAllProcessing, bool, false, true)
			DECLARE_PARAM_PROPERTY(DisableAllAlgorithmProcessing, bool, false, true)
			DECLARE_PARAM_PROPERTY(DisableAllROISaving, bool, false, true)
			DECLARE_PARAM_PROPERTY(DumpLeftStrip, bool, true, true)
			DECLARE_PARAM_PROPERTY(DumpRightStrip, bool, false, true)
			DECLARE_PARAM_PROPERTY(DumpI2S, bool, true, true)
			DECLARE_PARAM_PROPERTY(DumpC2CROIs, bool, true, true)

			DECLARE_PARAM_PROPERTY(AlgorithmParams, PARAM_GROUP_HEADER, { "Algorithm parameters" }, true)
			DECLARE_PARAM_PROPERTY(ProcessRightSide, bool, true, true)
			DECLARE_PARAM_PROPERTY(GenerateOverlays, bool, true, true)
			DECLARE_PARAM_PROPERTY(CircleTemplateBuffer, QByteArray, {}, true)

			DECLARE_PARAM_PROPERTY(FGSimulator, PARAM_GROUP_HEADER, { "Frame Grabber Simulator parameters" }, true)
			DECLARE_PARAM_PROPERTY(FGS_SourceFolderPath, QString, "c:\\temp", true)
			DECLARE_PARAM_PROPERTY(FGS_SourceFilePath, QString, "", true)
			DECLARE_PARAM_PROPERTY(FGS_FrameFrequencyInMSec, int, 500, true)

			DECLARE_PARAM_PROPERTY(OfflineProvider, PARAM_GROUP_HEADER, { "Offline Handler parameters" }, true)
			DECLARE_PARAM_PROPERTY(Off_SourceFolderPath, QString, "c:\\temp\\offline", true)
			DECLARE_PARAM_PROPERTY(Off_ImageMaxCount, int, 1000, true)

			DECLARE_PARAM_PROPERTY(SisoProvider, PARAM_GROUP_HEADER, { "Silicon Software Frame grabber parameters" }, true)
			DECLARE_PARAM_PROPERTY(SISO_AppletFilePath, QString, "", true)
			DECLARE_PARAM_PROPERTY(SISO_ConfigurationFilePath, QString, "", true)
			DECLARE_PARAM_PROPERTY(SISO_OutputImageFormat, QString, "", true)
			DECLARE_PARAM_PROPERTY(SISO_BoardList, QStringList, {}, true)
			DECLARE_PARAM_PROPERTY(SISO_BoardIndex, int, 0, true)

			DECLARE_PARAM_PROPERTY(Substrate, PARAM_GROUP_HEADER, {"Substrate parameters"}, true)
			DECLARE_PARAM_PROPERTY(SubstrateWidth_mm, double, 1000, true)
			DECLARE_PARAM_PROPERTY(SubstrateHeight_mm, double, 700, true)
			DECLARE_PARAM_PROPERTY(SubstrateImageMargin_mm, double, 2, true)

			DECLARE_PARAM_PROPERTY(Pixel2MM_X, double, 0.08466683, true)
			DECLARE_PARAM_PROPERTY(Pixel2MM_Y, double, 0.08660258, true)
			
			DECLARE_PARAM_PROPERTY(ReferenceColorTriplet, COLOR_TRIPLET_SINGLE, {}, true)
			DECLARE_PARAM_PROPERTY(ScanBitDepth, int, 24, true)

			/// strip properties
			DECLARE_PARAM_PROPERTY(Strip, PARAM_GROUP_HEADER, { "Strip offsets" }, true)
			//DECLARE_PARAM_PROPERTY(OffsetFromLeftEdge_mm, double, 10.5, true)
			DECLARE_PARAM_PROPERTY(OffsetFromLeftEdge_mm, double, 2.94, true)
			DECLARE_PARAM_PROPERTY(OffsetBetweenTriangles_mm, double, 1012.5, true)
			//DECLARE_PARAM_PROPERTY(StripWidth_mm, double, 13.7, true)
			DECLARE_PARAM_PROPERTY(StripWidth_mm, double, 11.85, true)

			// I2S Triangle
			DECLARE_PARAM_PROPERTY(I2S, PARAM_GROUP_HEADER, { "I2S parameters" }, true)
			//DECLARE_PARAM_PROPERTY(I2SOffsetFromPaperEdgeX_mm, double, 5.3, true)
			DECLARE_PARAM_PROPERTY(I2SOffsetFromPaperEdgeX_mm, double, 3.19, true)
			//DECLARE_PARAM_PROPERTY(I2SOffsetFromPaperEdgeY_mm, double, 5.6, true)
			DECLARE_PARAM_PROPERTY(I2SOffsetFromPaperEdgeY_mm, double, 10.5, true)
			DECLARE_PARAM_PROPERTY(I2SMarginX_mm, double, 2.5, true)
			DECLARE_PARAM_PROPERTY(I2SMarginY_mm, double, 2.5, true)
			DECLARE_PARAM_PROPERTY(I2SROIWidth_mm, double, 8.5, true)
			DECLARE_PARAM_PROPERTY(I2SROIHeight_mm, double, 8.5, true)

			// C2C ROIs
			DECLARE_PARAM_PROPERTY(C2CROI, PARAM_GROUP_HEADER, { "C2C ROI Parameters" }, true)
			DECLARE_PARAM_PROPERTY(C2CROISetsCount, int, 5, true)
			DECLARE_PARAM_PROPERTY(C2CDistanceBetweenDots_um, double, 3048, true)
			//DECLARE_PARAM_PROPERTY(C2CDistanceBetweenSets_um, double, 155500, true)
			DECLARE_PARAM_PROPERTY(C2CDistanceBetweenSets_um, double, 159300, true)
			DECLARE_PARAM_PROPERTY(C2CDistanceFromTriangle2FirstSet_um, double, 8300, true)

			// HSV
			DECLARE_PARAM_PROPERTY(Colors, PARAM_GROUP_HEADER, { "Color Parameters" }, true)
			DECLARE_PARAM_PROPERTY(ColorArray, QVector<COLOR_TRIPLET>, {}, true)
			DECLARE_PARAM_PROPERTY(TestTriplet, COLOR_TRIPLET, {}, true)
			DECLARE_PARAM_PROPERTY(TestSingleTriplet, COLOR_TRIPLET_SINGLE, {}, true)

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
			DECLARE_PARAM_PROPERTY(NumberOfColorDotsPerLine, int, 0, false) // 367
			DECLARE_PARAM_PROPERTY(WaveTriangleROIRect, int, 0, false) 
			DECLARE_PARAM_PROPERTY(WaveROIY_px, int, 0, false) 
			DECLARE_PARAM_PROPERTY(WaveROIHeight_px, int, 0, false)
			
	
		private slots:

			void onPropertyChanged(QString strPropName);
		
		private:
			
			int toPixelsX(const double val_mmx ) const { return val_mmx / _Pixel2MM_X;}
			int toPixelsY(const double val_mmy ) const { return val_mmy / _Pixel2MM_Y;}
			
			void recalculate();
		};
	}
}


