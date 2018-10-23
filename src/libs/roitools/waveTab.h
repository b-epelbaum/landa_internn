#pragma once

#include <QWidget>
#include "ui_waveTab.h"

class roiWidget;

class waveTab : public QWidget
{
	Q_OBJECT

public:
	waveTab(QWidget *parent = Q_NULLPTR);
	~waveTab();

	private slots:

	void onEditNumOfColorsEdited(const QString &text);
	void onEditSpinnerStepEdited(const QString &text);

	void onSpinTriangleOffsetXChanged(double dVal );
	void onSpinTriangleOffsetYChanged(double dVal );

	void onSpinA1Changed(double dVal );
	void onSpinA2Changed(double dVal );

private:
	Ui::waveTab ui;

	roiWidget * _waveImageBox;
};
