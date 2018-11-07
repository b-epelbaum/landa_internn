#pragma once

#include "roiRenderWidgetBase.h"

class roiRenderWidgetFull : public roiRenderWidgetBase
{
	Q_OBJECT

public:
	roiRenderWidgetFull(QWidget *parent);
	~roiRenderWidgetFull();

	void setROIs(
				  bool bInitROI
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
				, bool updateBoth);

	void setC2CRoisLinedUp ( bool bVal ) { _bC2CLinedUp = bVal; updateC2CForLineUp(bVal); }

protected:

	void cleanup() override;
	void cleanLocalObjects();
	void createCrossHairs( float creationScale ) override;
	void handleROIControlPointMoved( moveableLayerWidget* sender, QPoint topLeft, QPoint centerPos ) override;
	void paintROIRects( glDrawFunc func ) override;

	void reportLeftStripEdgeChange(const QPoint& i2sPoint);
	void reportRightStripEdgeChange(const QPoint& i2sPoint);
	void reportPageEdgeChange(const QPoint& i2sPoint);
	void reportI2S(const QPoint& i2sPoint, const QPoint& newLeftTop);
	void reportC2Cs(moveableLayerWidget* sender, const QPoint& i2sPoint, const QPoint& newLeftTopPos);

	void updateC2CForLineUp ( bool bLineUp );

signals:

	void roiChanged_leftStripEdge( const QPoint i2spt, const int edgeX );
	void roiChanged_rightStripEdge( const QPoint i2spt, const int edgeX );
	void roiChanged_pageStripEdge( const QPoint i2spt, const int edgeX );
	void roiChanged_i2s( const QPoint i2spt );
	void roiChanged_c2c( const QPoint i2spt, const QVector<QPoint>& c2cPts );

private:

	QRect _i2sROIRectLeft, _i2sROIRectRight;
	QRect _leftStripRect, _rightStripRect;
	QVector<QRect> _c2cROIRectsLeft, _c2cROIRectsRight;
	moveableLayerWidget * _i2sCrossLeft = nullptr;
	moveableLayerWidget * _leftStripLine = nullptr;
	moveableLayerWidget * _rightStripLine = nullptr;
	moveableLayerWidget * _pageStripLine = nullptr;
	QVector<moveableLayerWidget *> _c2cCrossesLeft;
	

	int	_i2sMarginX = 0;
	int	_i2sMarginY = 0;
	int	_c2cMarginX = 0;
	int	_c2cMarginY = 0;
	int _c2cCircleDiameter = 0;
	int _pageOffsetX = 0;
	
	bool _bC2CLinedUp = false;
	bool _bI2SMoved = false;
};
