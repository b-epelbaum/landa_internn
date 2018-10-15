#pragma once

#include <QDialog>
#include "ui_roiview.h"

class RenderWidget;

class roiview : public QDialog
{
	Q_OBJECT

public:
	roiview(QWidget *parent = Q_NULLPTR);
	~roiview();

private:
	Ui::roiview ui;

private slots:

	void onLeftImageLoad();
	void onRightImageLoad();

	void onEditNumOfColorsEdited(const QString &text);
	void onEditSpinnerStepEdited(const QString &text);

	void onSpinTriangleOffsetXChanged(double dVal );
	void onSpinTriangleOffsetYChanged(double dVal );

	void onSpinA1Changed(double dVal );
	void onSpinA2Changed(double dVal );

	void onLeftImageCursorPos(QPoint pt);
	void onRightImageCursorPos(QPoint pt);

private:

	QString selectBitmapFile();

	RenderWidget * _leftImageBox;
	RenderWidget * _rightImageBox;

	QLabel * _leftImageCoordsLabel, *_rightImageCoordsLabel;
};
