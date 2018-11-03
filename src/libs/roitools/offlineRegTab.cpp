#include "offlineRegTab.h"
#include "roiImageBox.h"
#include "roiParamWidget.h"

#include "commonTabs.h"
#include "unitSwitchLabel.h"
#include "applog.h"
#include "common/june_defs.h"
#include "ProcessParameters.h"

using namespace LandaJune;
using namespace Parameters;

#define OFFREGTAB_SCOPED_LOG PRINT_INFO6 << "[offlineRegTab] : "
#define OFFREGTAB_SCOPED_WARNING PRINT_WARNING << "[offlineRegTab] : "
#define OFFREGTAB_SCOPED_ERROR PRINT_ERROR << "[offlineRegTab] : "

double offlineRegTab::toMMX(const int val_pxx ) const { return val_pxx * _Pixel2MM_X;}
double offlineRegTab::toMMY(const int val_pxy ) const { return val_pxy * _Pixel2MM_Y;}


offlineRegTab::offlineRegTab(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	_leftImageBox = ui.leftROIWidget;
	_rightImageBox = ui.rightROIWidget;

	_leftImageBox->createWidget(roiImageBox::RENDER_STRIP);
	_rightImageBox->createWidget(roiImageBox::RENDER_STRIP);

	connect(_leftImageBox, &roiImageBox::imageLoaded, this, &offlineRegTab::onImageLoaded);
	connect(_rightImageBox, &roiImageBox::imageLoaded, this, &offlineRegTab::onImageLoaded);

	connect(_leftImageBox, &roiImageBox::roiChanged_strip, this, &offlineRegTab::onROIChanged);
	connect(_rightImageBox, &roiImageBox::roiChanged_strip, this, &offlineRegTab::onROIChanged);

	connect(_leftImageBox, &roiImageBox::scaleChanged, this, &offlineRegTab::onROIScaleChanged);
	connect(_rightImageBox, &roiImageBox::scaleChanged, this, &offlineRegTab::onROIScaleChanged);

	connect(_leftImageBox, &roiImageBox::scrollValuesChanged, this, &offlineRegTab::onROIScrollChanged);
	connect(_rightImageBox, &roiImageBox::scrollValuesChanged, this, &offlineRegTab::onROIScrollChanged);

	_leftImageBox->setFileMetaInfo("Load LEFT registration image file", ROITOOLS_KEY_LAST_REG_LEFT_FILE);
	_rightImageBox->setFileMetaInfo("Load RIGHT registration image file", ROITOOLS_KEY_LAST_REG_RIGHT_FILE);

	_leftImageBox->setAlias("LEFT REG");
	_rightImageBox->setAlias("RIGHT REG");
}

offlineRegTab::~offlineRegTab()
{
}

void offlineRegTab::setParameters(roiParamWidget* paramWidget, ProcessParametersPtr params)
{
	_paramWidget = paramWidget;
	_params.reset ( new ProcessParameters(*params));
	buildControls();
	setupInitialROIs ();
}

void offlineRegTab::onImageLoaded(QString strPath, LandaJune::CORE_ERROR err )
{
	if (err == RESULT_OK )
	{
		_paramWidget->enableControls(true);
	}
	else
	{
		_paramWidget->enableControls(_leftImageBox->hasImage() || _rightImageBox->hasImage() );
	}
}


void offlineRegTab::onPropertyChanged(QString propName, QVariant newVal)
{
	_params->setParamProperty(propName, newVal);
	_params->updateValues();
	updateLeftROIs();
	updateRightROIs();
}


void offlineRegTab::buildControls()
{
	_paramWidget->clear();
	_paramWidget->setProcessParameters(_params);

	/*
	_colorCounterCombo = _paramWidget->addComboBox("Number of colors");
	_colorCounterCombo->addItem("4");
	_colorCounterCombo->addItem("7");

	connect(_colorCounterCombo, QComboBox::currentTextChanged, this, &offlineRegTab::onColorCountChanged);

	_colorCounterCombo->setCurrentText(QString::number(_params->ColorArray().size()));

	*/
	_paramWidget->addControl( "I2SROIMarginX_mm", "Triangle ROI margin X", true);
	_paramWidget->addControl( "I2SROIMarginY_mm", "Triangle ROI margin Y", true);
	_paramWidget->addSpacer(20,15);
	_paramWidget->addControl( "C2CROIMarginX_mm", "C2C ROI margin X", true);
	_paramWidget->addControl( "C2CROIMarginY_mm", "C2C ROI margin Y", true);
	_paramWidget->addFinalSpacer();

	connect(_paramWidget, &roiParamWidget::propertyChanged, this, &offlineRegTab::onPropertyChanged);
	connect(_paramWidget, &roiParamWidget::done, this, &offlineRegTab::editDone);
}

void offlineRegTab::onColorCountChanged ( const QString&  newVal )
{
	auto const iVal = newVal.toInt();
	// TODO : validation
	if (iVal != 0)
	{
		
	}
}

void offlineRegTab::onROIScaleChanged(double glScale, double imageScale)
{
	auto targetWidget = ( dynamic_cast<roiImageBox*>(sender()) == _leftImageBox ) ? _rightImageBox : _leftImageBox;
	targetWidget->updateScaleFromExternal(glScale, imageScale);
}

