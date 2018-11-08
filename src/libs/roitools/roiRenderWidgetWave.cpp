#include "roiRenderWidgetWave.h"

roiRenderWidgetWave::roiRenderWidgetWave(QWidget *parent)
	: roiRenderWidgetBase(parent)
{
}

roiRenderWidgetWave::~roiRenderWidgetWave()
{
}

void roiRenderWidgetWave::cleanup()
{
	roiRenderWidgetBase::cleanup();
	cleanLocalObjects();
}

void roiRenderWidgetWave::cleanLocalObjects()
{
	if (_waveTriangleCross != nullptr )
		delete _waveTriangleCross;
	_waveTriangleCross = nullptr;
	_allCrossesArray.clear();
}

void roiRenderWidgetWave::setROIs(   bool bIsInitialROI
					, const QRect& waveTriangleRc
					, const QRect& waveROI	
					, QSize waveTriangleMargins )
{
	_waveTriangleROIRc = waveTriangleRc;
	_waveROIRc = waveROI;
	_waveTriangleMarginX = waveTriangleMargins.width();
	_waveTriangleMarginY = waveTriangleMargins.height();

	if(_hasImage )
	{
		if (!bIsInitialROI)
			createCrossHairs (_imageScaleRatio);
		update();
	}
}

void roiRenderWidgetWave::createCrossHairs( float creationScale )
{		
	cleanLocalObjects();

	if (!_waveTriangleROIRc.isEmpty())
	{
		_waveTriangleCross = new moveableLayerWidget(
				  this
				, moveableLayerWidget::CROSS_I2S
				, _waveTriangleMarginX * 2
				, _waveTriangleMarginY * 2
				, 0
				, _waveTriangleROIRc.topLeft()
				, I2S_COLOR
				, SOLID_PEN
				, creationScale);
	}

	_allCrossesArray << _waveTriangleCross;
	for ( auto elem : _allCrossesArray)
	{
		if ( elem)
		{
			connect (elem, &moveableLayerWidget::crossMoving, this, &roiRenderWidgetBase::onCrossMoving );
			connect (elem, &moveableLayerWidget::movingOver, this, &roiRenderWidgetBase::onCrossMovingOver );
			connect (elem, &moveableLayerWidget::crossMoved, this, &roiRenderWidgetBase::onROIControlPointMoved );
		}
	}
	updateInternalLayers();
}

void roiRenderWidgetWave::handleROIControlPointMoved(moveableLayerWidget* sender, QPoint topLeft, QPoint centerPos)
{
	// transform current TopLeft pt to original image position
	const auto newLeftToPos = fromWidget2OriginalImagePt(topLeft);
	sender->setTopLeftOnOriginalImage(newLeftToPos);

	// update rectangle for draw
	if (sender == _waveTriangleCross )
	{
		_waveTriangleROIRc = QRect(newLeftToPos.x(), newLeftToPos.y(), _waveTriangleROIRc.width(), _waveTriangleROIRc.height());
	}

	// send them up
	emit waveTriangleChanged(fromWidget2OriginalImagePt(centerPos));
	update();
}

void roiRenderWidgetWave::paintROIRects( glDrawFunc func )
{
	func(_waveTriangleROIRc, true, I2S_COLOR_FRAME, false, JGL_NO_COLOR);
	func(_waveROIRc, true, C2C_COLOR_FRAME, false, JGL_NO_COLOR);
}
