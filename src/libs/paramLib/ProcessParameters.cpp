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
	_bDirty = false;
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
	_LeftOffsetFromPaperEdgeX_px = toPixelsX(_LeftOffsetFromPaperEdgeX_mm);
	_ScanStartToPaperEdgeOffset_px = toPixelsX(_ScanStartToPaperEdgeOffset_mm);

	////////////////////////////////////////////////////////////////////
	// i2st triangles
	
	// left triangle
	// get start point from paper edge
	
	_I2SCornerLeft_px = { 
							_LeftStripRect_px.left() + toPixelsX(_LeftOffsetFromPaperEdgeX_mm + _I2SOffsetFromPaperEdgeX_mm ),
							toPixelsY(_I2SOffsetFromPaperEdgeY_mm )
						};

	// calculate triangle corner position relatively to start point
	_I2SRectLeft_px =	{
							_I2SCornerLeft_px.x(),
							_I2SCornerLeft_px.y(),
							toPixelsX(_I2SWidth_mm),
							toPixelsY(_I2SHeight_mm),
					};

	// adjust C2C starting point with triangle corner coordinates :
	const auto C2CStartPointLeft_px = _I2SRectLeft_px.topLeft();

	// and expand ROI to margins
	_I2SRectLeft_px.adjust(
				-1 * toPixelsX(_I2SROIMarginX_mm),
				-1 * toPixelsY(_I2SROIMarginY_mm),
				toPixelsX(_I2SROIMarginX_mm),
				toPixelsY(_I2SROIMarginY_mm)
			);

	// right triangle
	// get start point from paper edge

	_I2SCornerRight_px = { 
							_RightStripRect_px.left() + toPixelsX(_I2SOffsetFromPaperEdgeX_mm ),
							toPixelsY(_I2SOffsetFromPaperEdgeY_mm + _RightStripROIsOffsetY_mm)
						};

	// calculate triangle corner position relatively to start point
	_I2SRectRight_px =	{
						_I2SCornerRight_px.x(),
						_I2SCornerRight_px.y(),
						toPixelsX(_I2SWidth_mm),
						toPixelsY(_I2SHeight_mm),
					};

	// adjust C2C starting point with triangle corner coordinates :
	const auto C2CStartPointRight_px = _I2SRectRight_px.topLeft();

	// and expand ROI to margins
	_I2SRectRight_px.adjust(
				-1 * toPixelsX(_I2SROIMarginX_mm),
				-1 * toPixelsY(_I2SROIMarginY_mm),
				toPixelsX(_I2SROIMarginX_mm),
				toPixelsY(_I2SROIMarginY_mm)
			);


	_C2CCircleDiameter_px = toPixelsX(_C2CCircleDiameter_mm);

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
				firstColorCycleCenterLeft_px.x() - toPixelsX( _C2CCircleDiameter_mm / 2 +_C2CROIMarginX_mm ),
				firstColorCycleCenterLeft_px.y()  - toPixelsY(_C2CCircleDiameter_mm / 2 + _C2CROIMarginY_mm),
				toPixelsX(_C2CDistanceBetweenDotsX_mm + 2 * _C2CROIMarginX_mm + _C2CCircleDiameter_mm ),
				toPixelsY((ceil( static_cast<float>(_ColorArray.size())/2) -1) * _C2CDistanceBetweenDotsY_mm + 2 * _C2CROIMarginY_mm + _C2CCircleDiameter_mm )
			}
		);
	}

	auto leftRightOffset_px = C2CStartPointRight_px - C2CStartPointLeft_px;

	for ( const auto& leftC2CROI_px : _C2CROIArrayLeft_px )
	{
		_C2CROIArrayRight_px.push_back (leftC2CROI_px.adjusted(leftRightOffset_px.x(), leftRightOffset_px.y(), leftRightOffset_px.x(), leftRightOffset_px.y()));
	}


	
	_I2SMarginX_px =  toPixelsX(_I2SROIMarginX_mm);
	_I2SMarginY_px =  toPixelsY(_I2SROIMarginY_mm);

	_C2CMarginX_px =  toPixelsX(_C2CROIMarginX_mm);
	_C2CMarginY_px =  toPixelsY(_C2CROIMarginY_mm);
		
	/////////////////////////////////////////////////
	
	// wave calculations
	// wave I2S

	_WaveTriangleMarginX_px = toPixelsX(_WaveTriangleMarginX_mm);
	_WaveTriangleMarginY_px = toPixelsY(_WaveTriangleMarginY_mm);

	_WaveTriangleCorner_px = { 
								toPixelsX(_WaveTriangleCornerX_mm),
								toPixelsY(_WaveTriangleCornerY_mm)
								};

	// calculate Triangle rectangle without margins
	_WaveTriangleROI_px = 
		{
			_WaveTriangleCorner_px.x(),
			_WaveTriangleCorner_px.y(),
			toPixelsX(_WaveTriangleWidth_mm),
			toPixelsY(_WaveTriangleHeight_mm )
		};

	const int waveROIFirstLineTop = // corner point of triangle
									_WaveTriangleROI_px.y()
									// add offset from corner top to centr of the first line
								  + toPixelsY(_WaveOffsetFromCornerToFirstLineCenter_mm)
									// subtract a circle radius and margin
								  - toPixelsY(_WaveFirstLineCircleMarginY_mm);
								
	const int32_t waveROILeft = toPixelsX(_ScanStartToPaperEdgeOffset_mm + _WaveSideMarginsX_mm );
	const int32_t waveROIRight = toPixelsX(_ScanStartToPaperEdgeOffset_mm + _SubstrateWidth_mm - _WaveSideMarginsX_mm);
	const int32_t waveRegHeight = toPixelsY( _WaveDistanceBetweenCircleCentersY_mm * ( _ColorArray.size() - 1 ) + _WaveCircleDiameter_mm + 2 * _WaveFirstLineCircleMarginY_mm );

	// expand Wave triangle to ROI margins
		_WaveTriangleROI_px = {
								_WaveTriangleROI_px.x() - _WaveTriangleMarginX_px,
								_WaveTriangleROI_px.y() - _WaveTriangleMarginY_px,
								toPixelsX(_WaveTriangleWidth_mm + 2 * _WaveTriangleMarginX_mm ),
								toPixelsY(_WaveTriangleHeight_mm + 2 * _WaveTriangleMarginY_mm )
						};

	// build wave ROI
	_WaveROI_px =
	{
		waveROILeft,
		waveROIFirstLineTop,
		0,
		waveRegHeight
	};

	_WaveROI_px.setRight(waveROIRight);
	
	// wave dots count
	_WaveNumberOfColorDotsPerLine = lround((static_cast<float>(_WaveROI_px.width()) - static_cast<float>(toPixelsX(2 * static_cast<float>(_WaveSideMarginsX_mm) + (float)_WaveCircleDiameter_mm)) ) / (float)toPixelsX(_WaveDistanceBetweenCircleCentersX_mm));

	sortColorArray();
	emit updateCalculated();
}

