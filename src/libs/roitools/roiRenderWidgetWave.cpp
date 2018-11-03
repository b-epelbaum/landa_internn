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

	if (_waveTriangleCrossLeft != nullptr )
		delete _waveTriangleCrossLeft;
	_waveTriangleCrossLeft = nullptr;
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
	if (_waveTriangleCrossLeft != nullptr )
		delete _waveTriangleCrossLeft;
	
	_waveTriangleCrossLeft = nullptr;
	_allCrossesArray.clear();

	if (!_waveTriangleROIRc.isEmpty())
	{
		_waveTriangleCrossLeft = new moveableLayerWidget(
				  this
				, moveableLayerWidget::CROSS_I2S
				, _waveTriangleMarginX * 2
				, _waveTriangleMarginY * 2
				, 0
				, _waveTriangleROIRc.topLeft(), creationScale);
	}

	_allCrossesArray << _waveTriangleCrossLeft;
	for ( auto elem : _allCrossesArray)
	{
		connect (elem, &moveableLayerWidget::crossMoving, this, &roiRenderWidgetBase::onCrossMoving );
		connect (elem, &moveableLayerWidget::movingOver, this, &roiRenderWidgetBase::onCrossMovingOver );
		connect (elem, &moveableLayerWidget::crossMoved, this, &roiRenderWidgetBase::onROIControlPointMoved );
	}
	updateInternalLayers();
}

void roiRenderWidgetWave::handleROIControlPointMoved(moveableLayerWidget* sender, QPoint topLeft, QPoint centerPos)
{
	// transform current TopLeft pt to original image position
	const auto newLeftToPos = fromWidget2OriginalImagePt(topLeft);
	sender->setTopLeftOnOriginalImage(newLeftToPos);

	// update rectangle for draw
	if (sender == _waveTriangleCrossLeft )
	{
		_waveTriangleROIRc = QRect(newLeftToPos.x(), newLeftToPos.y(), _waveTriangleROIRc.width(), _waveTriangleROIRc.height());
	}

	// gather all ROI control points
	const auto roiPts = gatherROICenterPoints();

	// send them up
	emit roiChanged(roiPts);
	update();
}

void roiRenderWidgetWave::paintROIRects(std::function<void(const QRect&)> func)
{
	func(_waveTriangleROIRc);
	func(_waveROIRc);
}
