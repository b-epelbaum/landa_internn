#include "roiWidget.h"
#include <QAction>
#include "commonTabs.h"
#include <QMenuBar>

roiWidget::roiWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	QAction *opeFileAction = ui.browseFileEdit->addAction(QIcon(":/roiTools/Resources/file_open.png"), QLineEdit::TrailingPosition);
	connect(opeFileAction, &QAction::triggered, this, &roiWidget::onOpenFile);

	_renderWidget = ui.openGLWidget;

	connect(_renderWidget, &RenderWidget::cursorPos, this, &roiWidget::onImageCursorPos);
	connect(_renderWidget, &RenderWidget::scaleChanged, this, &roiWidget::onScaleChanged);
	
	connect(ui.horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzValueChanged(int)));
	connect(ui.verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(vertValueChanged(int)));

	ui.openGLWidget->assignScrollBars(ui.horizontalScrollBar, ui.verticalScrollBar);
	ui.imageCoordsLabel->setText ("");

	QMenu *menu = new QMenu();
	menu->addAction(new QAction("Actual pixels", this));
	menu->addAction(new QAction("Fit to screen", this));
	menu->addSeparator();
	menu->addAction(new QAction("20%", this));
	menu->addAction(new QAction("40%", this));
	menu->addAction(new QAction("80%", this));
	menu->addAction(new QAction("150%", this));
	menu->addAction(new QAction("250%", this));
	menu->addAction(new QAction("400%", this));
	menu->addAction(new QAction("500%", this));

	int idxProp = -1;
	for ( auto ac : menu->actions() )
	{
		ac->setProperty("idx", idxProp);
		connect(ac, &QAction::triggered, this, &roiWidget::onZoomAction);
		idxProp ++;
	}

	ui.zoomButt->setMenu(menu);
	ui.zoomButt->setPopupMode(QToolButton::InstantPopup);
}

roiWidget::~roiWidget()
{
}

void roiWidget::setImage(const QString& strFilePath)
{
	if (_renderWidget->setImage(strFilePath) )
	{
		displayMetaData();
	}
}

void roiWidget::displayMetaData()
{
	auto imgSize = _renderWidget->getImageSize();
	ui.labelWidth->setText(QString::number(imgSize.width()));
	ui.labelHeight->setText(QString::number(imgSize.height()));
}

void roiWidget::setZoom(int zoomPercentage)
{
	if ( zoomPercentage == -1)
	{
		_renderWidget->showActualPixels();
	}
	else if ( zoomPercentage == 0)
	{
		_renderWidget->showFitOnScreen();
	}
	else
	{
		_renderWidget->setZoom( zoomPercentage);
	}
}

void roiWidget::setScales(float glScale, float imageScale)
{
	ui.labelZoom->setText(QString::number(int(imageScale * 100)));
	_renderWidget->setScales(glScale, imageScale);
}

void roiWidget::setScrolls(int hScrollVal, int vScrollVal)
{
	disconnect(ui.horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzValueChanged(int)));
	disconnect(ui.verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(vertValueChanged(int)));
	ui.horizontalScrollBar->setValue(hScrollVal);

	ui.verticalScrollBar->setValue(vScrollVal);
	_renderWidget->updateHScroll(hScrollVal);
	_renderWidget->updateVScroll(vScrollVal);
	_renderWidget->update();

	connect(ui.horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzValueChanged(int)));
	connect(ui.verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(vertValueChanged(int)));

}

void roiWidget::onOpenFile()
{
	_loadedImageFilePath = commonTabs::selectBitmapFile(this, _strPrompt, _strFileSaveKey);
	if (!_loadedImageFilePath.isEmpty())
	{
		setImage(_loadedImageFilePath);
		ui.browseFileEdit->setText(_loadedImageFilePath);
	}
}

void roiWidget::onZoomAction()
{
	auto ac = dynamic_cast<QAction*>(sender());
	auto idx = ac->property("idx").toInt();

	int zoomPercentge = -1;

	switch (idx )
	{
		case 0 : zoomPercentge = 0; break;
		case 2 : zoomPercentge = 20; break;
		case 3 : zoomPercentge = 40; break;
		case 4 : zoomPercentge = 80; break;
		case 5 : zoomPercentge = 150; break;
		case 6 : zoomPercentge = 250; break;
		case 7 : zoomPercentge = 400; break;
		case 8 : zoomPercentge = 500; break;
		default: zoomPercentge = -1;;
	}

	setZoom(zoomPercentge);
}

void roiWidget::onImageCursorPos(QPoint pt, QSize size)
{
	QString strText = QString("X = %1, Y = %2").arg(pt.x()).arg(pt.y());
	if (!size.isEmpty() )
		strText = QString("X1 = %1, Y1 = %2, X2 = %3, Y2 = %4 [ %5, %6 ]").arg(pt.x()).arg(pt.y()).arg(pt.x() + size.width()).arg(pt.y()+ size.height()).arg(size.width()).arg(size.height());;
	ui.imageCoordsLabel->setText(strText);
}

void roiWidget::horzValueChanged(int hVal) 
{
	ui.openGLWidget->updateHScroll(hVal);
	emit scrollValuesChanged (hVal, ui.verticalScrollBar->value());
}

void roiWidget::vertValueChanged(int vVal) 
{
	ui.openGLWidget->updateVScroll(vVal);
	emit scrollValuesChanged (ui.horizontalScrollBar->value(), vVal);
}

void roiWidget::onScaleChanged(double newGLScale, double newImageScale)
{
	ui.labelZoom->setText(QString::number(int(newImageScale * 100)));
	emit scaleChanged(newGLScale, newImageScale );
}
