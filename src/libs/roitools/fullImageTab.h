#pragma once

#include <QWidget>
#include "ui_fullImageTab.h"
#include "roiParamWidget.h"
#include "common/type_usings.h"

class roiWidget;

class fullImageTab : public QWidget
{
	Q_OBJECT

public:
	fullImageTab(QWidget *parent = Q_NULLPTR);
	~fullImageTab();

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

	Ui::fullImageTab ui;

	roiImageBox *		_fullImageBox;
	roiParamWidget *	_paramWidget;

	double _Pixel2MM_X = 0.0;
	double _Pixel2MM_Y = 0.0;

	LandaJune::ProcessParametersPtr _params;

};
