#include "roiRenderWidgetFull.h"

roiRenderWidgetFull::roiRenderWidgetFull(QWidget *parent)
	: roiRenderWidgetBase(parent)
{
}

roiRenderWidgetFull::~roiRenderWidgetFull()
= default;

void roiRenderWidgetFull::cleanLocalObjects()
{
	if (_i2sCrossLeft != nullptr )
		delete _i2sCrossLeft;
	_i2sCrossLeft = nullptr;

	if (_leftStripLine != nullptr )
		delete _leftStripLine;
	_leftStripLine = nullptr;

	if (_rightStripLine != nullptr )
		delete _rightStripLine;
	_rightStripLine = nullptr;

	if (_pageStripLine != nullptr )
		delete _pageStripLine;
	_pageStripLine = nullptr;

	if (!_c2cCrossesLeft.isEmpty())
		qDeleteAll(_c2cCrossesLeft);


	_c2cCrossesLeft.clear();
	_allCrossesArray.clear();
}

void roiRenderWidgetFull::cleanup()
{
	roiRenderWidgetBase::cleanup();
	cleanLocalObjects();
}

void roiRenderWidgetFull::setROIs( bool bInitROI
				, const QRect& is2sRcLeft
				, const QRect& is2sRcRight
				, const QVector<QRect>& c2cRectsLeft
				, const QVector<QRect>& c2cRectsRight
				, QRect leftStripRect
				, QRect rightStripRect
				, int pageOffsetX
				, QSize i2sMargins
				, QSize c2cMargins
				, int c2cCircleDiameter
				, bool updateBoth )
{
	if (updateBoth )
	{
		_i2sROIRectLeft = is2sRcLeft;
		_c2cROIRectsLeft = c2cRectsLeft;
	}
	
	_leftStripRect = leftStripRect;
	_rightStripRect = rightStripRect;
	_pageOffsetX = pageOffsetX;

	_i2sROIRectRight = is2sRcRight;
	_c2cROIRectsRight = c2cRectsRight;

	_i2sMarginX = i2sMargins.width();
	_i2sMarginY = i2sMargins.height();

	_c2cMarginX = c2cMargins.width();
	_c2cMarginY = c2cMargins.height();
	_c2cCircleDiameter = c2cCircleDiameter;

	_bI2SMoved = false;
	if(_hasImage )
	{
		if (!bInitROI )
			createCrossHairs (_imageScaleRatio);
		update();
	}
}

