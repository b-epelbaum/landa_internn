#pragma once

#include <QWidget>
#include "ui_roiWidget.h"

class roiWidget : public QWidget
{
	Q_OBJECT

public:
	roiWidget(QWidget *parent = Q_NULLPTR);
	~roiWidget();

	void setImage ( const QString& strFilePath );

private slots:

	void onImageCursorPos(QPoint pt);
	void horzValueChanged(int);
	void vertValueChanged(int);

private:
	Ui::roiWidget ui;

	QWidget * _roiWidget;
};
