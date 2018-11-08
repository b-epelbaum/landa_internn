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
			virtual ~ProcessParameters(){};
			explicit ProcessParameters(const QJsonObject& obj );
			ProcessParameters(const ProcessParameters& other) = default;

			DECLARE_EDITABLE_ONLY_PROPERTY(Substrate, PARAM_GROUP_HEADER, {"Substrate parameters"})
			DECLARE_NORMAL_PARAM_PROPERTY(SubstrateWidth_mm,				double,					1000	)
			DECLARE_NORMAL_PARAM_PROPERTY(SubstrateHeight_mm,				double,					700		)
			DECLARE_NORMAL_PARAM_PROPERTY(ScanStartToPaperEdgeOffset_mm,	double,					10.7	)
			DECLARE_NORMAL_PARAM_PROPERTY(LeftOffsetFromPaperEdgeX_mm,		double,					2.94	)
			DECLARE_NORMAL_PARAM_PROPERTY(LeftStripWidth_mm,				double,					13.7	)
			DECLARE_NORMAL_PARAM_PROPERTY(RightStripROIsOffsetY_mm,			double,					0.0	)
			DECLARE_NORMAL_PARAM_PROPERTY(Pixel2MM_X,						double,					0.08466683	)
			DECLARE_NORMAL_PARAM_PROPERTY(Pixel2MM_Y,						double,					0.08660258	)
			DECLARE_NORMAL_PARAM_PROPERTY(ReferenceColorTriplet,			COLOR_TRIPLET_SINGLE,	{}		)
			DECLARE_NORMAL_PARAM_PROPERTY(OfflineRegStripOnly,				bool,					false	)
			DECLARE_NORMAL_PARAM_PROPERTY(OfflineTreatEvenFilesAsRight,		bool,					false	)
			
			// I2S Triangle
			DECLARE_EDITABLE_ONLY_PROPERTY(I2S, PARAM_GROUP_HEADER, { "I2S parameters" })
			DECLARE_NORMAL_PARAM_PROPERTY(I2SOffsetFromPaperEdgeX_mm,	double,				3.19	)
			DECLARE_NORMAL_PARAM_PROPERTY(I2SOffsetFromPaperEdgeY_mm,	double,				10.5	)
			DECLARE_NORMAL_PARAM_PROPERTY(I2SWidth_mm,					double,					8.5	)
			DECLARE_NORMAL_PARAM_PROPERTY(I2SHeight_mm,					double,					8.5	)
			DECLARE_NORMAL_PARAM_PROPERTY(I2SROIMarginX_mm,				double,					2.5	)
			DECLARE_NORMAL_PARAM_PROPERTY(I2SROIMarginY_mm,				double,					2.5	)

			// C2C ROIs
			DECLARE_EDITABLE_ONLY_PROPERTY(C2CROI, PARAM_GROUP_HEADER, { "C2C ROI Parameters" })
			DECLARE_NORMAL_PARAM_PROPERTY(C2COffsetsArray, QVector<QSizeF>, {})
			DECLARE_NORMAL_PARAM_PROPERTY(C2CDistanceBetweenDotsX_mm,	double,				3.048	)
			DECLARE_NORMAL_PARAM_PROPERTY(C2CDistanceBetweenDotsY_mm,	double,				3.048	)
			DECLARE_NORMAL_PARAM_PROPERTY(C2CCircleDiameter_mm,			double,				1.542	)
			DECLARE_NORMAL_PARAM_PROPERTY(C2CROIMarginX_mm,			double,				2.5	)
			DECLARE_NORMAL_PARAM_PROPERTY(C2CROIMarginY_mm,			double,				2.5	)

			DECLARE_EDITABLE_ONLY_PROPERTY(Wave, PARAM_GROUP_HEADER, { "Wave Parameters" })
			DECLARE_NORMAL_PARAM_PROPERTY(WaveTriangleCornerX_mm,						double,				510.230	)
			DECLARE_NORMAL_PARAM_PROPERTY(WaveTriangleCornerY_mm,						double,				4.223	)
			DECLARE_NORMAL_PARAM_PROPERTY(WaveTriangleWidth_mm,							double,				8.5		)
			DECLARE_NORMAL_PARAM_PROPERTY(WaveTriangleHeight_mm,						double,				8.5		)
			DECLARE_NORMAL_PARAM_PROPERTY(WaveTriangleMarginX_mm,						double,				2.5		)
			DECLARE_NORMAL_PARAM_PROPERTY(WaveTriangleMarginY_mm,						double,				2.5		)
			DECLARE_NORMAL_PARAM_PROPERTY(WaveSideMarginsX_mm,							double,				2.0		)
			DECLARE_NORMAL_PARAM_PROPERTY(WaveCircleDiameter_mm,						double,				1.016	)
			DECLARE_NORMAL_PARAM_PROPERTY(WaveFirstLineCircleMarginY_mm,				double,				2.5		)
			DECLARE_NORMAL_PARAM_PROPERTY(WaveDistanceBetweenCircleCentersX_mm,			double,				2.709	)
			DECLARE_NORMAL_PARAM_PROPERTY(WaveDistanceBetweenCircleCentersY_mm,			double,				3.048	)
			DECLARE_NORMAL_PARAM_PROPERTY(WaveOffsetFromCornerToFirstLineCenter_mm,		double,				9.687	)

			// HSV
			DECLARE_EDITABLE_ONLY_PROPERTY(Colors, PARAM_GROUP_HEADER, { "Color Parameters" })
			DECLARE_NORMAL_PARAM_PROPERTY(ColorArray,				QVector<COLOR_TRIPLET>, {})

			///////////////////////////

			DECLARE_EDITABLE_ONLY_PROPERTY(OffLineProvider, PARAM_GROUP_HEADER, { "Offline Frame Generator parameters" })
			DECLARE_NORMAL_PARAM_PROPERTY(SourceFolderPath,			QString,			"c:\\temp")
			DECLARE_NORMAL_PARAM_PROPERTY(SourceFilePath,			QString,			""	)
			DECLARE_NORMAL_PARAM_PROPERTY(FrameFrequencyInMSec,		int,				500	)
			DECLARE_NORMAL_PARAM_PROPERTY(CycleImage,				bool,				true)
			DECLARE_NORMAL_PARAM_PROPERTY(ImageMaxCount,			int,				-1	)
			
			DECLARE_EDITABLE_ONLY_PROPERTY(SisoProvider, PARAM_GROUP_HEADER, { "Silicon Software Frame grabber parameters" })
			DECLARE_NORMAL_PARAM_PROPERTY(SISO_AppletFilePath,			QString,			""	)
			DECLARE_NORMAL_PARAM_PROPERTY(SISO_ConfigurationFilePath,	QString,			""	)
			DECLARE_NORMAL_PARAM_PROPERTY(SISO_OutputImageFormat,		QString,			""	)
			DECLARE_NORMAL_PARAM_PROPERTY(SISO_BoardList,				QStringList,		{}	)
			DECLARE_NORMAL_PARAM_PROPERTY(SISO_BoardIndex,				int,				0	)
			
			DECLARE_EDITABLE_ONLY_PROPERTY(GeneralParams, PARAM_GROUP_HEADER, {"Process General parameters"})
			DECLARE_NORMAL_PARAM_PROPERTY(JobID,						int,				0	)
			DECLARE_NORMAL_PARAM_PROPERTY(SheetID,						int,				0	)
			DECLARE_NORMAL_PARAM_PROPERTY(PanelCount,					int,				11	)
			DECLARE_NORMAL_PARAM_PROPERTY(ScanBitDepth,					int,				24	)
			DECLARE_NORMAL_PARAM_PROPERTY(RootOutputFolder,				QString,			"C:\\temp\\june_out" )
			DECLARE_SAVEABLE_ONLY_PROPERTY(FrameProviderName,			QString,			"Offline Frame Provider"	)
			DECLARE_SAVEABLE_ONLY_PROPERTY(AlgorithmRunner,				QString,			"Full Image Runner"	)
			
			DECLARE_EDITABLE_ONLY_PROPERTY(ImageProcessing, PARAM_GROUP_HEADER, { "Image processing parameters" }	)
			DECLARE_NORMAL_PARAM_PROPERTY(ParalellizeCalculations,		bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(EnableProcessing,				bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(EnableAlgorithmProcessing,	bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(ProcessLeftStrip,				bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(ProcessLeftEdge,				bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(ProcessLeftI2S,				bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(ProcessLeftC2C,				bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(ProcessRightStrip,			bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(ProcessRightEdge,				bool,				false	)
			DECLARE_NORMAL_PARAM_PROPERTY(ProcessRightI2S,				bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(ProcessRightC2C,				bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(ProcessWaveI2S,				bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(ProcessWave,					bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(GenerateOverlays,				bool,				true	)
			DECLARE_SAVEABLE_ONLY_PROPERTY(CircleTemplateResourceC2C,	QString,			"C2CCircleTemplate.tif"	)
			DECLARE_SAVEABLE_ONLY_PROPERTY(CircleTemplateResourceWave,	QString,			"WaveCircleTemplate.tif"	)
			DECLARE_CALCULATED_PROPERTY(CircleTemplateBufferC2C,		QByteArray,			{}	)
			DECLARE_CALCULATED_PROPERTY(CircleTemplateBufferWave,		QByteArray,			{}	)

			DECLARE_EDITABLE_ONLY_PROPERTY(ImageSaving, PARAM_GROUP_HEADER, { "Image/CSV saving parameters" }	)
			DECLARE_NORMAL_PARAM_PROPERTY(AsyncSaveQueueMaxSizeGB,		double,				1	)
			DECLARE_NORMAL_PARAM_PROPERTY(EnableAnyDataSaving,			bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(EnableImageSaving,			bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(EnableCSVSaving,				bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(SaveC2CRegistrationCSV,		bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(SaveI2SPlacementCSV,			bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(SaveWaveCSV,					bool,				true	)

			DECLARE_EDITABLE_ONLY_PROPERTY(SourceImages, PARAM_GROUP_HEADER, { "Source Region saving parameters" })
			DECLARE_NORMAL_PARAM_PROPERTY(SaveSourceImages,				bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(SaveSourceLeftStrip,			bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(SaveSourceLeftI2S,			bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(SaveSourceLeftC2C,			bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(SaveSourceRightStrip,			bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(SaveSourceRightI2S,			bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(SaveSourceRightC2C,			bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(SaveSourceWave,				bool,				true	)
			
			DECLARE_EDITABLE_ONLY_PROPERTY(OverlayParameters, PARAM_GROUP_HEADER, { "Overlay saving parameters" }	)
			DECLARE_NORMAL_PARAM_PROPERTY(SaveOverlayImages,			bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(SaveOverlayLeftEdge,			bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(SaveOverlayLeftI2S,			bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(SaveOverlayLeftC2C,			bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(SaveOverlayRightEdge,			bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(SaveOverlayRightI2S,			bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(SaveOverlayRightC2C,			bool,				true	)
			DECLARE_NORMAL_PARAM_PROPERTY(SaveOverlayWave,				bool,				true	)
			
			
			//-------------------------------------------------------
			// calculated values

			DECLARE_CALCULATED_PROPERTY(SubstrateParamsCalc, PARAM_GROUP_HEADER, { "Substrate values" })
			DECLARE_CALCULATED_PROPERTY(OpenCVImageFormat,					int,	0)
			DECLARE_CALCULATED_PROPERTY(ScanStartToPaperEdgeOffset_px,		int,	0	)
			DECLARE_CALCULATED_PROPERTY(LeftOffsetFromPaperEdgeX_px,		int,	0)
			
			DECLARE_CALCULATED_PROPERTY(LeftStripRect_px,					QRect, {})
			DECLARE_CALCULATED_PROPERTY(RightStripRect_px,					QRect, {})

			DECLARE_CALCULATED_PROPERTY(I2SCornerLeft_px,					QPoint, {})
			DECLARE_CALCULATED_PROPERTY(I2SCornerRight_px,					QPoint, {})

			DECLARE_CALCULATED_PROPERTY(I2SRectLeft_px,						QRect, {})
			DECLARE_CALCULATED_PROPERTY(I2SRectRight_px,					QRect, {})

			DECLARE_CALCULATED_PROPERTY(I2SMarginX_px,						int,	0)
			DECLARE_CALCULATED_PROPERTY(I2SMarginY_px,						int,	0)

			DECLARE_CALCULATED_PROPERTY(C2CMarginX_px,						int,	0)
			DECLARE_CALCULATED_PROPERTY(C2CMarginY_px,						int,	0)


			// TODO : This value is never calculated, but used in algorithm : check
			DECLARE_CALCULATED_PROPERTY(EdgeTriangleApproximateY_px, int, 0)
			
			// C2C ROIs values
			DECLARE_CALCULATED_PROPERTY(C2CROICount, int, 0 )
			DECLARE_CALCULATED_PROPERTY(C2CCircleDiameter_px, int, 0 )
			DECLARE_CALCULATED_PROPERTY(C2CROIArrayLeft_px, QVector<QRect>, {})
			DECLARE_CALCULATED_PROPERTY(C2CROIArrayRight_px, QVector<QRect>, {})

			DECLARE_CALCULATED_PROPERTY(WaveCalc, PARAM_GROUP_HEADER, { "Wave Parameters" })
			DECLARE_CALCULATED_PROPERTY(WaveTriangleCorner_px,				QPoint,		{})
			DECLARE_CALCULATED_PROPERTY(WaveTriangleROI_px,					QRect,		{})
			DECLARE_CALCULATED_PROPERTY(WaveROI_px,							QRect,		{})
			DECLARE_CALCULATED_PROPERTY(WaveTriangleMarginX_px,				int,		0) 
			DECLARE_CALCULATED_PROPERTY(WaveTriangleMarginY_px,				int,		0) 
			DECLARE_CALCULATED_PROPERTY(WaveNumberOfColorDotsPerLine,		int,		0) // 367

		protected:

			void recalculate() override {_recalculate();};
			void reset() override;
		
		private:

			void _recalculate();

			void recalculateForFullImage();
			void recalculateForOfflineLeftStrip();

			int toPixelsX(const double val_mmx ) const { return lround(val_mmx / _Pixel2MM_X);}
			int toPixelsY(const double val_mmy ) const { return lround(val_mmy / _Pixel2MM_Y);}
			
		};
	}
}


