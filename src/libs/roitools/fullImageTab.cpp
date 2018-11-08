#include "fullImageTab.h"
#include "roiImageBox.h"
#include "roiParamWidget.h"

#include "commonTabs.h"
#include "applog.h"
#include "common/june_defs.h"
#include "ProcessParameters.h"

using namespace LandaJune;
using namespace Parameters;

#define FULLTAB_SCOPED_LOG PRINT_INFO6 << "[fullImageTab] : "
#define FULLTAB_SCOPED_WARNING PRINT_WARNING << "[fullImageTab] : "
#define FULLTAB_SCOPED_ERROR PRINT_ERROR << "[fullImageTab] : "

double fullImageTab::toMMX(const int val_pxx ) const { return val_pxx * _Pixel2MM_X;}
double fullImageTab::toMMY(const int val_pxy ) const { return val_pxy * _Pixel2MM_Y;}


fullImageTab::fullImageTab(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	_fullImageBox = ui.fullImageROIWidget;
	_fullImageBox->createWidget(roiImageBox::RENDER_FULL);

	connect(_fullImageBox, &roiImageBox::imageLoaded, this, &fullImageTab::onImageLoaded);
	connect(_fullImageBox, &roiImageBox::roiChanged_full_leftStripEdge, this, &fullImageTab::onLeftStripEdgeChanged);
	connect(_fullImageBox, &roiImageBox::roiChanged_full_rightStripEdge, this, &fullImageTab::onRightStripEdgeChanged);
	connect(_fullImageBox, &roiImageBox::roiChanged_full_pageEdge, this, &fullImageTab::onPageEdgeChanged);
	connect(_fullImageBox, &roiImageBox::roiChanged_full_i2s, this, &fullImageTab::onI2SROIChanged);
	connect(_fullImageBox, &roiImageBox::roiChanged_full_c2c, this, &fullImageTab::onC2CROIsChanged);
	
	connect(_fullImageBox, &roiImageBox::unitsChanged, this, &fullImageTab::onUnitsChanged);

	connect(_fullImageBox, &roiImageBox::doubleClick, this, &fullImageTab::onDoubleClick);

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

	_Pixel2MM_X = _params->Pixel2MM_X();
	_Pixel2MM_Y = _params->Pixel2MM_Y();

	_originalColorTriplets = _params->ColorArray();
	
	buildControls();
	setupInitialROIs ();
	
	_fullImageBox->setpx2mmRatio(_Pixel2MM_X, _Pixel2MM_Y);

}

