#include "roiImageBox.h"

#include <QAction>
#include "commonTabs.h"
#include <QMenuBar>

roiImageBox::roiImageBox(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	QAction *opeFileAction = ui.browseFileEdit->addAction(QIcon(":/roiTools/Resources/file_open.png"), QLineEdit::TrailingPosition);
	connect(opeFileAction, &QAction::triggered, this, &roiImageBox::onOpenFile);

	_renderWidget = ui.openGLWidget;

	connect(_renderWidget, &roiRenderWidget::i2sPosChanged, this, &roiImageBox::i2sPosChanged);
	connect(_renderWidget, &roiRenderWidget::c2cPosChanged, this, &roiImageBox::c2cPosChanged);

	connect(_renderWidget, &roiRenderWidget::cursorPos, this, &roiImageBox::onImageCursorPos);
	connect(_renderWidget, &roiRenderWidget::scaleChanged, this, &roiImageBox::onScaleChanged);
	
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
		connect(ac, &QAction::triggered, this, &roiImageBox::onZoomAction);
		idxProp ++;
	}

	ui.zoomButt->setMenu(menu);
	ui.zoomButt->setPopupMode(QToolButton::InstantPopup);
}

roiImageBox::~roiImageBox()
{
}

LandaJune::CORE_ERROR roiImageBox::setImage(const QString& strFilePath)
{
	auto err = _renderWidget->setImage(strFilePath);
	if (err == LandaJune::CORE_ERROR::RESULT_OK)
	{
		displayMetaData();
	}
	return err;
}

void roiImageBox::displayMetaData()
{
	auto imgSize = _renderWidget->getImageSize();
	ui.labelWidth->setText(QString::number(imgSize.width()));
	ui.labelHeight->setText(QString::number(imgSize.height()));
}

void roiImageBox::setZoom(int zoomPercentage)
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

void roiImageBox::setScales(float glScale, float imageScale)
{
	ui.labelZoom->setText(QString::number(int(imageScale * 100)));
	_renderWidget->setScales(glScale, imageScale);
}

void roiImageBox::setScrolls(int hScrollVal, int vScrollVal)
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

void roiImageBox::onOpenFile()
{
	_loadedImageFilePath = commonTabs::selectBitmapFile(this, _strPrompt, _strFileSaveKey);
	if (!_loadedImageFilePath.isEmpty())
	{
		setImage(_loadedImageFilePath);
		ui.browseFileEdit->setText(_loadedImageFilePath);
	}
}

void roiImageBox::onZoomAction()
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

void roiImageBox::onImageCursorPos(QPoint pt, QSize size)
{
	QString strText = QString("X = %1, Y = %2").arg(pt.x()).arg(pt.y());
	if (!size.isEmpty() )
		strText = QString("X1 = %1, Y1 = %2, X2 = %3, Y2 = %4 [ %5, %6 ]").arg(pt.x()).arg(pt.y()).arg(pt.x() + size.width()).arg(pt.y()+ size.height()).arg(size.width()).arg(size.height());;
	ui.imageCoordsLabel->setText(strText);
}

void roiImageBox::horzValueChanged(int hVal) 
{
	ui.openGLWidget->updateHScroll(hVal);
	emit scrollValuesChanged (hVal, ui.verticalScrollBar->value());
}

void roiImageBox::vertValueChanged(int vVal) 
{
	ui.openGLWidget->updateVScroll(vVal);
	emit scrollValuesChanged (ui.horizontalScrollBar->value(), vVal);
}

void roiImageBox::onScaleChanged(double newGLScale, double newImageScale)
{
	ui.labelZoom->setText(QString::number(int(newImageScale * 100)));
	emit scaleChanged(newGLScale, newImageScale );
}
