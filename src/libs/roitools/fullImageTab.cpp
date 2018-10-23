#include "fullImageTab.h"

#include "commonTabs.h"
#include "applog.h"
#include "common/june_defs.h"

#define OFFREGTAB_SCOPED_LOG PRINT_INFO5 << "[fullImageTab] : "
#define OFFREGTAB_SCOPED_WARNING PRINT_WARNING << "[fullImageTab] : "
#define OFFREGTAB_SCOPED_ERROR PRINT_ERROR << "[fullImageTab] : "


fullImageTab::fullImageTab(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	_fullImageBox = ui.fullROIWidget;

	_fullImageBox->setFileMetaInfo("Load full image file", ROITOOLS_KEY_LAST_FULL_FILE);

	connect(ui.editNumColors, &QLineEdit::textEdited, this, &fullImageTab::onEditNumOfColorsEdited);
	connect(ui.editSpinnerStep, &QLineEdit::textEdited, this, &fullImageTab::onEditSpinnerStepEdited);

	connect(ui.spinTriangleOffsetX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &fullImageTab::onSpinTriangleOffsetXChanged);
	connect(ui.spinTriangleOffsetY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &fullImageTab::onSpinTriangleOffsetYChanged);
	connect(ui.spinA1X, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &fullImageTab::onSpinA1Changed);
	connect(ui.spinA2Y, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &fullImageTab::onSpinA2Changed);
}

fullImageTab::~fullImageTab()
{
}


void fullImageTab::onEditNumOfColorsEdited(const QString& text)
{
}

void fullImageTab::onEditSpinnerStepEdited(const QString& text)
{
}

void fullImageTab::onSpinTriangleOffsetXChanged(double dVal)
{
}

void fullImageTab::onSpinTriangleOffsetYChanged(double dVal)
{
}

void fullImageTab::onSpinA1Changed(double dVal)
{
}

void fullImageTab::onSpinA2Changed(double dVal)
{
}
