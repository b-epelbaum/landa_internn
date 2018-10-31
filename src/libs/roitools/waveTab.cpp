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


waveTab::waveTab(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	_waveImageBox = ui.waveROIWidget;
	_waveImageBox->setFileMetaInfo("Load wave image file", ROITOOLS_KEY_LAST_WAVE_FILE);
}

waveTab::~waveTab()
{
}


void waveTab::setParameters(roiParamWidget* paramWidget, ProcessParametersPtr params)
{
	_paramWidget = paramWidget;
	_params.reset ( new ProcessParameters(*params));
	buildControls();
}

void waveTab::onPropertyChanged(QString propName, QVariant newVal)
{
	recalculate();
}


void waveTab::buildControls()
{
	_paramWidget->clear();
		
	connect(_paramWidget, &roiParamWidget::propertyChanged, this, &waveTab::onPropertyChanged);
}

void waveTab::recalculate()
{

}