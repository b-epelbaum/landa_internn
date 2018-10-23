#include "offlineRegTab.h"
#include "roiWidget.h"
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QLineEdit>

#include "commonTabs.h"
#include "applog.h"
#include "common/june_defs.h"

#define OFFREGTAB_SCOPED_LOG PRINT_INFO6 << "[offlineRegTab] : "
#define OFFREGTAB_SCOPED_WARNING PRINT_WARNING << "[offlineRegTab] : "
#define OFFREGTAB_SCOPED_ERROR PRINT_ERROR << "[offlineRegTab] : "


class QSettings;

offlineRegTab::offlineRegTab(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	_leftImageBox = ui.leftROIWidget;
	_rightImageBox = ui.rightROIWidget;

	_leftImageBox->setFileMetaInfo("Load registration image file", ROITOOLS_KEY_LAST_REG_FILE);
	_rightImageBox->setFileMetaInfo("Load registration image file", ROITOOLS_KEY_LAST_REG_FILE);

	connect(_leftImageBox, &roiWidget::scaleChanged, this, &offlineRegTab::onLeftRightROIScaleChanged);
	connect(_rightImageBox, &roiWidget::scaleChanged, this, &offlineRegTab::onLeftRightROIScaleChanged);

	connect(_leftImageBox, &roiWidget::scrollValuesChanged, this, &offlineRegTab::onLeftRightROIScrollChanged);
	connect(_rightImageBox, &roiWidget::scrollValuesChanged, this, &offlineRegTab::onLeftRightROIScrollChanged);

	connect(ui.editNumColors, &QLineEdit::textEdited, this, &offlineRegTab::onEditNumOfColorsEdited);
	connect(ui.editSpinnerStep, &QLineEdit::textEdited, this, &offlineRegTab::onEditSpinnerStepEdited);

	connect(ui.spinTriangleOffsetX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &offlineRegTab::onSpinTriangleOffsetXChanged);
	connect(ui.spinTriangleOffsetY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &offlineRegTab::onSpinTriangleOffsetYChanged);
	connect(ui.spinA1X, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &offlineRegTab::onSpinA1Changed);
	connect(ui.spinA2Y, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &offlineRegTab::onSpinA2Changed);
}

offlineRegTab::~offlineRegTab()
{
}


void offlineRegTab::onLeftRightROIScaleChanged(float glScale, float imageScale)
{
	auto targetWidget = ( dynamic_cast<roiWidget*>(sender()) == _leftImageBox ) ? _rightImageBox : _leftImageBox;
	targetWidget->setScales(glScale, imageScale);
}

void offlineRegTab::onLeftRightROIScrollChanged(int hScroll, int vScroll)
{
	auto targetWidget = ( dynamic_cast<roiWidget*>(sender()) == _leftImageBox ) ? _rightImageBox : _leftImageBox;
	targetWidget->setScrolls(hScroll, vScroll);
}


void offlineRegTab::onEditNumOfColorsEdited(const QString& text)
{
}

void offlineRegTab::onEditSpinnerStepEdited(const QString& text)
{
}

void offlineRegTab::onSpinTriangleOffsetXChanged(double dVal)
{
}

void offlineRegTab::onSpinTriangleOffsetYChanged(double dVal)
{
}

void offlineRegTab::onSpinA1Changed(double dVal)
{
}

void offlineRegTab::onSpinA2Changed(double dVal)
{
}
