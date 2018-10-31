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


	_C2COffsetsArray 
			<< QSizeF(-0.3, 8.8 ) 
			<< QSizeF(-0.3, 167.8 ) 
			<< QSizeF(-0.3, 327.8 ) 
			<< QSizeF(-0.3, 487.8 ) 
			<< QSizeF(-0.3, 647.8 );

	_recalculate();
}

ProcessParameters::ProcessParameters(const QJsonObject& obj)
{
}

void ProcessParameters::reset()
{
	*this = {};
}

void ProcessParameters::_recalculate()
{
	// cleanup 
	setC2CROIArrayLeft_px({});
	setC2CROIArrayRight_px({});

	// color depth
	switch (_ScanBitDepth)
	{
		case 8:  _OpenCVImageFormat = CV_8U; break;
		case 16: _OpenCVImageFormat = CV_16U; break;
		case 24: _OpenCVImageFormat = CV_8UC3; break;
		case 32: _OpenCVImageFormat = CV_32S; break;
		default: _OpenCVImageFormat = CV_8UC3;

	}

	if (_OfflineRegStripOnly)
	{
		recalculateForOfflineLeftStrip();
	}
	else
	{
		recalculateForFullImage();
	}

	/////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	// left edge
	_LeftEdgeApproxOffsetX_px = toPixelsX(_OffsetFromLeftEdge_mm);

	////////////////////////////////////////////////////////////////////
	// i2st triangles
	
	// left triangle
	// get start point from paper edge
	QPoint topLeftPage_px = { _LeftStripRect_px.left() + toPixelsX(_OffsetFromLeftEdge_mm), 0 };

	// calculate triangle corner position relatively to start point
	_I2RectLeft_px =	{
						topLeftPage_px.x() + toPixelsX(_I2SOffsetFromPaperEdgeX_mm ),
						topLeftPage_px.y() + toPixelsX(_I2SOffsetFromPaperEdgeY_mm ),
						toPixelsX(_I2SWidth_mm),
						toPixelsY(_I2SHeight_mm),
					};

	// adjust C2C starting point with triangle corner coordinates :
	const auto C2CStartPointLeft_px = _I2RectLeft_px.topLeft();

	// and expand ROI to margins
	_I2RectLeft_px.adjust(
				-1 * toPixelsX(_I2SROIMarginX_mm),
				-1 * toPixelsY(_I2SROIMarginY_mm),
				toPixelsY(_I2SROIMarginY_mm),
				toPixelsY(_I2SROIMarginY_mm)
			);

	// right triangle
	// get start point from paper edge
	QPoint topRightPage_px = { _RightStripRect_px.left(), 0 };

	// calculate triangle corner position relatively to start point
	_I2RectRight_px =	{
						topRightPage_px.x() + toPixelsX(_I2SOffsetFromPaperEdgeX_mm ),
						topRightPage_px.y() + toPixelsX(_I2SOffsetFromPaperEdgeY_mm ),
						toPixelsX(_I2SWidth_mm),
						toPixelsY(_I2SHeight_mm),
					};

	// adjust C2C starting point with triangle corner coordinates :
	const auto C2CStartPointRight_px = _I2RectRight_px.topLeft();

	// and expand ROI to margins
	_I2RectRight_px.adjust(
				-1 * toPixelsX(_I2SROIMarginX_mm),
				-1 * toPixelsY(_I2SROIMarginY_mm),
				toPixelsY(_I2SROIMarginY_mm),
				toPixelsY(_I2SROIMarginY_mm)
			);



	////////////////////////////////////////////////////////////////////
	// C2c ROIs
	// Left side

	_C2CROICount = _C2COffsetsArray.size();

	for ( const auto& c2cOffset_mm : _C2COffsetsArray )
	{
		// define first color circle center point
		QPoint firstColorCycleCenterLeft_px = C2CStartPointLeft_px;
		firstColorCycleCenterLeft_px.setX(firstColorCycleCenterLeft_px.x() + toPixelsX(c2cOffset_mm.width()));
		firstColorCycleCenterLeft_px.setY(firstColorCycleCenterLeft_px.y() + toPixelsY(c2cOffset_mm.height()));

		_C2CROIArrayLeft_px.push_back
		(
			{
				firstColorCycleCenterLeft_px.x() - toPixelsX(_C2CDROIMarginX_mm),
				firstColorCycleCenterLeft_px.y() - toPixelsY(_C2CDROIMarginY_mm),
				toPixelsX(_C2CDistanceBetweenDotsX_mm + 2 * _C2CDROIMarginX_mm + _C2CCircleDiameter_mm),
				toPixelsY((ceil(_ColorArray.size()/2) -1) * _C2CDistanceBetweenDotsY_mm + 2 * _C2CDROIMarginY_mm + _C2CCircleDiameter_mm)
			}
		);
	}

	auto leftRightOffset_px = C2CStartPointRight_px - C2CStartPointLeft_px;
	leftRightOffset_px.setY(leftRightOffset_px.y() + toPixelsY(_RightStripROIsOffsetY_mm));

	for ( const auto& leftC2CROI_px : _C2CROIArrayLeft_px )
	{
		_C2CROIArrayRight_px.push_back (leftC2CROI_px.adjusted(leftRightOffset_px.x(), leftRightOffset_px.y(), leftRightOffset_px.x(), leftRightOffset_px.y()));
	}


	
	_I2SMarginX_px =  toPixelsX(_I2SROIMarginX_mm);
	_I2SMarginY_px =  toPixelsY(_I2SROIMarginY_mm);

	_C2CMarginX_px =  toPixelsX(_C2CDROIMarginX_mm);
	_C2CMarginY_px =  toPixelsY(_C2CDROIMarginY_mm);
		
	/////////////////////////////////////////////////
	
	//wave
	// wave I2S
	setWaveTriangleROIRect(
		{
			toPixelsX(_WaveTriangleApproximateX_um / 1000 - _I2SROIMarginX_mm),
			toPixelsY(_WaveTriangleApproximateY_um / 1000 - _I2SROIMarginY_mm ),
			toPixelsX(_I2SWidth_mm),
			toPixelsY(_I2SHeight_mm )
		}
	);

	const int32_t waveROILeft = toPixelsX(_OffsetFromLeftEdge_mm + _WaveImageMarginX_um / 1000 - _I2SROIMarginX_mm);
	const int32_t waveROIRight = toPixelsX(_OffsetFromLeftEdge_mm + _SubstrateWidth_mm - _I2SROIMarginX_mm);

	const int32_t waveRegHeight = toPixelsY((_WaveDistanceBetweenDotsY_um * ( _ColorArray.size() - 1 ) ) / 1000 + (2 * _I2SROIMarginY_mm));
	
	setWaveROI (QRect(
		waveROILeft
		, _WaveTriangleROIRect.top() + toPixelsY(_WaveDistanceBetweenTriangleAndFirstRow_um / 1000 )
		, waveROIRight - waveROILeft
		, waveRegHeight
	)
	);

	// wave dots count
	_WaveNumberOfColorDotsPerLine = (_SubstrateWidth_mm- 2 * _WaveImageMarginX_um / 1000) / (_WaveDistanceBetweenDotsX_um / 1000 );

	emit updateCalculated();
}

void ProcessParameters::recalculateForFullImage()
{
	_LeftStripRect_px =  {	
						toPixelsX(_ScanStartToPaperEdgeOffset_mm - _OffsetFromLeftEdge_mm), 
						0, 
						toPixelsX(_LeftStripWidth_mm), 
						toPixelsY(_SubstrateHeight_mm) 
					  };

	_RightStripRect_px = {
						toPixelsX(_ScanStartToPaperEdgeOffset_mm + _SubstrateWidth_mm -_OffsetFromLeftEdge_mm ),
						0,
						toPixelsX(_LeftStripWidth_mm - _OffsetFromLeftEdge_mm ),
						toPixelsY(_SubstrateHeight_mm) 
					  };

	
}
			
void ProcessParameters::recalculateForOfflineLeftStrip()
{
	_LeftStripRect_px =  {	
						0, 
						0, 
						toPixelsX(_LeftStripWidth_mm), 
						toPixelsY(_SubstrateHeight_mm) 
					  };

	_RightStripRect_px = {
						0,
						0,
						toPixelsX(_LeftStripWidth_mm - _OffsetFromLeftEdge_mm ),
						toPixelsY(_SubstrateHeight_mm) 
					  };

	
}
