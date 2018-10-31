#include "fullImageTab.h"

#include "roiImageBox.h"

#include "commonTabs.h"
#include "unitSwitchLabel.h"
#include "applog.h"
#include "common/june_defs.h"
#include "ProcessParameters.h"

using namespace LandaJune;
using namespace Parameters;

#define OFFREGTAB_SCOPED_LOG PRINT_INFO5 << "[fullImageTab] : "
#define OFFREGTAB_SCOPED_WARNING PRINT_WARNING << "[fullImageTab] : "
#define OFFREGTAB_SCOPED_ERROR PRINT_ERROR << "[fullImageTab] : "

double fullImageTab::toMMX(const int val_pxx ) const { return val_pxx * _Pixel2MM_X;}
double fullImageTab::toMMY(const int val_pxy ) const { return val_pxy * _Pixel2MM_Y;}


fullImageTab::fullImageTab(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	_fullImageBox = ui.fullImageROIWidget;

	connect(_fullImageBox, &roiImageBox::i2sPosChanged, this, &fullImageTab::oni2sPosChanged);
	connect(_fullImageBox, &roiImageBox::c2cPosChanged, this, &fullImageTab::onc2cPosChanged);
}

fullImageTab::~fullImageTab()
{
}

void fullImageTab::setParameters(roiParamWidget* paramWidget, ProcessParametersPtr params)
{
	_paramWidget = paramWidget;
	_params.reset ( new ProcessParameters(*params));
	buildControls();
}

void fullImageTab::onPropertyChanged(QString propName, QVariant newVal)
{
	recalculate();
}

void fullImageTab::oni2sPosChanged(QPoint pt)
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

