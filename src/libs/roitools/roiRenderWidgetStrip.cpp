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

void roiRenderWidgetStrip::setROIs(
			  bool bIsInitialROI
			, const QRect& is2sRc
			, const QVector<QRect>& c2cRects
			, int edgeX
			, QSize i2sMargins
			, QSize c2cMargins
			, int c2cCircleDiameter
			, bool bInteractive  )
{
	_i2sROIRect = is2sRc;
	_c2cROIRects = c2cRects;

	_i2sMarginX = i2sMargins.width();
	_i2sMarginY = i2sMargins.height();

	_c2cMarginX = c2cMargins.width();
	_c2cMarginY = c2cMargins.height();

	_c2cCircleDiameter = c2cCircleDiameter;

	_edgeX = edgeX;

	_bInteractive = bInteractive;

	if(_hasImage )
	{
		if (!bIsInitialROI)
			createCrossHairs (_imageScaleRatio);
		update();
	}
}

void roiRenderWidgetStrip::createCrossHairs( float creationScale )
{		
	if (!_bInteractive)
		return;

	if (_i2sCross != nullptr )
		delete _i2sCross;
	_i2sCross = nullptr;

	if (_edgeLine != nullptr )
		delete _edgeLine;
	_edgeLine = nullptr;

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
				, _i2sROIRect.topLeft()
				, I2S_COLOR
				, SOLID_PEN
				, creationScale);
	}

	if ( _edgeX != 0 )
	{
		_edgeLine = new moveableLayerWidget(
					  this
					, moveableLayerWidget::CROSS_EDGE
					, 4
					, _imageSize.height()
					, _c2cCircleDiameter
					, QPoint(_edgeX, 0)
					, PAGE_OFFSET_COLOR
					, DOT_PEN
					, creationScale);
	}
	
	if (!_c2cROIRects.isEmpty())
	{
		auto c2cCross = new moveableLayerWidget(
					  this
					, moveableLayerWidget::CROSS_SPOT_FIRST
					, _c2cMarginX*2 + _c2cCircleDiameter
					, _c2cMarginY*2 + _c2cCircleDiameter
					, _c2cCircleDiameter
					, _c2cROIRects[0].topLeft()
					, C2C_COLOR
					, SOLID_PEN
					, creationScale);

		c2cCross->setProperty("idx", 0);
		_c2cCrosses.push_back(c2cCross);


		for (auto i = 1; i < _c2cROIRects.size(); i++ )
		{
			auto c2c = new moveableLayerWidget(
						  this
						, moveableLayerWidget::CROSS_SPOT_OTHER
						, _c2cMarginX*2 + _c2cCircleDiameter
						, _c2cMarginY*2 + _c2cCircleDiameter
						, _c2cCircleDiameter
						, _c2cROIRects[i].topLeft()
						, C2C_COLOR
						, SOLID_PEN
						, creationScale);
			c2c->setProperty("idx", i);
			c2c->enableXChange(!_bC2CLinedUp);
			_c2cCrosses.push_back(c2c);
		}
	}

	_allCrossesArray << _i2sCross << _c2cCrosses;
	if (_edgeLine)
		_allCrossesArray << _edgeLine;

	for ( auto elem : _allCrossesArray)
	{
		if (elem )
		{
			connect (elem, &moveableLayerWidget::crossMoving, this, &roiRenderWidgetBase::onCrossMoving );
			connect (elem, &moveableLayerWidget::movingOver, this, &roiRenderWidgetBase::onCrossMovingOver );
			connect (elem, &moveableLayerWidget::crossMoved, this, &roiRenderWidgetBase::onROIControlPointMoved );
		}
	}
	updateInternalLayers();
}