void roiRenderWidgetFull::createCrossHairs( float creationScale )
{		
	cleanLocalObjects();

	if (!_i2sROIRectLeft.isEmpty())
	{
		_i2sCrossLeft = new moveableLayerWidget(
				  this
				, moveableLayerWidget::CROSS_I2S
				, _i2sMarginX * 2
				, _i2sMarginY * 2
				, _c2cCircleDiameter
				, _i2sROIRectLeft.topLeft()
				, I2S_COLOR
				, SOLID_PEN
				, creationScale);
		_allCrossesArray << _i2sCrossLeft;
	}

	if ( !_leftStripRect.isEmpty())
	{
		_leftStripLine = new moveableLayerWidget(
					  this
					, moveableLayerWidget::CROSS_EDGE
					, 4
					, _leftStripRect.height()
					, _c2cCircleDiameter
					, QPoint(_leftStripRect.x(), 0)
					, STRIP_MARGIN_COLOR
					, SOLID_PEN
					, creationScale);

		_rightStripLine = new moveableLayerWidget(
					  this
					, moveableLayerWidget::CROSS_EDGE
					, 4
					, _leftStripRect.height()
					, _c2cCircleDiameter
					, QPoint(_leftStripRect.x() + _leftStripRect.width(), 0)
					, STRIP_MARGIN_COLOR
					, SOLID_PEN
					, creationScale);

		_allCrossesArray << _leftStripLine << _rightStripLine;
	}
	
	if ( _pageOffsetX != 0)
	{
		_pageStripLine = new moveableLayerWidget(
					  this
					, moveableLayerWidget::CROSS_EDGE
					, 4
					, _imageSize.height()
					, _c2cCircleDiameter
					, QPoint(_pageOffsetX, 0)
					, PAGE_OFFSET_COLOR
					, DOT_PEN
					,  creationScale);

		_allCrossesArray << _pageStripLine;
	}

	
	if (!_c2cROIRectsLeft.isEmpty())
	{
		auto c2cCross = new moveableLayerWidget(
				  this 
				, moveableLayerWidget::CROSS_SPOT_FIRST
				, _c2cMarginX*2 + _c2cCircleDiameter
				, _c2cMarginY*2 + _c2cCircleDiameter
				, _c2cCircleDiameter
				, _c2cROIRectsLeft[0].topLeft()
				, C2C_COLOR
				, SOLID_PEN
				, creationScale);

		c2cCross->setProperty("idx", 0);
		_c2cCrossesLeft.push_back(c2cCross);


		for (auto i = 1; i < _c2cROIRectsLeft.size(); i++ )
		{
			auto c2c = new moveableLayerWidget(
							  this 
							, moveableLayerWidget::CROSS_SPOT_OTHER
							, _c2cMarginX*2 + _c2cCircleDiameter
							, _c2cMarginY*2 + _c2cCircleDiameter
							, _c2cCircleDiameter
							, _c2cROIRectsLeft[i].topLeft()
							, C2C_COLOR
							, SOLID_PEN
							, creationScale);

			c2c->setProperty("idx", i);
			c2c->enableXChange(!_bC2CLinedUp);
			_c2cCrossesLeft.push_back(c2c);
		}

		_allCrossesArray << _c2cCrossesLeft;
	}

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

void roiRenderWidgetFull::handleROIControlPointMoved(moveableLayerWidget* sender, QPoint topLeft, QPoint centerPos)
{
	// transform current TopLeft pt to original image position
	const auto newLeftTopPos = fromWidget2OriginalImagePt(topLeft);
	sender->setTopLeftOnOriginalImage(newLeftTopPos);
	auto const i2sPt = fromWidget2OriginalImagePt(_i2sCrossLeft ? _i2sCrossLeft->getCenterPoint (_i2sCrossLeft->mapToParent (_i2sCrossLeft->rect().topLeft())) : QPoint());

	// update rectangle for draw
	if (sender == _i2sCrossLeft )
	{
		reportI2S(i2sPt, newLeftTopPos);
	}
	else if (sender == _leftStripLine )
	{
		reportLeftStripEdgeChange(i2sPt);
	}
	else if (sender == _rightStripLine )
	{
		reportRightStripEdgeChange(i2sPt);
	}
	else if (sender == _pageStripLine )
	{
		reportPageEdgeChange(i2sPt);
	}
	else
	{
		reportC2Cs(sender, i2sPt, newLeftTopPos);
	}
	update();
}

void roiRenderWidgetFull::reportLeftStripEdgeChange(const QPoint& i2sPoint)
{
	auto const targetWidget = _leftStripLine;
	auto const newX = targetWidget ? targetWidget->getCenterPoint (targetWidget->mapToParent (targetWidget->rect().topLeft())).x() : 0;
	emit roiChanged_leftStripEdge(i2sPoint, fromWidget2OriginalImagePt(QPoint(newX,0)).x());
}

void roiRenderWidgetFull::reportRightStripEdgeChange(const QPoint& i2sPoint)
{
	auto const targetWidget = _rightStripLine;
	auto const newX = targetWidget ? targetWidget->getCenterPoint (targetWidget->mapToParent (targetWidget->rect().topLeft())).x() : 0;
	emit roiChanged_rightStripEdge(i2sPoint, fromWidget2OriginalImagePt(QPoint(newX,0)).x());
}

void roiRenderWidgetFull::reportPageEdgeChange(const QPoint& i2sPoint)
{
	auto const targetWidget = _pageStripLine;
	auto const newX = targetWidget ? targetWidget->getCenterPoint (targetWidget->mapToParent (targetWidget->rect().topLeft())).x() : 0;
	emit roiChanged_pageStripEdge(i2sPoint, fromWidget2OriginalImagePt(QPoint(newX,0)).x());
}

void roiRenderWidgetFull::reportI2S(const QPoint& i2sPoint, const QPoint& newLeftTop)
{
	_i2sROIRectLeft = QRect(newLeftTop.x(), newLeftTop.y(), _i2sROIRectLeft.width(), _i2sROIRectLeft.height());
	_bI2SMoved = true;
	emit roiChanged_i2s(i2sPoint);
}
	
void roiRenderWidgetFull::reportC2Cs(moveableLayerWidget* sender, const QPoint& i2sPoint, const QPoint& newLeftTopPos)
{
	if (sender)
	{
		const auto idx = sender->property("idx").toInt();
		_c2cROIRectsLeft[idx] = QRect(newLeftTopPos.x(), newLeftTopPos.y(), _c2cROIRectsLeft[idx].width(), _c2cROIRectsLeft[idx].height());
	}

	QVector<QPoint> c2cpts;
	for ( auto cr : _c2cCrossesLeft)
	{
		const auto calcTL = cr->mapToParent (cr->rect().topLeft());
		c2cpts <<  fromWidget2OriginalImagePt(cr->getCenterPoint (calcTL));
	}
	emit roiChanged_c2c(i2sPoint, c2cpts);
}

void roiRenderWidgetFull::paintROIRects(glDrawFunc func)
{
	func(_i2sROIRectLeft, true, I2S_COLOR_FRAME, false, JGL_NO_COLOR );
	func(_i2sROIRectRight, true, I2S_COLOR_FRAME, false, JGL_NO_COLOR);

	func(_leftStripRect, false, JGL_NO_COLOR, true, STRIP_COLOR_FILL);
	func(_rightStripRect, false, JGL_NO_COLOR, true, STRIP_COLOR_FILL);
	
	for (auto const& rc : _c2cROIRectsLeft) 
	{
		func(rc, true, C2C_COLOR_FRAME, false, JGL_NO_COLOR);
	}
	for (auto const& rc : _c2cROIRectsRight) 
	{
		func(rc, true, C2C_COLOR_FRAME, false, JGL_NO_COLOR);
	}
}

void roiRenderWidgetFull::updateC2CForLineUp( bool bLineUp )
{
	// get left for the first C2C ROI
	if (_c2cCrossesLeft.isEmpty())
		return;

	if (bLineUp)
	{
		const auto firstPT = _c2cCrossesLeft[0]->geometry().topLeft();
		const auto firstPTMapped = fromWidget2OriginalImagePt(firstPT);

		for ( int i = 1; i < _c2cCrossesLeft.size(); i++ )
		{
			auto const currentPt = fromWidget2OriginalImagePt(_c2cCrossesLeft[i]->geometry().topLeft());
			_c2cCrossesLeft[i]->setTopLeftOnOriginalImage(QPoint(firstPTMapped.x(), _c2cCrossesLeft[i]->topLeftOnOriginalImage().y()));
			_c2cCrossesLeft[i]->move(firstPT.x(), _c2cCrossesLeft[i]->geometry().topLeft().y());
			_c2cCrossesLeft[i]->enableXChange(false);
			_c2cROIRectsLeft[i] = QRect(firstPTMapped.x(), currentPt.y(), _c2cROIRectsLeft[i].width(), _c2cROIRectsLeft[i].height());
		}

		auto const i2sPt = fromWidget2OriginalImagePt(_i2sCrossLeft ? _i2sCrossLeft->getCenterPoint (_i2sCrossLeft->mapToParent (_i2sCrossLeft->rect().topLeft())) : QPoint());
		reportC2Cs (nullptr, i2sPt, QPoint());
	}
	else
	{
		for ( int i = 0; i < _c2cCrossesLeft.size(); i++ )
		{
			_c2cCrossesLeft[i]->enableXChange(true);
		}
	}
}
