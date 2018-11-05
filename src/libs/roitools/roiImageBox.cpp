#include "roiImageBox.h"

#include "commonTabs.h"
#include <QMenuBar>

roiImageBox::roiImageBox(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	QAction *opeFileAction = ui.browseFileEdit->addAction(QIcon(":/roiTools/Resources/file_open.png"), QLineEdit::TrailingPosition);
	connect(opeFileAction, &QAction::triggered, this, &roiImageBox::onOpenFile);

	_horizontalBar = ui.horizontalScrollBar;
	_verticalBar = ui.verticalScrollBar;
	_zoomButt = ui.zoomButt;

	connect(_horizontalBar, &QScrollBar::valueChanged, this, &roiImageBox::onHorizonalScrollbarValueChanged);
	connect(_verticalBar, &QScrollBar::valueChanged, this, &roiImageBox::onVerticalScrollbarValueChanged);

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

	connect(ui.mmButt, &QToolButton::clicked, this, &roiImageBox::onChangeUnits);
	connect(ui.pxButt, &QToolButton::clicked, this, &roiImageBox::onChangeUnits);
}

roiImageBox::~roiImageBox()
{

}

void roiImageBox::updateUnits ( int oldUnits, int newUnits )
{
	auto _oldUnits = static_cast<unitSwitchLabel::LABEL_UNITS>(oldUnits);
	_currentUnits = static_cast<unitSwitchLabel::LABEL_UNITS>(newUnits);

	ui.mmButt->setChecked(_currentUnits == unitSwitchLabel::MM);
	ui.pxButt->setChecked(_currentUnits == unitSwitchLabel::PX);
	updateCursorPositionText (QPoint(), QSize());
}

void roiImageBox::onChangeUnits()
{
	const auto butt = dynamic_cast<QToolButton*>(sender());
	auto const oldUnits = _currentUnits;
	if (butt == ui.mmButt)
	{
		_currentUnits = unitSwitchLabel::MM;
	}
	else
	{
		_currentUnits = unitSwitchLabel::PX;
	}
	updateCursorPositionText (QPoint(), QSize());
	emit unitsChanged ( static_cast<int>(oldUnits), static_cast<int>(_currentUnits) );
}

void roiImageBox::createWidget(RENDER_WIDGET_TYPE rType)
{
	_renderWidgetType = rType;
	createRenderWidget();
}

void roiImageBox::setpx2mmRatio(double hRatio, double vRatio)
{
	_hRatio = hRatio;
	_vRatio = vRatio;
}

void roiImageBox::setC2CRoisLinedUp ( bool bVal )
{
	if (_renderWidgetType == RENDER_STRIP )
	{
		static_cast<roiRenderWidgetStrip*>(_renderWidget)->setC2CRoisLinedUp(bVal);
	}
}

void roiImageBox::createRenderWidget()
{
	// replace to 
	if (_renderWidgetType == RENDER_STRIP )
	{
		_renderWidget = new roiRenderWidgetStrip(this);
		connect(static_cast<roiRenderWidgetStrip*>(_renderWidget), &roiRenderWidgetStrip::roiChanged_edge, this, &roiImageBox::roiChanged_strip_edge);
		connect(static_cast<roiRenderWidgetStrip*>(_renderWidget), &roiRenderWidgetStrip::roiChanged_i2s, this, &roiImageBox::roiChanged_strip_i2s);
		connect(static_cast<roiRenderWidgetStrip*>(_renderWidget), &roiRenderWidgetStrip::roiChanged_c2c, this, &roiImageBox::roiChanged_strip_c2c);
	}
	else if (_renderWidgetType == RENDER_FULL )
	{
		_renderWidget = new roiRenderWidgetFull(this);
		//connect(_renderWidget, &roiRenderWidgetBase::roiChanged, this, &roiImageBox::roiChanged_full);
	}
	else if (_renderWidgetType == RENDER_WAVE )
	{
		_renderWidget = new roiRenderWidgetWave(this);
		//connect(_renderWidget, &roiRenderWidgetBase::roiChanged, this, &roiImageBox::roiChanged_wave);
	}

	if (_renderWidget != nullptr )
	{
		auto oldWidget = ui.pageGL->layout()->replaceWidget(ui.openGLWidget, _renderWidget );
		if ( oldWidget )
			delete oldWidget;
		connect(_renderWidget, &roiRenderWidgetBase::cursorPos, this, &roiImageBox::onImageCursorPos);
		connect(_renderWidget, &roiRenderWidgetBase::scaleChanged, this, &roiImageBox::onScaleChanged);
		connect(_renderWidget, &roiRenderWidgetBase::doubleClick, this, &roiImageBox::onDoubleClick);
		_renderWidget->assignScrollBars(_horizontalBar, _verticalBar);
	}
}


