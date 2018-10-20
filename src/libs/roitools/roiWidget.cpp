#include "roiWidget.h"

roiWidget::roiWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.openGLWidget, &RenderWidget::cursorPos, this, &roiWidget::onImageCursorPos);
	
	connect(ui.horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzValueChanged(int)));
	connect(ui.verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(vertValueChanged(int)));

	ui.openGLWidget->assignScrollBars(ui.verticalScrollBar, ui.horizontalScrollBar);
	ui.imageCoordsLabel->setText ("");

	/*
	_roiWidget = new QWidget(this);
	_roiWidget->setMaximumSize(200,200);
	_roiWidget->setMinimumSize(200,200);
	_roiWidget->setStyleSheet("QWidget { background-color : red; }");
	_roiWidget->setGeometry(20,20, 100, 100);
	_roiWidget->raise();
	_roiWidget->show();
	*/
}

roiWidget::~roiWidget()
{
}

void roiWidget::setImage(const QString& strFilePath)
{
	ui.openGLWidget->setImage(strFilePath);
}

void roiWidget::onImageCursorPos(QPoint pt)
{
	ui.imageCoordsLabel->setText(QString("X = %1, Y = %2").arg(pt.x()).arg(pt.y()));
}

void roiWidget::horzValueChanged(int hVal) 
{
	ui.openGLWidget->update();
	//_roiWidget->move(-hVal, 0);
}

void roiWidget::vertValueChanged(int vVal) 
{
	ui.openGLWidget->update();
	//_roiWidget->move(0, -vVal);
}