void offlineRegTab::onROIScrollChanged(int hScroll, int vScroll)
{
	auto targetWidget = ( dynamic_cast<roiImageBox*>(sender()) == _leftImageBox ) ? _rightImageBox : _leftImageBox;
	targetWidget->updateScrollsFromExternal(hScroll, vScroll);
}

void offlineRegTab::onROIChanged(const QVector<QPoint> ptArray)
{
	if ( ptArray.empty() || ptArray.size() != _params->C2CROIArrayLeft_px().size() + 1 )
		return;

	const auto targetWidget = ( dynamic_cast<roiImageBox*>(sender()) == _leftImageBox ) ? _rightImageBox : _leftImageBox;
	
	recalculateOffsets(ptArray, ( dynamic_cast<roiImageBox*>(sender()) == _leftImageBox )  );

	if (targetWidget == _rightImageBox)
	{
		updateRightROIs();
	}
	else
	{
		updateLeftROIs();
	}
}

void offlineRegTab::updateLeftROIs()
{
	const auto i2sROILeft = _params->I2SRectLeft_px();
	const auto c2cROIsLeft = _params->C2CROIArrayLeft_px();

		_leftImageBox->updateROIs_Strip(i2sROILeft, c2cROIsLeft, 
		{
			_params->I2SMarginX_px(),
			_params->I2SMarginY_px()
		},
		{
			_params->C2CMarginX_px(),
			_params->C2CMarginY_px()
		},
		_params->C2CCircleDiameter_px()
	);
}

void offlineRegTab::updateRightROIs()
{
	const auto i2sROIRight = _params->I2SRectRight_px();
	const auto c2cROIsRight = _params->C2CROIArrayRight_px();


	_rightImageBox->updateROIs_Strip(i2sROIRight, c2cROIsRight, 
	{
		_params->I2SMarginX_px(),
		_params->I2SMarginY_px()
	},
	{
		_params->C2CMarginX_px(),
		_params->C2CMarginY_px()
	},
	_params->C2CCircleDiameter_px()
	);

}

void offlineRegTab::setupLeftROIs()
{
	const auto i2sROILeft = _params->I2SRectLeft_px();
	const auto c2cROIsLeft = _params->C2CROIArrayLeft_px();

		_leftImageBox->setInitialROIs_Strip(i2sROILeft, c2cROIsLeft, 
		{
			_params->I2SMarginX_px(),
			_params->I2SMarginY_px()
		},
		{
			_params->C2CMarginX_px(),
			_params->C2CMarginY_px()
		},
		_params->C2CCircleDiameter_px()
	);
}

void offlineRegTab::setupRightROIs()
{
	const auto i2sROIRight = _params->I2SRectRight_px();
	const auto c2cROIsRight = _params->C2CROIArrayRight_px();


	_rightImageBox->setInitialROIs_Strip(i2sROIRight, c2cROIsRight, 
	{
		_params->I2SMarginX_px(),
		_params->I2SMarginY_px()
	},
	{
		_params->C2CMarginX_px(),
		_params->C2CMarginY_px()
	},
	_params->C2CCircleDiameter_px()
	);
}

void offlineRegTab::setupInitialROIs() 
{
	setupLeftROIs();
	setupRightROIs();
}

void offlineRegTab::recalculateI2SOffset(const QPoint& pt, bool bLeft   )
{
	auto const offsetFromLeftEdge_px = bLeft ? _params->OffsetFromLeftEdge_mm() : 0;
	auto const scanStartToPaperEdge_px = bLeft ? _params->ScanStartToPaperEdgeOffset_mm() : 0;

	QPointF mmPoint = { toMMX(pt.x()), toMMY(pt.y()) };
	const auto overallOffset = scanStartToPaperEdge_px + offsetFromLeftEdge_px;
	mmPoint.setX(mmPoint.x() - overallOffset);

	//const auto oldI2SOffsetX = _params->I2SOffsetFromPaperEdgeX_mm();
	//const auto oldI2SOffsetY = _params->I2SOffsetFromPaperEdgeY_mm();
	//const auto newI2SOffsetX = mmPoint.x();
	//const auto newI2SOffsetY = mmPoint.y();
	//OFFREGTAB_SCOPED_LOG << "OLD I2S offset : [" << oldI2SOffsetX << "," << oldI2SOffsetY << "]";
	//OFFREGTAB_SCOPED_LOG << "NEW I2S offset : [" << newI2SOffsetX << "," << newI2SOffsetY << "]";

	_params->setI2SOffsetFromPaperEdgeX_mm(mmPoint.x());
	_params->setI2SOffsetFromPaperEdgeY_mm(mmPoint.y());
}

void offlineRegTab::recalculateOffsets(const QVector<QPoint>& pts, bool bLeft)
{
	_Pixel2MM_X = _params->Pixel2MM_X();
	_Pixel2MM_Y = _params->Pixel2MM_Y();

	recalculateI2SOffset(pts[0], bLeft);

	auto const newI2SCornerPt = pts[0];

	// calculate C2C offsets
	
	QVector<QSizeF> c2cOffsets_mm;
	for ( int i = 1; i < pts.size(); i++)
	{
		c2cOffsets_mm << QSizeF (
								toMMX(pts[i].x() - newI2SCornerPt.x()),
								toMMY(pts[i].y() - newI2SCornerPt.y())
							);
	}
	
	_params->setC2COffsetsArray(c2cOffsets_mm);
	_params->updateValues();
}
