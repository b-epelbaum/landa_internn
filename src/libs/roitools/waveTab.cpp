#include "waveTab.h"

#include "commonTabs.h"
#include "applog.h"
#include "common/june_defs.h"

#define OFFREGTAB_SCOPED_LOG PRINT_INFO5 << "[waveTab] : "
#define OFFREGTAB_SCOPED_WARNING PRINT_WARNING << "[waveTab] : "
#define OFFREGTAB_SCOPED_ERROR PRINT_ERROR << "[waveTab] : "


waveTab::waveTab(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	_waveImageBox = ui.waveROIWidget;

	_waveImageBox->setFileMetaInfo("Load wave image file", ROITOOLS_KEY_LAST_WAVE_FILE);

	connect(ui.editNumColors, &QLineEdit::textEdited, this, &waveTab::onEditNumOfColorsEdited);
	connect(ui.editSpinnerStep, &QLineEdit::textEdited, this, &waveTab::onEditSpinnerStepEdited);

	connect(ui.spinTriangleOffsetX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &waveTab::onSpinTriangleOffsetXChanged);
	connect(ui.spinTriangleOffsetY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &waveTab::onSpinTriangleOffsetYChanged);
	connect(ui.spinA1X, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &waveTab::onSpinA1Changed);
	connect(ui.spinA2Y, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &waveTab::onSpinA2Changed);
}

waveTab::~waveTab()
{
}

void waveTab::onEditNumOfColorsEdited(const QString& text)
{
}

void waveTab::onEditSpinnerStepEdited(const QString& text)
{
}

void waveTab::onSpinTriangleOffsetXChanged(double dVal)
{
}

void waveTab::onSpinTriangleOffsetYChanged(double dVal)
{
}

void waveTab::onSpinA1Changed(double dVal)
{
}

void waveTab::onSpinA2Changed(double dVal)
{
}
