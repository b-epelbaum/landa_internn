#pragma once

#include "roiRenderWidgetBase.h"

class roiRenderWidgetStrip : public roiRenderWidgetBase
{
	Q_OBJECT

public:
	roiRenderWidgetStrip(QWidget *parent);
	~roiRenderWidgetStrip();

	void setROIs(bool bInitROI, const QRect& is2sRc, const QVector<QRect>& c2cRects, QSize i2sMargins, QSize c2cMargins, int c2cCircleDiameter );

protected:

	void cleanup() override;
	void createCrossHairs( float creationScale ) override;
	void handleROIControlPointMoved( moveableLayerWidget* sender, QPoint topLeft, QPoint centerPos ) override;
	void paintROIRects( std::function<void(const QRect&)> func ) override;

private:

	QRect _i2sROIRect;
	QVector<QRect> _c2cROIRects;
	moveableLayerWidget * _i2sCross = nullptr;;
	QVector<moveableLayerWidget *> _c2cCrosses;

	int	_i2sMarginX = 0;
	int	_i2sMarginY = 0;
	int	_c2cMarginX = 0;
	int	_c2cMarginY = 0;
	int _c2cCircleDiameter = 0;
};