void ProcessParameters::recalculateForFullImage()
{
	_LeftStripRect_px =  {	
						toPixelsX(_ScanStartToPaperEdgeOffset_mm - _LeftOffsetFromPaperEdgeX_mm), 
						0, 
						toPixelsX(_LeftStripWidth_mm), 
						toPixelsY(_SubstrateHeight_mm) 
					  };

	_RightStripRect_px = {
						toPixelsX(_ScanStartToPaperEdgeOffset_mm + _SubstrateWidth_mm - _LeftStripWidth_mm  + _LeftOffsetFromPaperEdgeX_mm ),
						0,
						toPixelsX(_LeftStripWidth_mm - _LeftOffsetFromPaperEdgeX_mm ),
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
						toPixelsX(_LeftStripWidth_mm - _LeftOffsetFromPaperEdgeX_mm ),
						toPixelsY(_SubstrateHeight_mm) 
					  };

	
}

void ProcessParameters::sortColorArray()
{
	std::sort(_ColorArray.begin(), _ColorArray.end(),   
			[](const COLOR_TRIPLET& left, const COLOR_TRIPLET& right) 
			{
				auto lV (left.ColorName().toStdString());
				auto rV (right.ColorName().toStdString());

				std::transform(lV.begin(), lV.end(), lV.begin(), ::tolower);
				std::transform(rV.begin(), rV.end(), rV.begin(), ::tolower);

				auto const lIt = colorOrderMap.find(lV);
				auto const rIt = colorOrderMap.find(rV);

				return lIt != colorOrderMap.end() && rIt != colorOrderMap.end() 
								? colorOrderMap.find(lV)->second <  colorOrderMap.find(rV)->second 
								: false;
			});	
}