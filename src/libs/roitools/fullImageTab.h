#pragma once

#include <QWidget>
#include "ui_fullImageTab.h"

class roiWidget;

class fullImageTab : public QWidget
{
	Q_OBJECT

public:
	fullImageTab(QWidget *parent = Q_NULLPTR);
	~fullImageTab();

	private slots:

	void onEditNumOfColorsEdited(const QString &text);
	void onEditSpinnerStepEdited(const QString &text);

	void onSpinTriangleOffsetXChanged(double dVal );
	void onSpinTriangleOffsetYChanged(double dVal );

	void onSpinA1Changed(double dVal );
	void onSpinA2Changed(double dVal );

private:
	Ui::fullImageTab ui;

	roiWidget * _fullImageBox;
};
