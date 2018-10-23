#pragma once

#include <QWidget>
#include "ui_offlineRegTab.h"

class roiWidget;

class offlineRegTab : public QWidget
{
	Q_OBJECT

public:
	offlineRegTab(QWidget *parent = Q_NULLPTR);
	~offlineRegTab();

private slots:

	void onLeftRightROIScaleChanged( float glScale, float imageScale );
	void onLeftRightROIScrollChanged( int hScroll, int vScroll );

	void onEditNumOfColorsEdited(const QString &text);
	void onEditSpinnerStepEdited(const QString &text);

	void onSpinTriangleOffsetXChanged(double dVal );
	void onSpinTriangleOffsetYChanged(double dVal );

	void onSpinA1Changed(double dVal );
	void onSpinA2Changed(double dVal );


private:
	Ui::offlineRegTab ui;

	roiWidget * _leftImageBox;
	roiWidget * _rightImageBox;

};
