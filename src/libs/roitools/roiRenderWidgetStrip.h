#pragma once

#include "roiRenderWidgetBase.h"

class roiRenderWidgetStrip : public roiRenderWidgetBase
{
	Q_OBJECT

public:
	roiRenderWidgetStrip(QWidget *parent);
	~roiRenderWidgetStrip();

	void setROIs(
				bool bInitROI
			  , const QRect& is2sRc
			  , const QVector<QRect>& c2cRects
			  , int edgeX
			  , QSize i2sMargins
			  , QSize c2cMargins
			  , int c2cCircleDiameter
			  , bool bInteractive );

	void setC2CRoisLinedUp ( bool bVal ) { _bC2CLinedUp = bVal; updateC2CForLineUp(bVal); }

protected:

	void cleanup() override;
	void createCrossHairs( float creationScale ) override;
	void handleROIControlPointMoved( moveableLayerWidget* sender, QPoint topLeft, QPoint centerPos ) override;
	void paintROIRects( std::function<void(const QRect&)> func ) override;

	void reportEdgeChange(const QPoint& i2sPoint);
	void reportI2S(const QPoint& i2sPoint, const QPoint& newLeftTop);
	void reportC2Cs(moveableLayerWidget* sender, const QPoint& i2sPoint, const QPoint& newLeftTopPos);

	void updateC2CForLineUp ( bool bLineUp );

signals:

	void roiChanged_edge( const QPoint i2spt, const int edgeX );
	void roiChanged_i2s( const QPoint i2spt );
	void roiChanged_c2c( const QPoint i2spt, const QVector<QPoint>& c2cPts );

private:

	QRect _i2sROIRect;
	QVector<QRect> _c2cROIRects;
	moveableLayerWidget * _i2sCross = nullptr;
	moveableLayerWidget * _edgeLine = nullptr;
	QVector<moveableLayerWidget *> _c2cCrosses;

	int	_i2sMarginX = 0;
	int	_i2sMarginY = 0;
	int	_c2cMarginX = 0;
	int	_c2cMarginY = 0;
	int _c2cCircleDiameter = 0;
	int _edgeX = 0;

	bool _bInteractive = true;
	bool _bC2CLinedUp = false;
};