void roiRenderWidgetStrip::handleROIControlPointMoved(moveableLayerWidget* sender, QPoint topLeft, QPoint centerPos)
{
	// transform current TopLeft pt to original image position
	const auto newLeftTopPos = fromWidget2OriginalImagePt(topLeft);
	sender->setTopLeftOnOriginalImage(newLeftTopPos);
	auto const i2sPt = fromWidget2OriginalImagePt(_i2sCross ? _i2sCross->getCenterPoint (_i2sCross->mapToParent (_i2sCross->rect().topLeft())) : QPoint());

	// update rectangle for draw
	if (sender == _i2sCross )
	{
		reportI2S(i2sPt, newLeftTopPos);
	}
	else if (sender == _edgeLine )
	{
		// no rects
		reportEdgeChange(i2sPt);
	}
	else
	{
		reportC2Cs(sender, i2sPt, newLeftTopPos);
	}
	update();
}

void roiRenderWidgetStrip::reportEdgeChange(const QPoint& i2sPoint)
{
	// no rects
	auto const edgeX = _edgeLine ? _edgeLine->getCenterPoint (_edgeLine->mapToParent (_edgeLine->rect().topLeft())).x() : 0;
	emit roiChanged_edge(i2sPoint, fromWidget2OriginalImagePt(QPoint(edgeX,0)).x());
}

void roiRenderWidgetStrip::reportI2S(const QPoint& i2sPoint, const QPoint& newLeftTop)
{
	_i2sROIRect = QRect(newLeftTop.x(), newLeftTop.y(), _i2sROIRect.width(), _i2sROIRect.height());
	emit roiChanged_i2s(i2sPoint);
}
	
void roiRenderWidgetStrip::reportC2Cs(moveableLayerWidget* sender, const QPoint& i2sPoint, const QPoint& newLeftTopPos)
{
	if (sender)
	{
		const auto idx = sender->property("idx").toInt();
		_c2cROIRects[idx] = QRect(newLeftTopPos.x(), newLeftTopPos.y(), _c2cROIRects[idx].width(), _c2cROIRects[idx].height());
	}

	QVector<QPoint> c2cpts;
	for ( auto cr : _c2cCrosses)
	{
		const auto calcTL = cr->mapToParent (cr->rect().topLeft());
		c2cpts <<  fromWidget2OriginalImagePt(cr->getCenterPoint (calcTL));
	}
	emit roiChanged_c2c(i2sPoint, c2cpts);
}

void roiRenderWidgetStrip::paintROIRects( glDrawFunc func )
{
	func(_i2sROIRect, true, I2S_COLOR_FRAME, false, JGL_NO_COLOR);
	for (auto const& rc : _c2cROIRects) 
	{
		func(rc, true, C2C_COLOR_FRAME, false, JGL_NO_COLOR);
	}
}

void roiRenderWidgetStrip::updateC2CForLineUp( bool bLineUp )
{
	// get left for the first C2C ROI
	if (_c2cCrosses.isEmpty())
		return;

	if (bLineUp)
	{
		const auto firstPT = _c2cCrosses[0]->geometry().topLeft();
		const auto firstPTMapped = fromWidget2OriginalImagePt(firstPT);

		for ( int i = 1; i < _c2cCrosses.size(); i++ )
		{
			auto const currentPt = fromWidget2OriginalImagePt(_c2cCrosses[i]->geometry().topLeft());
			_c2cCrosses[i]->setTopLeftOnOriginalImage(QPoint(firstPTMapped.x(), _c2cCrosses[i]->topLeftOnOriginalImage().y()));
			_c2cCrosses[i]->move(firstPT.x(), _c2cCrosses[i]->geometry().topLeft().y());
			_c2cCrosses[i]->enableXChange(false);
			_c2cROIRects[i] = QRect(firstPTMapped.x(), currentPt.y(), _c2cROIRects[i].width(), _c2cROIRects[i].height());
		}

		auto const i2sPt = fromWidget2OriginalImagePt(_i2sCross ? _i2sCross->getCenterPoint (_i2sCross->mapToParent (_i2sCross->rect().topLeft())) : QPoint());
		reportC2Cs (nullptr, i2sPt, QPoint());
	}
	else
	{
		for ( int i = 0; i < _c2cCrosses.size(); i++ )
		{
			_c2cCrosses[i]->enableXChange(true);
		}
	}
}
