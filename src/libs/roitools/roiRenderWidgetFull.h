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
					, QSize i2sMargins, QSize c2cMargins, int c2cCircleDiameter );

protected:

	void cleanup() override;
	void createCrossHairs( float creationScale ) override;
	void handleROIControlPointMoved( moveableLayerWidget* sender, QPoint topLeft, QPoint centerPos ) override;
	void paintROIRects( std::function<void(const QRect&)> func ) override;

signals:

	void roiChanged( const QVector<QPoint>& c2cPts );

private:

	QRect _i2sROIRectLeft, _i2sROIRectRight;
	QVector<QRect> _c2cROIRectsLeft, _c2cROIRectsRight;
	moveableLayerWidget * _i2sCrossLeft = nullptr;;
	moveableLayerWidget * _i2sCrossRight = nullptr;;
	QVector<moveableLayerWidget *> _c2cCrossesLeft;
	QVector<moveableLayerWidget *> _c2cCrossesRight;

	int	_i2sMarginX = 0;
	int	_i2sMarginY = 0;
	int	_c2cMarginX = 0;
	int	_c2cMarginY = 0;
	int _c2cCircleDiameter = 0;
};
