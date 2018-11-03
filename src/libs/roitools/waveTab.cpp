#include "waveTab.h"
#include "roiImageBox.h"
#include "roiParamWidget.h"

#include "commonTabs.h"
#include "applog.h"
#include "common/june_defs.h"
#include "ProcessParameters.h"


using namespace LandaJune;
using namespace Parameters;

#define OFFREGTAB_SCOPED_LOG PRINT_INFO5 << "[waveTab] : "
#define OFFREGTAB_SCOPED_WARNING PRINT_WARNING << "[waveTab] : "
#define OFFREGTAB_SCOPED_ERROR PRINT_ERROR << "[waveTab] : "


double waveTab::toMMX(const int val_pxx ) const { return val_pxx * _Pixel2MM_X;}
double waveTab::toMMY(const int val_pxy ) const { return val_pxy * _Pixel2MM_Y;}

waveTab::waveTab(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	_waveImageBox = ui.waveROIWidget;
	_waveImageBox->createWidget(roiImageBox::RENDER_WAVE);

	connect(_waveImageBox, &roiImageBox::imageLoaded, this, &waveTab::onImageLoaded);
	_waveImageBox->setFileMetaInfo("Load wave image file", ROITOOLS_KEY_LAST_WAVE_FILE);
	_waveImageBox->setAlias("WAVE IMG");
}

waveTab::~waveTab()
{
}


void waveTab::setParameters(roiParamWidget* paramWidget, ProcessParametersPtr params)
{
	_paramWidget = paramWidget;
	_params.reset ( new ProcessParameters(*params));
	buildControls();
	setupInitialROIs ();
}

void waveTab::setupInitialROIs() 
{
	const auto waveTriangleRect = _params->WaveTriangleROI_px();
	const auto waveROIRect = _params->WaveROI_px();
	
	_waveImageBox->setInitialROIs_Wave(waveTriangleRect, waveROIRect, 
		{
			_params->WaveTriangleMarginX_px(),
			_params->WaveTriangleMarginY_px()
		}
	);
}

void waveTab::updateROIs() 
{
	const auto waveTriangleRect = _params->WaveTriangleROI_px();
	const auto waveROIRect = _params->WaveROI_px();

	_waveImageBox->updateROIs_Wave(waveTriangleRect, waveROIRect, 
		{
			_params->WaveTriangleMarginX_px(),
			_params->WaveTriangleMarginY_px()
		}
	);
}

void waveTab::onImageLoaded(QString strPath, LandaJune::CORE_ERROR err )
{
	_paramWidget->enableControls(err == RESULT_OK);
}

void waveTab::onPropertyChanged(QString propName, QVariant newVal)
{
	_params->setParamProperty(propName, newVal);
	_params->updateValues();
	updateROIs();
}


void waveTab::buildControls()
{
	_paramWidget->clear();

	_paramWidget->setProcessParameters(_params);
	
	_paramWidget->addControl( "I2SROIMarginX_mm", "Triangle ROI margin X", true);
	_paramWidget->addControl( "I2SROIMarginY_mm", "Triangle ROI margin Y", true);
	_paramWidget->addSpacer(20,15);
	_paramWidget->addControl( "C2CROIMarginX_mm", "C2C ROI margin X", true);
	_paramWidget->addControl( "C2CROIMarginY_mm", "C2C ROI margin Y", true);
	_paramWidget->addFinalSpacer();

	connect(_paramWidget, &roiParamWidget::propertyChanged, this, &waveTab::onPropertyChanged);
	connect(_paramWidget, &roiParamWidget::done, this, &waveTab::editDone);
}

void waveTab::onROIChanged(const QVector<QPoint> ptArray)
{
	if ( ptArray.empty() || ptArray.size() != _params->C2CROIArrayLeft_px().size() + 1 )
		return;
	
	recalculateOffsets(ptArray);
	updateROIs();
}

void waveTab::recalculateI2SOffset(const QPoint& pt )
{
}


void waveTab::recalculateOffsets(const QVector<QPoint>& pts)
{
	_Pixel2MM_X = _params->Pixel2MM_X();
	_Pixel2MM_Y = _params->Pixel2MM_Y();

	recalculateI2SOffset(pts[0]);
}