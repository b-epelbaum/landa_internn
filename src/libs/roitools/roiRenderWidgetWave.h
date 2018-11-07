#pragma once

#include "roiRenderWidgetBase.h"

class roiRenderWidgetWave : public roiRenderWidgetBase
{
	Q_OBJECT

public:
	roiRenderWidgetWave(QWidget *parent);
	~roiRenderWidgetWave();

	void setROIs(
					  bool bInitROI
					, const QRect& waveTriangleRc
					, const QRect& waveROI	
					, QSize waveTriangleMargins );

protected:

	void cleanup() override;
	void cleanLocalObjects();
	void createCrossHairs( float creationScale ) override;
	void handleROIControlPointMoved( moveableLayerWidget* sender, QPoint topLeft, QPoint centerPos ) override;
	void paintROIRects( glDrawFunc func ) override;

signals:

	void waveTriangleChanged( QPoint controlPoint );

private:

	QRect _waveTriangleROIRc, _waveROIRc;
	moveableLayerWidget * _waveTriangleCross = nullptr;

	int	_waveTriangleMarginX = 0;
	int	_waveTriangleMarginY = 0;
};
