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

};