void roiImageBox::onDoubleClick (QPoint pos)
{
	ui.metaWidget->setVisible(!ui.metaWidget->isVisible());
	emit doubleClick (pos);
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

void roiImageBox::setImage(const QString& strFilePath)
{
	const auto err = _renderWidget->setImage(strFilePath);
	
	_zoomButt->setEnabled(err == LandaJune::CORE_ERROR::RESULT_OK);
	_horizontalBar->setEnabled(err == LandaJune::CORE_ERROR::RESULT_OK);
	_verticalBar->setEnabled(err == LandaJune::CORE_ERROR::RESULT_OK);
	ui.dimsWidget->setEnabled(err == LandaJune::CORE_ERROR::RESULT_OK);
	ui.zoomWidget->setEnabled(err == LandaJune::CORE_ERROR::RESULT_OK);
	ui.cursorInfoWidget->setEnabled(err == LandaJune::CORE_ERROR::RESULT_OK);

	if (err == LandaJune::CORE_ERROR::RESULT_OK)
	{
		displayMetaData();
	}
	emit imageLoaded(strFilePath, err);
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

void roiImageBox::updateCursorPositionText(QPoint pt, QSize size)
{
	if (!_renderWidget->hasImage())
		return;

	if ( pt.isNull() )
		pt = _lastPostionPt;
	else
		_lastPostionPt = pt;

	QString strText;
	if ( _currentUnits == unitSwitchLabel::MM )
		strText = QString("X = %1, Y = %2").arg(_lastPostionPt.x() * _hRatio, 0, 'f', 3).arg(_lastPostionPt.y() * _vRatio, 0, 'f', 3 );
	else
	{
		strText = QString("X = %1, Y = %2").arg(_lastPostionPt.x()).arg(_lastPostionPt.y());
	}

	if (!size.isEmpty() )
	{
		if ( _currentUnits == unitSwitchLabel::MM )
		{
			strText = QString("X1 = %1, Y1 = %2, X2 = %3, Y2 = %4 [ %5, %6 ]")
						.arg(pt.x() * _hRatio)
						.arg(pt.y() * _vRatio)
						.arg((pt.x() + size.width())* _hRatio)
						.arg((pt.y() + size.height()) * _vRatio)
						.arg(size.width()* _hRatio)
						.arg(size.height() * _vRatio);
		}
		else
		{
			strText = QString("X1 = %1, Y1 = %2, X2 = %3, Y2 = %4 [ %5, %6 ]").arg(pt.x()).arg(pt.y()).arg(pt.x() + size.width()).arg(pt.y()+ size.height()).arg(size.width()).arg(size.height());;
		}
	}
	ui.imageCoordsLabel->setText(strText);
}

void roiImageBox::onImageCursorPos(QPoint pt, QSize size)
{
	if (!_renderWidget->hasImage())
		return;

	updateCursorPositionText(pt, size);
}

void roiImageBox::onScaleChanged(double newGLScale, double newImageScale)
{
	if (!_renderWidget->hasImage())
		return;

	ui.labelZoom->setText(QString::number(int(newImageScale * 100)));
	emit scaleChanged(newGLScale, newImageScale );
}
