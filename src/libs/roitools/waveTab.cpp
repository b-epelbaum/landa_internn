#include "waveTab.h"
#include "roiimagebox.h"
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
	connect(_waveImageBox, &roiImageBox::roiChanged_waveTriangle, this, &waveTab::onWaveTriangleChanged);

	connect(_waveImageBox, &roiImageBox::unitsChanged, this, &waveTab::onUnitsChanged);

	connect(_waveImageBox, &roiImageBox::doubleClick, this, &waveTab::onDoubleClick);

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

	_Pixel2MM_X = _params->Pixel2MM_X();
	_Pixel2MM_Y = _params->Pixel2MM_Y();

	_originalColorTriplets = _params->ColorArray();
	
	buildControls();
	setupInitialROIs ();
	
	_waveImageBox->setpx2mmRatio(_Pixel2MM_X, _Pixel2MM_Y);
}

void waveTab::setupInitialROIs() const
{
	_waveImageBox->setInitialROIs_Wave(
				  _params->WaveTriangleROI_px()
				, _params->WaveROI_px(), 
				{
					_params->WaveTriangleMarginX_px(),
					_params->WaveTriangleMarginY_px()
				}
	);
}

void waveTab::updateROIs() const
{
	_waveImageBox->updateROIs_Wave(
				  _params->WaveTriangleROI_px()
				, _params->WaveROI_px(), 
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

	auto colorCombo = _paramWidget->addComboBox("Number of colors");
	colorCombo->addItem("4");
	colorCombo->addItem("7");
	colorCombo->setCurrentText(QString::number(_params->ColorArray().size()));
	connect(colorCombo, &QComboBox::currentTextChanged, this, &waveTab::onColorCountChanged);

	_paramWidget->addControl( "WaveTriangleCornerX_mm", "Wave Triangle corner X", true);
	_paramWidget->addControl( "WaveTriangleCornerY_mm", "Wave Triangle corner Y", true);
	_paramWidget->addSpacer(20,15);

	_paramWidget->addControl( "WaveTriangleMarginX_mm", "Triangle ROI Horizontal Margin", true);
	_paramWidget->addControl( "WaveTriangleMarginY_mm", "Triangle ROI Vertical Margin", true);
	_paramWidget->addSpacer(20,15);
	_paramWidget->addControl( "WaveSideMarginsX_mm", "Wave side horizontal offset", true);
	_paramWidget->addSpacer(20,15);
	_paramWidget->addControl( "WaveOffsetFromCornerToFirstLineCenter_mm", "Offset from Triangle to First Row", true);
	_paramWidget->addSpacer(20,15);
	_paramWidget->addControl( "WaveFirstLineCircleMarginY_mm", "Wave ROI vertical margin", true);
	_paramWidget->addSpacer(20,15);
	_paramWidget->addSpacer(20,15);
	_paramWidget->addControl( "WaveNumberOfColorDotsPerLine", "Color dots per line number", false, false);
	_paramWidget->addFinalSpacer();

	connect(_paramWidget, &roiParamWidget::propertyChanged, this, &waveTab::onPropertyChanged);
	connect(_paramWidget, &roiParamWidget::done, this, &waveTab::editDone);
}

void waveTab::setFullScreen(bool bSet)
{
	emit wantFullScreen (_bFullScreen);
}

void waveTab::onColorCountChanged ( const QString&  newVal )
{
	auto const iVal = newVal.toInt();
	if (iVal != 0)
	{
		QVector<LandaJune::Parameters::COLOR_TRIPLET> tempColorArray  (iVal);
		_params->setColorArray(tempColorArray);
		_params->updateValues();
		updateROIs();
	}
}

void waveTab::onUnitsChanged ( int oldUnits, int newUnits )
{
	_waveImageBox->updateUnits(oldUnits, newUnits);
}

void waveTab::onDoubleClick(QPoint pos)
{
	_bFullScreen = !_bFullScreen;
	setFullScreen(_bFullScreen);
}

void waveTab::onEditDone(bool bApply)
{
	if (bApply)
	{
		_params->setColorArray(_originalColorTriplets);
	}
	emit editDone(bApply);
}


void waveTab::onWaveTriangleChanged(QPoint newControlPoint)
{
	recalculateWaveTriangleOffset(newControlPoint);
	updateROIs();
}

void waveTab::recalculateWaveTriangleOffset(const QPoint& pt) const
{
	QPointF mmPoint = { toMMX(pt.x()), toMMY(pt.y()) };

	_params->setWaveTriangleCornerX_mm(toMMX(pt.x()));
	_params->setWaveTriangleCornerY_mm(toMMY(pt.y()));
	_params->updateValues();
}
