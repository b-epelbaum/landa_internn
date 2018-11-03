#include "roiRenderWidgetStrip.h"

roiRenderWidgetStrip::roiRenderWidgetStrip(QWidget *parent)
	: roiRenderWidgetBase(parent)
{
}

roiRenderWidgetStrip::~roiRenderWidgetStrip()
{
}

void roiRenderWidgetStrip::cleanup()
{
	roiRenderWidgetBase::cleanup();

	if (_i2sCross != nullptr )
		delete _i2sCross;

	_i2sCross = nullptr;

	if (!_c2cCrosses.isEmpty())
		qDeleteAll(_c2cCrosses);

	_c2cCrosses.clear();
}

void roiRenderWidgetStrip::setROIs(bool bIsInitialROI, const QRect& is2sRc, const QVector<QRect>& c2cRects, QSize i2sMargins, QSize c2cMargins, int c2cCircleDiameter )
{
	_i2sROIRect = is2sRc;
	_c2cROIRects = c2cRects;

	_i2sMarginX = i2sMargins.width();
	_i2sMarginY = i2sMargins.height();

	_c2cMarginX = c2cMargins.width();
	_c2cMarginY = c2cMargins.height();

	_c2cCircleDiameter = c2cCircleDiameter;

	if(_hasImage )
	{
		if (!bIsInitialROI)
			createCrossHairs (_imageScaleRatio);
		update();
	}
}

void roiRenderWidgetStrip::createCrossHairs( float creationScale )
{		
	if (_i2sCross != nullptr )
		delete _i2sCross;
	_i2sCross = nullptr;
	if (!_c2cCrosses.isEmpty())
		qDeleteAll(_c2cCrosses);
	_c2cCrosses.clear();
	_allCrossesArray.clear();
	
	if (!_i2sROIRect.isEmpty())
	{
		_i2sCross = new moveableLayerWidget(
				  this
				, moveableLayerWidget::CROSS_I2S
				, _i2sMarginX * 2
				, _i2sMarginY * 2
				, _c2cCircleDiameter
				, _i2sROIRect.topLeft(), creationScale);
	}
	
	if (!_c2cROIRects.isEmpty())
	{
		auto c2cCross = new moveableLayerWidget(this , moveableLayerWidget::CROSS_SPOT_FIRST, _c2cMarginX*2 + _c2cCircleDiameter, _c2cMarginY*2 + _c2cCircleDiameter, _c2cCircleDiameter, _c2cROIRects[0].topLeft(), creationScale);
		c2cCross->setProperty("idx", 0);
		_c2cCrosses.push_back(c2cCross);


		for (auto i = 1; i < _c2cROIRects.size(); i++ )
		{
			auto c2c = new moveableLayerWidget(this , moveableLayerWidget::CROSS_SPOT_FIRST, _c2cMarginX*2 + _c2cCircleDiameter, _c2cMarginY*2 + _c2cCircleDiameter, _c2cCircleDiameter, _c2cROIRects[i].topLeft(), creationScale);
			c2c->setProperty("idx", i);
			_c2cCrosses.push_back(c2c);
		}
	}

	_allCrossesArray << _i2sCross << _c2cCrosses;

	for ( auto elem : _allCrossesArray)
	{
		connect (elem, &moveableLayerWidget::crossMoving, this, &roiRenderWidgetBase::onCrossMoving );
		connect (elem, &moveableLayerWidget::movingOver, this, &roiRenderWidgetBase::onCrossMovingOver );
		connect (elem, &moveableLayerWidget::crossMoved, this, &roiRenderWidgetBase::onROIControlPointMoved );
	}
	updateInternalLayers();
}

void roiRenderWidgetStrip::handleROIControlPointMoved(moveableLayerWidget* sender, QPoint topLeft, QPoint centerPos)
{
	// transform current TopLeft pt to original image position
	const auto newLeftToPos = fromWidget2OriginalImagePt(topLeft);
	sender->setTopLeftOnOriginalImage(newLeftToPos);

	// update rectangle for draw
	if (sender == _i2sCross )
		_i2sROIRect = QRect(newLeftToPos.x(), newLeftToPos.y(), _i2sROIRect.width(), _i2sROIRect.height());
	else
	{
		const auto idx = sender->property("idx").toInt();
		_c2cROIRects[idx] = QRect(newLeftToPos.x(), newLeftToPos.y(), _c2cROIRects[idx].width(), _c2cROIRects[idx].height());
	}

	// gather all ROI control points
	const auto roiPts = gatherROICenterPoints();

	// send them up
	emit roiChanged(roiPts);
	update();
}

void roiRenderWidgetStrip::paintROIRects(std::function<void(const QRect&)> func)
{
	func(_i2sROIRect);
	for (auto const& rc : _c2cROIRects) 
	{
		func(rc);
	}
}