void fullImageTab::setupInitialROIs() const
{
	const auto pageOffsetX = _params->ScanStartToPaperEdgeOffset_px();

	_fullImageBox->setInitialROIs_Full(
					_params->I2SRectLeft_px(), 
					_params->I2SRectRight_px(), 
					_params->C2CROIArrayLeft_px(), 
					_params->C2CROIArrayRight_px(), 
					_params->LeftStripRect_px(),
					_params->RightStripRect_px(),
					 pageOffsetX, 
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

void fullImageTab::updateROIs( bool bUpdateBoth) const
{
	const auto pageOffsetX = _params->ScanStartToPaperEdgeOffset_px();

	_fullImageBox->updateROIs_Full(
				_params->I2SRectLeft_px(), 
				_params->I2SRectRight_px(), 
				_params->C2CROIArrayLeft_px(), 
				_params->C2CROIArrayRight_px(), 
				_params->LeftStripRect_px(),
				_params->RightStripRect_px(),
				 pageOffsetX, 
				{
					_params->I2SMarginX_px(),
					_params->I2SMarginY_px()
				},
				{
					_params->C2CMarginX_px(),
					_params->C2CMarginY_px()
				},
				_params->C2CCircleDiameter_px(),
				bUpdateBoth
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
	updateROIs(true);
}


void fullImageTab::buildControls()
{
	_paramWidget->clear();
	_paramWidget->setProcessParameters(_params);
	
	auto colorCombo = _paramWidget->addComboBox("Number of colors");
	colorCombo->addItem("4");
	colorCombo->addItem("7");
	colorCombo->setCurrentText(QString::number(_params->ColorArray().size()));
	connect(colorCombo, &QComboBox::currentTextChanged, this, &fullImageTab::onColorCountChanged);

	_paramWidget->addSpacer(20,15);
	_paramWidget->addControl( "ScanStartToPaperEdgeOffset_mm", "Scan start to paper edge offset", true, "<img src=':/tips/Resources/tips/scan_start.png'/> Scan start to paper edge offset defines a distance from physical scanning point to the beginning of the sheet");
	_paramWidget->addControl( "LeftOffsetFromPaperEdgeX_mm", "Left offset from paper edge", true,"<img src=':/tips/Resources/tips/left_offset.png'/> Left offset from paper edge defines a distance from paper edge toward scan starting point");
	_paramWidget->addControl( "LeftStripWidth_mm", "Left strip width", true, "<img src=':/tips/Resources/tips/strip_width.png'/> Left strip width");
	_paramWidget->addSpacer(20,15);
	_paramWidget->addControl( "I2SOffsetFromPaperEdgeX_mm", "I2S offset from paper edge, X", true);
	_paramWidget->addControl( "I2SOffsetFromPaperEdgeY_mm", "I2S offset from paper edge, Y", true);
	_paramWidget->addSpacer(20,15);
	_paramWidget->addControl( "I2SROIMarginX_mm", "Triangle ROI margin X", true);
	_paramWidget->addControl( "I2SROIMarginY_mm", "Triangle ROI margin Y", true);
	_paramWidget->addSpacer(20,15);
	_paramWidget->addControl( "C2CROIMarginX_mm", "C2C ROI margin X", true);
	_paramWidget->addControl( "C2CROIMarginY_mm", "C2C ROI margin Y", true);
	_paramWidget->addSpacer(20,15);
	connect(
				_paramWidget->addCheckBox("C2C ROIs are lined up"), 
				&QCheckBox::stateChanged, 
				this, 
				[this]( int iState)
				{
					_fullImageBox->setC2CRoisLinedUp(iState == Qt::Checked);
				}
	);
	_paramWidget->addFinalSpacer();

	connect(_paramWidget, &roiParamWidget::propertyChanged, this, &fullImageTab::onPropertyChanged);
	connect(_paramWidget, &roiParamWidget::done, this, &fullImageTab::onEditDone);
}

void fullImageTab::setFullScreen(bool bSet)
{
	emit wantFullScreen (_bFullScreen);
}


void fullImageTab::onColorCountChanged ( const QString&  newVal )
{
	auto const iVal = newVal.toInt();
	if (iVal != 0)
	{
		QVector<LandaJune::Parameters::COLOR_TRIPLET> tempColorArray  (iVal);
		_params->setColorArray(tempColorArray);
		_params->updateValues();
		updateROIs(true);
	}
}

void fullImageTab::onUnitsChanged ( int oldUnits, int newUnits )
{
	_fullImageBox->updateUnits(oldUnits, newUnits);
}


void fullImageTab::onDoubleClick(QPoint pos)
{
	_bFullScreen = !_bFullScreen;
	setFullScreen(_bFullScreen);
}

void fullImageTab::onEditDone(bool bApply)
{
	if (bApply)
	{
		_params->setColorArray(_originalColorTriplets);
	}
	emit editDone(bApply);
}


void fullImageTab::onLeftStripEdgeChanged(const QPoint i2spt, const int edgeX)
{
	recalculateLeftStripEdgeOffset(i2spt, edgeX);
	updateROIs(false);
}

void fullImageTab::onRightStripEdgeChanged(const QPoint i2spt, const int edgeX)
{
	recalculateRightStripEdgeOffset(i2spt, edgeX);
	updateROIs(false);
}


void fullImageTab::onPageEdgeChanged(const QPoint i2spt, const int edgeX)
{
	recalculatePageEdgeOffset(i2spt, edgeX);
	updateROIs(false);
}


void fullImageTab::onI2SROIChanged(const QPoint i2spt)
{
	recalculateI2SOffset(i2spt);
	updateROIs(true);
}

void fullImageTab::onC2CROIsChanged( const QPoint i2spt, const QVector<QPoint>& c2cPts )
{
	recalculateC2COffsets(i2spt, c2cPts);
	updateROIs(false);
}


void fullImageTab::recalculateLeftStripEdgeOffset(const QPoint i2spt, int offset ) const
{
	//get old left strip width
	auto const newOffsetFromLeft = _params->ScanStartToPaperEdgeOffset_px() - offset;
	_params->setLeftOffsetFromPaperEdgeX_mm(toMMX(newOffsetFromLeft));
	_params->updateValues();
}

void fullImageTab::recalculateRightStripEdgeOffset(const QPoint i2spt, int offset ) const
{
	//get old left strip width
	auto const newWidth = offset  - _params->LeftStripRect_px().left();
	_params->setLeftStripWidth_mm(toMMX(newWidth));
	_params->updateValues();
}

void fullImageTab::recalculatePageEdgeOffset(const QPoint i2spt, int offset ) const
{
	_params->setScanStartToPaperEdgeOffset_mm(toMMX(offset));
	_params->setI2SOffsetFromPaperEdgeX_mm(toMMX(i2spt.x() - offset) );
	_params->updateValues();
}

void fullImageTab::recalculateI2SOffset(const QPoint& pt) const
{
	auto const scanStartToPaperEdge_mm = _params->ScanStartToPaperEdgeOffset_mm();

	QPointF mmPoint = { toMMX(pt.x()), toMMY(pt.y()) };
	const auto overallOffset = scanStartToPaperEdge_mm;// + offsetFromLeftEdge_mm;
	mmPoint.setX(mmPoint.x() - overallOffset);

	/*
	const auto oldI2SOffsetX = _params->I2SOffsetFromPaperEdgeX_mm();
	const auto oldI2SOffsetY = _params->I2SOffsetFromPaperEdgeY_mm();
	const auto newI2SOffsetX = mmPoint.x();
	const auto newI2SOffsetY = mmPoint.y();
	FULLTAB_SCOPED_LOG << "OLD I2S offset : [" << oldI2SOffsetX << "," << oldI2SOffsetY << "]";
	FULLTAB_SCOPED_LOG << "NEW I2S offset : [" << newI2SOffsetX << "," << newI2SOffsetY << "]";
	*/

	_params->setI2SOffsetFromPaperEdgeX_mm(mmPoint.x());
	_params->setI2SOffsetFromPaperEdgeY_mm(mmPoint.y());
	_params->updateValues();
}

void fullImageTab::recalculateC2COffsets(const QPoint& i2spt, const QVector<QPoint>& c2cPts)
{
	auto const newI2SCornerPt = i2spt;

	// calculate C2C offsets
	QVector<QSizeF> c2cOffsets_mm;
	for ( int i = 0; i < c2cPts.size(); i++)
	{
		c2cOffsets_mm << QSizeF (
								toMMX(c2cPts[i].x() - newI2SCornerPt.x()),
								toMMY(c2cPts[i].y() - newI2SCornerPt.y())
							);
	}
	
	_params->setC2COffsetsArray(c2cOffsets_mm);
	_params->updateValues();
}
