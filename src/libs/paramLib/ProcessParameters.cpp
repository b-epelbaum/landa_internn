#include "ProcessParameters.h"
#include <opencv2/imgcodecs.hpp>
#include <thread>

#include <QFile>

using namespace LandaJune::Parameters;

ProcessParameters::ProcessParameters()
{
	setReferenceColorTriplet ( {0,0,0,"Black"});

	// ROI colors
	const COLOR_TRIPLET_SINGLE color1_min = { 80, 90, 170, "Cyan" };
	const COLOR_TRIPLET_SINGLE color2_min = { 10, 90, 170, "Yellow" };
	const COLOR_TRIPLET_SINGLE color3_min = { 135, 90, 170, "Magenta" };
	const COLOR_TRIPLET_SINGLE color4_min = { 0, 0, 0, "Black" };

	const COLOR_TRIPLET_SINGLE color1_max = { 120, 255, 255, "Cyan" };
	const COLOR_TRIPLET_SINGLE color2_max = { 40, 255, 255, "Yellow" };
	const COLOR_TRIPLET_SINGLE color3_max = { 179, 255, 255, "Magenta" };
	const COLOR_TRIPLET_SINGLE color4_max = { 255, 90, 140, "Black" };


	_ColorArray
		<< COLOR_TRIPLET{ color1_min, color1_max, "Cyan" }
		<< COLOR_TRIPLET{ color2_min, color2_max, "Yellow" }
		<< COLOR_TRIPLET{ color3_min, color3_max, "Magenta" }
		<< COLOR_TRIPLET{ color4_min, color4_max, "Black" };


	_recalculate();
}

ProcessParameters::ProcessParameters(const QJsonObject& obj)
{
}

void ProcessParameters::_recalculate()
{
	// cleanup 
	setC2CROIArrayLeft({});
	setC2CROIArrayRight({});

	// sheet dimensions
	_SubstrateWidth_px = toPixelsX(_SubstrateWidth_mm + _OffsetFromLeftEdge_mm * 2 );
	_SubstrateHeight_px = toPixelsY(_SubstrateHeight_mm);
	_OffsetBetweenTriangles_px = toPixelsX(_OffsetBetweenTriangles_mm);

	// color depth
	switch (_ScanBitDepth)
	{
		case 8: _OpenCVImageFormat = CV_8U; break;
		case 16: _OpenCVImageFormat = CV_16U; break;
		case 24: _OpenCVImageFormat = CV_8UC3; break;
		case 32: _OpenCVImageFormat = CV_32S; break;
			default: _OpenCVImageFormat = CV_8UC3;

	}

	_LeftStripRect =  {	0, 
						0, 
						toPixelsX(_OffsetFromLeftEdge_mm + _StripWidth_mm), 
						toPixelsY(_SubstrateHeight_mm) 
					  };

	_RightStripRect = _LeftStripRect.translated(_OffsetBetweenTriangles_px, 0);

	// edges
	_EdgeApproximateDistanceX_px = toPixelsX(_OffsetFromLeftEdge_mm);

	// i2s
	// Triangles
	_I2SMarginX_px = toPixelsX(_I2SMarginX_mm);
	_I2SMarginY_px = toPixelsY(_I2SMarginY_mm);
	_I2SROIWidth_px = toPixelsX(_I2SROIWidth_mm);
	_I2SROIHeight_px = toPixelsY(_I2SROIHeight_mm);

	_I2SApproximateTriangleRectLeft =
	{
			toPixelsX(_OffsetFromLeftEdge_mm + _I2SOffsetFromPaperEdgeX_mm - _I2SMarginX_mm),
			toPixelsY(_I2SOffsetFromPaperEdgeY_mm - _I2SMarginY_mm),
			toPixelsX(_I2SROIWidth_mm),
			toPixelsY(_I2SROIHeight_mm)
	};

	_I2SApproximateTriangleRectRight = _I2SApproximateTriangleRectLeft.translated(_OffsetBetweenTriangles_px, 0);

	
	// C2c ROIs
	_C2CDistanceBetweenDots_px = toPixelsY(_C2CDistanceBetweenDots_um / 1000);
	_C2CDistanceBetweenSets_px = toPixelsY(_C2CDistanceBetweenSets_um / 1000);
	_C2CDistanceFromTriangle2FirstSet_px = toPixelsY(_C2CDistanceFromTriangle2FirstSet_um / 1000);

	const QRect Roi0L =
	{
		_I2SApproximateTriangleRectLeft.left(),
		_I2SApproximateTriangleRectLeft.top() + _C2CDistanceFromTriangle2FirstSet_px,
		_I2SROIWidth_px,
		static_cast<int>((ceil(_ColorArray.size() / 2) ) * _C2CDistanceBetweenDots_px) + 2 *_I2SMarginY_px
	};

	_C2CROIArrayLeft << Roi0L;
	for (auto i = 1; i < _C2CROISetsCount; i++)
	{
		_C2CROIArrayLeft <<	Roi0L.translated(0, _C2CDistanceBetweenSets_px * i);
	}

	for (auto i = 0; i < _C2CROISetsCount; i++)
	{
		_C2CROIArrayRight << _C2CROIArrayLeft[i].translated(_OffsetBetweenTriangles_px, 0);
	}

	//wave
	// wave I2S
	setWaveTriangleROIRect(QRect(
		 toPixelsX(_WaveTriangleApproximateX_um / 1000 - _I2SMarginX_mm)
		,toPixelsY(_WaveTriangleApproximateY_um / 1000 - _I2SMarginY_mm )
		, _I2SROIWidth_px
		, _I2SROIHeight_px )
	);

	const int32_t waveROILeft = toPixelsX(_OffsetFromLeftEdge_mm + _WaveImageMarginX_um / 1000 - _I2SMarginX_mm);
	const int32_t waveROIRight = toPixelsX(_OffsetFromLeftEdge_mm + _SubstrateWidth_mm - _I2SMarginX_mm);

	const int32_t waveRegHeight = toPixelsY((_WaveDistanceBetweenDotsY_um * ( _ColorArray.size() - 1 ) ) / 1000 + (2 * _I2SMarginY_mm));
	
	setWaveROI (QRect(
		waveROILeft
		, _WaveTriangleROIRect.top() + toPixelsY(_WaveDistanceBetweenTriangleAndFirstRow_um / 1000 )
		, waveROIRight - waveROILeft
		, waveRegHeight
	)
	);

	// wave dots count
	_NumberOfColorDotsPerLine = (_SubstrateWidth_mm- 2 * _WaveImageMarginX_um / 1000) / (_WaveDistanceBetweenDotsX_um / 1000 );

	emit updateCalculated();
}
