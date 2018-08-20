#include "ProcessParameter.h"
#include <opencv2/imgcodecs.hpp>
#include <thread>

using namespace LandaJune::Parameters;

ProcessParameter::ProcessParameter()
{
	connect(this, &BaseParameter::propertyChanged, this, &ProcessParameter::onPropertyChanged);
	recalculate();
}

void ProcessParameter::onPropertyChanged(QString strPropName)
{
	(void)strPropName;
	recalculate();
}

void ProcessParameter::recalculate()
{
	// cleanup 
	_HSVARRAY.clear();
	_C2CROIArrayLeft.clear();
	_C2CROIArrayRight.clear();

	// general parameters
	

	// sheet dimensions
	_SubstrateWidth_px = toPixelsX(_SubstrateWidth_mm + _OffsetFromLeftEdge_mm * 2 );
	_SubstrateHeight_px = toPixelsY(_SubstrateHeight_mm);
	_OffsetBetweenTriangles_px = toPixelsX(_OffsetBetweenTriangles_mm);

	// color depth
	switch (_BitDepth)
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

	// ROI colors
	COLOR_TRIPLET_SINGLE color1_min = { 0, 170, 0, "cyan" };
	COLOR_TRIPLET_SINGLE color2_min = { 80, 170, 0, "yellow" };
	COLOR_TRIPLET_SINGLE color3_min = { 110, 170, 0, "magenta" };
	COLOR_TRIPLET_SINGLE color4_min = { 0, 0, 0, "black" };

	COLOR_TRIPLET_SINGLE color1_max = { 30, 255, 255, "cyan" };
	COLOR_TRIPLET_SINGLE color2_max = { 110, 255, 255, "yellow" };
	COLOR_TRIPLET_SINGLE color3_max = { 140, 255, 255, "magenta" };
	COLOR_TRIPLET_SINGLE color4_max = { 255, 100, 128, "black" };


	_HSVARRAY 
		<< COLOR_TRIPLET{color1_min, color1_max}
		<< COLOR_TRIPLET{ color2_min, color2_max }
		<< COLOR_TRIPLET{ color3_min, color3_max }
		<< COLOR_TRIPLET{ color4_min, color4_max };

	// C2c ROIs
	_C2CDistanceBetweenDots_px = toPixelsY(_C2CDistanceBetweenDots_um / 1000);
	_C2CDistanceBetweenSets_px = toPixelsY(_C2CDistanceBetweenSets_um / 1000);
	_C2CDistanceFromTriangle2FirstSet_px = toPixelsY(_C2CDistanceFromTriangle2FirstSet_um / 1000);

	const QRect Roi0L =
	{
		_I2SApproximateTriangleRectLeft.left(),
		_I2SApproximateTriangleRectLeft.top() + _C2CDistanceFromTriangle2FirstSet_px,
		_I2SROIWidth_px,
		static_cast<int>((ceil(_ColorNumber / 2) - 1) * _C2CDistanceBetweenDots_px) + 2 *_I2SMarginY_px
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
}
