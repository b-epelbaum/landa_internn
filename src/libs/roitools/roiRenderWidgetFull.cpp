#include "roiRenderWidgetFull.h"

roiRenderWidgetFull::roiRenderWidgetFull(QWidget *parent)
	: roiRenderWidgetBase(parent)
{
}

roiRenderWidgetFull::~roiRenderWidgetFull()
{
}

void roiRenderWidgetFull::cleanup()
{
	roiRenderWidgetBase::cleanup();

	if (_i2sCrossLeft != nullptr )
		delete _i2sCrossLeft;
	if (_i2sCrossRight != nullptr )
		delete _i2sCrossLeft;

	_i2sCrossLeft = nullptr;
	_i2sCrossRight = nullptr;

	if (!_c2cCrossesLeft.isEmpty())
		qDeleteAll(_c2cCrossesLeft);
	_c2cCrossesLeft.clear();

	if (!_c2cCrossesRight.isEmpty())
		qDeleteAll(_c2cCrossesRight);
	_c2cCrossesRight.clear();
}

void roiRenderWidgetFull::setROIs( bool bIsInitialROI
					, const QRect& is2sRcLeft
					, const QRect& is2sRcRight	
					, const QVector<QRect>& c2cRectsLeft
					, const QVector<QRect>& c2cRectsRight
					, QSize i2sMargins, QSize c2cMargins, int c2cCircleDiameter )
{
	_i2sROIRectLeft = is2sRcLeft;
	_i2sROIRectRight = is2sRcRight;

	_c2cROIRectsLeft = c2cRectsLeft;
	_c2cROIRectsRight = c2cRectsRight;

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

void roiRenderWidgetFull::createCrossHairs( float creationScale )
{		
	if (_i2sCrossLeft != nullptr )
		delete _i2sCrossLeft;
	if (_i2sCrossRight != nullptr )
		delete _i2sCrossRight;

	_i2sCrossLeft = nullptr;
	_i2sCrossRight = nullptr;

	if (!_c2cCrossesLeft.isEmpty())
		qDeleteAll(_c2cCrossesLeft);
	_c2cCrossesLeft.clear();

	if (!_c2cCrossesRight.isEmpty())
		qDeleteAll(_c2cCrossesRight);
	_c2cCrossesRight.clear();
	
	if (!_i2sROIRectLeft.isEmpty())
	{
		_i2sCrossLeft = new moveableLayerWidget(
				  this
				, moveableLayerWidget::CROSS_I2S
				, _i2sMarginX * 2
				, _i2sMarginY * 2
				, _c2cCircleDiameter
				, _i2sROIRectLeft.topLeft(), creationScale);
	}

	if (!_i2sROIRectRight.isEmpty())
	{
		_i2sCrossRight = new moveableLayerWidget(
				  this
				, moveableLayerWidget::CROSS_I2S
				, _i2sMarginX * 2
				, _i2sMarginY * 2
				, _c2cCircleDiameter
				, _i2sROIRectRight.topLeft(), creationScale);
	}
	
	if (!_c2cROIRectsLeft.isEmpty())
	{
		auto c2cCross = new moveableLayerWidget(this , moveableLayerWidget::CROSS_SPOT_FIRST, _c2cMarginX*2 + _c2cCircleDiameter, _c2cMarginY*2 + _c2cCircleDiameter, _c2cCircleDiameter, _c2cROIRectsLeft[0].topLeft(), creationScale);
		c2cCross->setProperty("idx", 0);
		_c2cCrossesLeft.push_back(c2cCross);


		for (auto i = 1; i < _c2cROIRectsLeft.size(); i++ )
		{
			auto c2c = new moveableLayerWidget(this , moveableLayerWidget::CROSS_SPOT_FIRST, _c2cMarginX*2 + _c2cCircleDiameter, _c2cMarginY*2 + _c2cCircleDiameter, _c2cCircleDiameter, _c2cROIRectsLeft[i].topLeft(), creationScale);
			c2c->setProperty("idx", i);
			_c2cCrossesLeft.push_back(c2c);
		}
	}

	if (!_c2cROIRectsRight.isEmpty())
	{
		auto c2cCross = new moveableLayerWidget(this , moveableLayerWidget::CROSS_SPOT_FIRST, _c2cMarginX*2 + _c2cCircleDiameter, _c2cMarginY*2 + _c2cCircleDiameter, _c2cCircleDiameter, _c2cROIRectsRight[0].topLeft(), creationScale);
		c2cCross->setProperty("idx", 10);
		_c2cCrossesRight.push_back(c2cCross);


		for (auto i = 1; i < _c2cROIRectsRight.size(); i++ )
		{
			auto c2c = new moveableLayerWidget(this , moveableLayerWidget::CROSS_SPOT_FIRST, _c2cMarginX*2 + _c2cCircleDiameter, _c2cMarginY*2 + _c2cCircleDiameter, _c2cCircleDiameter, _c2cROIRectsRight[i].topLeft(), creationScale);
			c2c->setProperty("idx", 10 + i);
			_c2cCrossesRight.push_back(c2c);
		}
	}

	_allCrossesArray << _i2sCrossLeft << _i2sCrossRight << _c2cCrossesLeft << _c2cCrossesRight;

	for ( auto elem : _allCrossesArray)
	{
		connect (elem, &moveableLayerWidget::crossMoving, this, &roiRenderWidgetBase::onCrossMoving );
		connect (elem, &moveableLayerWidget::movingOver, this, &roiRenderWidgetBase::onCrossMovingOver );
		connect (elem, &moveableLayerWidget::crossMoved, this, &roiRenderWidgetBase::onROIControlPointMoved );
	}
	updateInternalLayers();
}

void roiRenderWidgetFull::handleROIControlPointMoved(moveableLayerWidget* sender, QPoint topLeft, QPoint centerPos)
{
	// transform current TopLeft pt to original image position
	const auto newLeftToPos = fromWidget2OriginalImagePt(topLeft);
	sender->setTopLeftOnOriginalImage(newLeftToPos);

	// update rectangle for draw
	if (sender == _i2sCrossLeft )
	{
		_i2sROIRectLeft = QRect(newLeftToPos.x(), newLeftToPos.y(), _i2sROIRectLeft.width(), _i2sROIRectLeft.height());
	}
	else if (sender == _i2sCrossRight )
	{
		_i2sROIRectRight = QRect(newLeftToPos.x(), newLeftToPos.y(), _i2sROIRectLeft.width(), _i2sROIRectRight.height());
	}
	else
	{
		const auto idx = sender->property("idx").toInt();
		if (idx < 10 )
			_c2cROIRectsLeft[idx] = QRect(newLeftToPos.x(), newLeftToPos.y(), _c2cROIRectsLeft[idx].width(), _c2cROIRectsLeft[idx].height());
		else
			_c2cROIRectsRight[idx - 10] = QRect(newLeftToPos.x(), newLeftToPos.y(), _c2cROIRectsRight[idx - 10].width(), _c2cROIRectsRight[idx - 10].height());
	}

	// gather all ROI control points
	const auto roiPts = gatherROICenterPoints();

	// send them up
	emit roiChanged(roiPts);
	update();
}

void roiRenderWidgetFull::paintROIRects(std::function<void(const QRect&)> func)
{
	func(_i2sROIRectLeft);
	func(_i2sROIRectRight);
	for (auto const& rc : _c2cROIRectsLeft) 
	{
		func(rc);
	}
	for (auto const& rc : _c2cROIRectsRight) 
	{
		func(rc);
	}
}
