#include "fullImageTab.h"
#include "roiImageBox.h"
#include "roiParamWidget.h"

#include "commonTabs.h"
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
	_fullImageBox->createWidget(roiImageBox::RENDER_FULL);

	connect(_fullImageBox, &roiImageBox::imageLoaded, this, &fullImageTab::onImageLoaded);
	_fullImageBox->setFileMetaInfo("Load full image file", ROITOOLS_KEY_LAST_FULL_FILE);
	_fullImageBox->setAlias("FULL IMG");
}

fullImageTab::~fullImageTab()
{
}

void fullImageTab::setParameters(roiParamWidget* paramWidget, ProcessParametersPtr params)
{
	_paramWidget = paramWidget;
	_params.reset ( new ProcessParameters(*params));
	buildControls();
	setupInitialROIs ();
}

void fullImageTab::setupInitialROIs() 
{
	const auto i2sROILeft = _params->I2SRectLeft_px();
	const auto c2cROIsLeft = _params->C2CROIArrayLeft_px();
	
	const auto i2sROIRight = _params->I2SRectRight_px();
	const auto c2cROIsRight = _params->C2CROIArrayRight_px();

	_fullImageBox->setInitialROIs_Full(i2sROILeft, i2sROIRight, c2cROIsLeft, c2cROIsRight,
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

void fullImageTab::updateROIs() 
{
	const auto i2sROILeft = _params->I2SRectLeft_px();
	const auto c2cROIsLeft = _params->C2CROIArrayLeft_px();
	
	const auto i2sROIRight = _params->I2SRectRight_px();
	const auto c2cROIsRight = _params->C2CROIArrayRight_px();

		_fullImageBox->updateROIs_Full(i2sROILeft, i2sROIRight, c2cROIsLeft, c2cROIsRight,
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



void fullImageTab::onImageLoaded(QString strPath, LandaJune::CORE_ERROR err )
{
	_paramWidget->enableControls(err == RESULT_OK);
}

void fullImageTab::onPropertyChanged(QString propName, QVariant newVal)
{
	_params->setParamProperty(propName, newVal);
	_params->updateValues();
	updateROIs();
}


void fullImageTab::buildControls()
{
	_paramWidget->clear();

	_paramWidget->setProcessParameters(_params);
	
	_paramWidget->addControl( "I2SROIMarginX_mm", "Triangle ROI margin X", true);
	_paramWidget->addControl( "I2SROIMarginY_mm", "Triangle ROI margin Y", true);
	_paramWidget->addSpacer(20,15);
	_paramWidget->addControl( "C2CROIMarginX_mm", "C2C ROI margin X", true);
	_paramWidget->addControl( "C2CROIMarginY_mm", "C2C ROI margin Y", true);
	_paramWidget->addFinalSpacer();

	connect(_paramWidget, &roiParamWidget::propertyChanged, this, &fullImageTab::onPropertyChanged);
	connect(_paramWidget, &roiParamWidget::done, this, &fullImageTab::editDone);
}

void fullImageTab::onROIChanged(const QVector<QPoint> ptArray)
{
	if ( ptArray.empty() || ptArray.size() != _params->C2CROIArrayLeft_px().size() + 1 )
		return;
	
	recalculateOffsets(ptArray);
	updateROIs();
}

void fullImageTab::recalculateI2SOffset(const QPoint& pt )
{
	auto const offsetFromLeftEdge_px = _params->OffsetFromLeftEdge_mm();
	auto const scanStartToPaperEdge_px = _params->ScanStartToPaperEdgeOffset_mm();

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


void fullImageTab::recalculateOffsets(const QVector<QPoint>& pts)
{
	_Pixel2MM_X = _params->Pixel2MM_X();
	_Pixel2MM_Y = _params->Pixel2MM_Y();

	recalculateI2SOffset(pts[0]);

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
