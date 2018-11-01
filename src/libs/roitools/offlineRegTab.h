#pragma once

#include <QWidget>
#include "ui_offlineRegTab.h"
#include "common/type_usings.h"

class roiWidget;
class roiParamWidget;

class offlineRegTab : public QWidget
{
	Q_OBJECT

public:
	offlineRegTab(QWidget *parent = Q_NULLPTR);
	~offlineRegTab();

	void setParameters (roiParamWidget* paramWidget, LandaJune::ProcessParametersPtr params );

private slots:

	void onPropertyChanged (QString propName, QVariant newVal );

	void onROIScaleChanged( double glScale, double imageScale );
	void onROIScrollChanged( int hScroll, int vScroll );

	void oni2sPosChanged(QPoint pt);
	void onc2cPosChanged(int idx, QPoint pt);

private:

	void buildControls();
	void recalculate ();

	void setupROIs() const;
	void updateROIs() const;

	double toMMX(int val_pxx) const;
	double toMMY(int val_pxy) const;

	Ui::offlineRegTab ui;

	roiImageBox * _leftImageBox;
	roiImageBox * _rightImageBox;
	roiParamWidget * _paramWidget;

	double _Pixel2MM_X = 0.0;
	double _Pixel2MM_Y = 0.0;

	QComboBox * _colorCounterCombo = nullptr;
	
	LandaJune::ProcessParametersUniquePtr _params;

};