void fullImageTab::onc2cPosChanged(int idx, QPoint pt)
{
	// offset c2c coordinates from I2S rect

	pt.setX(pt.x() - _params->I2RectLeft_px().x());
	pt.setY(pt.y() - _params->I2RectLeft_px().y());
	
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


void fullImageTab::buildControls()
{
	_paramWidget->clear();

	_fullImageBox = ui.fullImageROIWidget;
	_fullImageBox->setFileMetaInfo("Load full image file", ROITOOLS_KEY_LAST_FULL_FILE);

	_paramWidget->setProcessParameters(_params);

	//_paramWidget->addControl("SubstrateWidth_mm",	"Substrate Width",	true);
	//_paramWidget->addControl("SubstrateHeight_mm",	"Substrate Height", true);
	//_paramWidget->addSpacer(20,15);
	//auto spinner = dynamic_cast<QDoubleSpinBox*>(_paramWidget->addControl("Pixel2MM_X", "Pixel to MM ratio, X", true));
	//spinner->setDecimals(8);
	//spinner = dynamic_cast<QDoubleSpinBox*>(_paramWidget->addControl("Pixel2MM_Y", "Pixel to MM ratio, Y", true));
	//spinner->setDecimals(8);
	
	//_paramWidget->addSpacer(20,15);
	_paramWidget->addControl( "ScanStartToPaperEdgeOffset_mm", "Scan Offset to Paper Edge", true);
	_paramWidget->addControl( "OffsetFromLeftEdge_mm", "Left Offset to Paper Edge", true);
	_paramWidget->addControl( "LeftStripWidth_mm", "Left Strip Width", true);
	_paramWidget->addSpacer(20,15);
	_paramWidget->addControl( "I2SOffsetFromPaperEdgeX_mm", "I2S Offset from Paper Edge, X", true);
	_paramWidget->addControl( "I2SOffsetFromPaperEdgeY_mm", "I2S Offset from Paper Edge, Y", true);
	_paramWidget->addControl( "I2SWidth_mm", "I2S Width", true);
	_paramWidget->addControl( "I2SHeight_mm", "I2S Height", true);
	_paramWidget->addControl( "I2SROIMarginX_mm", "I2S ROI Margin, X", true);
	_paramWidget->addControl( "I2SROIMarginY_mm", "I2S ROI Margin, Y", true);
	_paramWidget->addSpacer(20,15);
	_paramWidget->addControl( "C2CDistanceBetweenDotsX_mm", "C2C Dot center distance, X", true);
	_paramWidget->addControl( "C2CDistanceBetweenDotsY_mm", "C2C Dot center distance, Y", true);
	_paramWidget->addControl( "C2CDROIMarginX_mm", "C2C Dot Margin, X", true);
	_paramWidget->addControl( "C2CDROIMarginY_mm", "C2C Dot Margin, Y", true);
	_paramWidget->addFinalSpacer();

	/*
	_paramWidget->addDoubleSpinBox(-DBL_MAX, DBL_MAX, 1.0, _params->property("ScanStartToPaperEdgeOffset_mm").toDouble(), "Scan Offset to Paper Edge", "ScanStartToPaperEdgeOffset_mm", unitSwitchLabel::MM );
	_paramWidget->addSpacer(20,15);
	_paramWidget->addDoubleSpinBox(-DBL_MAX, DBL_MAX, 1.0, 0.0, "Left Offset from Paper Edge", "OffsetFromLeftEdge_mm", unitSwitchLabel::MM );
	_paramWidget->addSpacer(20,15);
	_paramWidget->addDoubleSpinBox(-DBL_MAX, DBL_MAX, 1.0, 0.0, "Triangle ROI margin X", "I2SMarginX_mm", unitSwitchLabel::MM );
	_paramWidget->addDoubleSpinBox(-DBL_MAX, DBL_MAX, 1.0, 0.0, "Triangle ROI margin Y", "I2SMarginY_mm", unitSwitchLabel::MM );
	_paramWidget->addSpacer(20,15);
	_paramWidget->addDoubleSpinBox(-DBL_MAX, DBL_MAX, 1.0, 0.0, "Color ROI margin X", "ColorROIMarginX_mm", unitSwitchLabel::MM );
	_paramWidget->addDoubleSpinBox(-DBL_MAX, DBL_MAX, 1.0, 0.0, "Color ROI margin Y", "ColorROIMarginY_mm", unitSwitchLabel::MM );
	_paramWidget->addSpacer(20,15);
	_paramWidget->addDoubleSpinBox(-DBL_MAX, DBL_MAX, 1.0, 0.0, "Color Center Distance X", "ColorCenterDistanceX_mm", unitSwitchLabel::MM );
	_paramWidget->addDoubleSpinBox(-DBL_MAX, DBL_MAX, 1.0, 0.0, "Color Center Distance Y", "ColorCenterDistanceX_mm", unitSwitchLabel::MM );
	_paramWidget->addSpacer(20,15);
	_paramWidget->addDoubleSpinBox(-DBL_MAX, DBL_MAX, 1.0, 0.0, "Pixel2MM ratio X", "Pixel2MM_X");
	_paramWidget->addDoubleSpinBox(-DBL_MAX, DBL_MAX, 1.0, 0.0, "Pixel2MM ratio Y", "Pixel2MM_Y");
	_paramWidget->addSpacer(20,15);

	auto comboBox = _paramWidget->addComboBox("Number of colors");
	comboBox->addItem("4");
	comboBox->addItem("7");

	*/
	connect(_paramWidget, &roiParamWidget::propertyChanged, this, &fullImageTab::onPropertyChanged);
	setupROIs();
}

void fullImageTab::recalculate()
{
	_params->updateValues();
	updateROIs();
}

void fullImageTab::setupROIs() const
{
	const auto i2sROI = _params->I2RectLeft_px();
	const auto c2cROIs = _params->C2CROIArrayLeft_px();

	_fullImageBox->setInitialROIs(i2sROI, c2cROIs, 
		{
			_params->I2SMarginX_px(),
			_params->I2SMarginY_px()
		},
		{
			_params->C2CMarginX_px(),
			_params->C2CMarginY_px()
		}
	);
}

void fullImageTab::updateROIs() const
{
	const auto i2sROI = _params->I2RectLeft_px();
	const auto c2cROIs = _params->C2CROIArrayLeft_px();

	_fullImageBox->updateROIs(i2sROI, c2cROIs);
}
