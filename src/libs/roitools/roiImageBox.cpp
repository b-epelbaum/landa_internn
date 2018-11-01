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
	_horizontalBar = ui.horizontalScrollBar;
	_verticalBar = ui.verticalScrollBar;
	_zoomButt = ui.zoomButt;

	connect(_renderWidget, &roiRenderWidget::i2sPosChanged, this, &roiImageBox::i2sPosChanged);
	connect(_renderWidget, &roiRenderWidget::c2cPosChanged, this, &roiImageBox::c2cPosChanged);

	connect(_renderWidget, &roiRenderWidget::cursorPos, this, &roiImageBox::onImageCursorPos);
	connect(_renderWidget, &roiRenderWidget::scaleChanged, this, &roiImageBox::onScaleChanged);
	
	connect(_horizontalBar, &QScrollBar::valueChanged, this, &roiImageBox::onHorizonalScrollbarValueChanged);
	connect(_verticalBar, &QScrollBar::valueChanged, this, &roiImageBox::onVerticalScrollbarValueChanged);

	_renderWidget->assignScrollBars(_horizontalBar, _verticalBar);
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

	_zoomButt->setMenu(menu);
	_zoomButt->setPopupMode(QToolButton::InstantPopup);
	_zoomButt->setEnabled(false);
	_horizontalBar->setEnabled(false);
	_verticalBar->setEnabled(false);
}

roiImageBox::~roiImageBox()
{
}


void roiImageBox::onHorizonalScrollbarValueChanged(int hVal) 
{
	if (!_renderWidget->hasImage())
		return;

	_renderWidget->redrawAll();
	emit scrollValuesChanged (hVal, -1);
}

void roiImageBox::onVerticalScrollbarValueChanged(int vVal) 
{
	if (!_renderWidget->hasImage())
		return;

	_renderWidget->redrawAll();
	emit scrollValuesChanged (-1, vVal);
}

LandaJune::CORE_ERROR roiImageBox::setImage(const QString& strFilePath)
{
	auto err = _renderWidget->setImage(strFilePath);
	_zoomButt->setEnabled(err == LandaJune::CORE_ERROR::RESULT_OK);
	_horizontalBar->setEnabled(err == LandaJune::CORE_ERROR::RESULT_OK);
	_verticalBar->setEnabled(err == LandaJune::CORE_ERROR::RESULT_OK);

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
	if (!_renderWidget->hasImage())
		return;

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

void roiImageBox::updateScaleFromExternal(double glScale, double imageScale)
{
	if (!_renderWidget->hasImage())
		return;
	
	ui.labelZoom->setText(QString::number(int(imageScale * 100)));
	_renderWidget->updateScaleFromExternal(glScale, imageScale);
}

void roiImageBox::updateScrollsFromExternal(int hScrollVal, int vScrollVal)
{
	if (!_renderWidget->hasImage())
		return;

	if (vScrollVal != -1)
	{
		disconnect(_verticalBar, &QScrollBar::valueChanged, this, &roiImageBox::onVerticalScrollbarValueChanged);
		_verticalBar->setValue(vScrollVal);
		connect(_verticalBar, &QScrollBar::valueChanged, this, &roiImageBox::onVerticalScrollbarValueChanged);
	}

	if (hScrollVal != -1)
	{
		disconnect(_horizontalBar, &QScrollBar::valueChanged, this, &roiImageBox::onHorizonalScrollbarValueChanged);
		_horizontalBar->setValue(hScrollVal);
		connect(_horizontalBar, &QScrollBar::valueChanged, this, &roiImageBox::onHorizonalScrollbarValueChanged);
	}
		
	_renderWidget->redrawAll();
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
	if (!_renderWidget->hasImage())
		return;

	QString strText = QString("X = %1, Y = %2").arg(pt.x()).arg(pt.y());
	if (!size.isEmpty() )
		strText = QString("X1 = %1, Y1 = %2, X2 = %3, Y2 = %4 [ %5, %6 ]").arg(pt.x()).arg(pt.y()).arg(pt.x() + size.width()).arg(pt.y()+ size.height()).arg(size.width()).arg(size.height());;
	ui.imageCoordsLabel->setText(strText);
}

void roiImageBox::onScaleChanged(double newGLScale, double newImageScale)
{
	if (!_renderWidget->hasImage())
		return;

	ui.labelZoom->setText(QString::number(int(newImageScale * 100)));
	emit scaleChanged(newGLScale, newImageScale );
}
