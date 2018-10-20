#include "roiview.h"
#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>

#include "applog.h"
#include "RenderWidget.h"


#define ROIVIEW_SCOPED_LOG PRINT_INFO6 << "[roiview] : "
#define ROIVIEW_SCOPED_WARNING PRINT_WARNING << "[roiview] : "
#define ROIVIEW_SCOPED_ERROR PRINT_ERROR << "[roiview] : "


roiview::roiview(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	_leftImageBox = ui.leftROIWidget;
	_rightImageBox = ui.rightROIWidget;


	connect(ui.loadLeftButt, &QPushButton::clicked, this, &roiview::onLeftImageLoad);
	connect(ui.loadRightButt, &QPushButton::clicked, this, &roiview::onRightImageLoad);

	connect(ui.editNumColors, &QLineEdit::textEdited, this, &roiview::onEditNumOfColorsEdited);
	connect(ui.editSpinnerStep, &QLineEdit::textEdited, this, &roiview::onEditSpinnerStepEdited);

	connect(ui.spinTriangleOffsetX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &roiview::onSpinTriangleOffsetXChanged);
	connect(ui.spinTriangleOffsetY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &roiview::onSpinTriangleOffsetYChanged);
	connect(ui.spinA1X, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &roiview::onSpinA1Changed);
	connect(ui.spinA2Y, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &roiview::onSpinA2Changed);
}

roiview::~roiview()
{
}

void roiview::onLeftImageLoad()
{
	auto filePath = selectBitmapFile();
	if (filePath.isEmpty() )
		return;

	_leftImageBox->setImage(filePath);
}

void roiview::onRightImageLoad()
{
}

void roiview::onEditNumOfColorsEdited(const QString& text)
{
}

void roiview::onEditSpinnerStepEdited(const QString& text)
{
}

void roiview::onSpinTriangleOffsetXChanged(double dVal)
{
}

void roiview::onSpinTriangleOffsetYChanged(double dVal)
{
}

void roiview::onSpinA1Changed(double dVal)
{
}

void roiview::onSpinA2Changed(double dVal)
{
}

QString roiview::selectBitmapFile()
{
	QSettings settings("Landa Corp", "June QCS");
	auto lastBitmapFolder = settings.value("ROITools/lastRegFilesFolder").toString();

	if ( lastBitmapFolder.isEmpty() )
	{
		lastBitmapFolder = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation)[0];
	}

	auto fileName = QFileDialog::getOpenFileName(this,
    tr("Load registration image file"), lastBitmapFolder, tr("Bitmap files (*.BMP *bmp)"));

	if (fileName.isEmpty())
	{
		ROIVIEW_SCOPED_LOG << "Loading registration image cancelled";
		return "";
	}

	ROIVIEW_SCOPED_LOG << "Loading registration image file  : " << fileName;
	settings.setValue("ROITools/lastRegFilesFolder", QFileInfo(fileName).absoluteDir().absolutePath());

	return fileName;
}