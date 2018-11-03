#pragma once

#include <QWidget>
#include "ui_waveTab.h"
#include "roiParamWidget.h"
#include "common/type_usings.h"

class roiWidget;

class waveTab : public QWidget
{
	Q_OBJECT

public:
	waveTab(QWidget *parent = Q_NULLPTR);
	~waveTab();
	
	void setParameters (roiParamWidget* paramWidget, LandaJune::ProcessParametersPtr params );

private slots:

	void onImageLoaded ( QString strPath, LandaJune::CORE_ERROR );
	void onPropertyChanged (QString propName, QVariant newVal );
	void onROIChanged( const QVector<QPoint> ptArray );

signals :

	void editDone( bool bApply );

private:

	double toMMX(int val_pxx) const;
	double toMMY(int val_pxy) const;

	void buildControls();
	void setupInitialROIs();
	void updateROIs();

	void recalculateOffsets (const QVector<QPoint>& pts);
	void recalculateI2SOffset(const QPoint& pt);

	Ui::waveTab ui;

	roiImageBox * _waveImageBox;
	roiParamWidget * _paramWidget;

	double _Pixel2MM_X = 0.0;
	double _Pixel2MM_Y = 0.0;

	LandaJune::ProcessParametersPtr _params;
};
