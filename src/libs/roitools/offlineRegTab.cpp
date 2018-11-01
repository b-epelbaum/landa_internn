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

class QSettings;

offlineRegTab::offlineRegTab(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	_leftImageBox = ui.leftROIWidget;
	_rightImageBox = ui.rightROIWidget;

	connect(_leftImageBox, &roiImageBox::i2sPosChanged, this, &offlineRegTab::oni2sPosChanged);
	connect(_leftImageBox, &roiImageBox::c2cPosChanged, this, &offlineRegTab::onc2cPosChanged);
	
	connect(_leftImageBox, &roiImageBox::scaleChanged, this, &offlineRegTab::onROIScaleChanged);
	connect(_rightImageBox, &roiImageBox::scaleChanged, this, &offlineRegTab::onROIScaleChanged);

	connect(_leftImageBox, &roiImageBox::scrollValuesChanged, this, &offlineRegTab::onROIScrollChanged);
	connect(_rightImageBox, &roiImageBox::scrollValuesChanged, this, &offlineRegTab::onROIScrollChanged);

	_leftImageBox->setFileMetaInfo("Load LEFT registration image file", ROITOOLS_KEY_LAST_REG_LEFT_FILE);
	_rightImageBox->setFileMetaInfo("Load RIGHT registration image file", ROITOOLS_KEY_LAST_REG_RIGHT_FILE);
}

offlineRegTab::~offlineRegTab()
{
}

void offlineRegTab::setParameters(roiParamWidget* paramWidget, ProcessParametersPtr params)
{
	_paramWidget = paramWidget;
	_params.reset ( new ProcessParameters(*params));
	buildControls();
}

void offlineRegTab::onPropertyChanged(QString propName, QVariant newVal)
{
	recalculate();
}


void offlineRegTab::buildControls()
{
	_paramWidget->clear();

	/*
	_paramWidget->addDoubleSpinBox(0, DBL_MAX, 1.0, 0.0, "Triangle offset X", "I2SOffsetFromPaperEdgeX_mm", unitSwitchLabel::MM );
	_paramWidget->addDoubleSpinBox(0, DBL_MAX, 1.0, 0.0, "Triangle offset Y", "I2SOffsetFromPaperEdgeX_mm", unitSwitchLabel::MM );
	_paramWidget->addSpacer(20,15);
	_paramWidget->addDoubleSpinBox(0, DBL_MAX, 1.0, 0.0, "Triangle ROI margin X", "I2SMarginX_mm", unitSwitchLabel::MM );
	_paramWidget->addDoubleSpinBox(0, DBL_MAX, 1.0, 0.0, "Triangle ROI margin Y", "I2SMarginY_mm", unitSwitchLabel::MM );
	_paramWidget->addSpacer(20,15);
	_paramWidget->addDoubleSpinBox(0, DBL_MAX, 1.0, 0.0, "Color ROI margin X", "ColorROIMarginX_mm", unitSwitchLabel::MM );
	_paramWidget->addDoubleSpinBox(0, DBL_MAX, 1.0, 0.0, "Color ROI margin Y", "ColorROIMarginY_mm", unitSwitchLabel::MM );
	_paramWidget->addSpacer(20,15);
	_paramWidget->addDoubleSpinBox(0, DBL_MAX, 1.0, 0.0, "Color Center Distance X", "ColorCenterDistanceX_mm", unitSwitchLabel::MM );
	_paramWidget->addDoubleSpinBox(0, DBL_MAX, 1.0, 0.0, "Color Center Distance Y", "ColorCenterDistanceX_mm", unitSwitchLabel::MM );
	_paramWidget->addSpacer(20,15);
	_paramWidget->addDoubleSpinBox(0, DBL_MAX, 1.0, 0.0, "Pixel2MM ratio X", "Pixel2MM_X");
	_paramWidget->addDoubleSpinBox(0, DBL_MAX, 1.0, 0.0, "Pixel2MM ratio Y", "Pixel2MM_Y");
	_paramWidget->addSpacer(20,15);
	*/
	_colorCounterCombo = _paramWidget->addComboBox("Number of colors");
	_colorCounterCombo->addItem("4");
	_colorCounterCombo->addItem("7");

	connect(_paramWidget, &roiParamWidget::propertyChanged, this, &offlineRegTab::onPropertyChanged);
	setupROIs();
}

void offlineRegTab::recalculate()
{

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


void offlineRegTab::oni2sPosChanged(QPoint pt)
{
	// translate absolute i2s coordinate to mm

	_Pixel2MM_X = _params->Pixel2MM_X();
	_Pixel2MM_Y = _params->Pixel2MM_Y();
	QPointF mmPoint = { toMMX(pt.x()), toMMY(pt.y()) };

	// and offset it for paper edge origin
	const auto offsetScanToPaperEdge = _params->property("ScanStartToPaperEdgeOffset_mm").toFloat();
	mmPoint.setX(mmPoint.x() - offsetScanToPaperEdge);

	// update parameters :
	const auto oldI2SOffsetX = _params->I2SOffsetFromPaperEdgeX_mm();
	const auto oldI2SOffsetY = _params->I2SOffsetFromPaperEdgeY_mm();

	_params->setI2SOffsetFromPaperEdgeX_mm(mmPoint.x());
	_params->setI2SOffsetFromPaperEdgeY_mm(mmPoint.y());

	qDebug() << " I2S origin changed. [ " << oldI2SOffsetX << " , " << oldI2SOffsetY << " ] ==> [ " << mmPoint.x() << " , " << mmPoint.y()  << " ]";

	recalculate();
}

void offlineRegTab::onc2cPosChanged(int idx, QPoint pt)
{
	// offset c2c coordinates from I2S rect

	pt.setX(pt.x() - _params->I2SRectLeft_px().x());
	pt.setY(pt.y() - _params->I2SRectLeft_px().y());
	
	// translate absolute i2s coordinate to mm

	_Pixel2MM_X = _params->Pixel2MM_X();
	_Pixel2MM_Y = _params->Pixel2MM_Y();

	QPointF newPt = QPointF{ toMMX(pt.x()), toMMY(pt.y()) };
	
	// update parameters :
	auto sizes = _params->C2COffsetsArray();
	
	const auto oldC2COSize = sizes[idx];
	sizes[idx] = QSizeF(newPt.x(), newPt.y());

	_params->setC2COffsetsArray(sizes);
	
	qDebug() << " C2C # " << idx  << " origin changed. [ " << oldC2COSize << " ] ==> [ " << _params->C2COffsetsArray()[idx]  << " ]";

	recalculate();
}	
	
void offlineRegTab::setupROIs() const
{
	const auto i2sROILeft = _params->I2SRectLeft_px();
	const auto i2sROIRight = _params->I2SRectRight_px();
	const auto c2cROIsLeft = _params->C2CROIArrayLeft_px();
	const auto c2cROIsRight = _params->C2CROIArrayRight_px();

	_leftImageBox->setInitialROIs(i2sROILeft, c2cROIsLeft, 
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

	_rightImageBox->setInitialROIs(i2sROIRight, c2cROIsRight, 
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

void offlineRegTab::updateROIs() const
{
	const auto i2sROI = _params->I2SRectLeft_px();
	const auto c2cROIs = _params->C2CROIArrayLeft_px();

	_leftImageBox->updateROIs(i2sROI, c2cROIs);
}