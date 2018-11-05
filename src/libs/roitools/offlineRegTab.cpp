#include "offlineRegTab.h"
#include "roiImageBox.h"
#include "roiParamWidget.h"

#include "commonTabs.h"
#include "unitSwitchLabel.h"
#include "applog.h"
#include "common/june_defs.h"
#include "ProcessParameters.h"

using namespace LandaJune;
using namespace Parameters;

#define OFFREGTAB_SCOPED_LOG PRINT_INFO6 << "[offlineRegTab] : "
#define OFFREGTAB_SCOPED_WARNING PRINT_WARNING << "[offlineRegTab] : "
#define OFFREGTAB_SCOPED_ERROR PRINT_ERROR << "[offlineRegTab] : "

double offlineRegTab::toMMX(const int val_pxx ) const { return val_pxx * _Pixel2MM_X;}
double offlineRegTab::toMMY(const int val_pxy ) const { return val_pxy * _Pixel2MM_Y;}


offlineRegTab::offlineRegTab(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	_leftImageBox = ui.leftROIWidget;
	_rightImageBox = ui.rightROIWidget;

	_leftImageBox->createWidget(roiImageBox::RENDER_STRIP);
	_rightImageBox->createWidget(roiImageBox::RENDER_STRIP);

	connect(_leftImageBox, &roiImageBox::imageLoaded, this, &offlineRegTab::onImageLoaded);
	connect(_rightImageBox, &roiImageBox::imageLoaded, this, &offlineRegTab::onImageLoaded);

	connect(_leftImageBox, &roiImageBox::roiChanged_strip_edge, this, &offlineRegTab::onEdgeChanged);
	connect(_leftImageBox, &roiImageBox::roiChanged_strip_i2s, this, &offlineRegTab::onI2SROIChanged);
	connect(_leftImageBox, &roiImageBox::roiChanged_strip_c2c, this, &offlineRegTab::onC2CROIsChanged);

	connect(_leftImageBox, &roiImageBox::scaleChanged, this, &offlineRegTab::onROIScaleChanged);
	connect(_rightImageBox, &roiImageBox::scaleChanged, this, &offlineRegTab::onROIScaleChanged);

	connect(_leftImageBox, &roiImageBox::scrollValuesChanged, this, &offlineRegTab::onROIScrollChanged);
	connect(_rightImageBox, &roiImageBox::scrollValuesChanged, this, &offlineRegTab::onROIScrollChanged);

	connect(_leftImageBox, &roiImageBox::unitsChanged, this, &offlineRegTab::onUnitsChanged);
	connect(_rightImageBox, &roiImageBox::unitsChanged, this, &offlineRegTab::onUnitsChanged);

	connect(_leftImageBox, &roiImageBox::doubleClick, this, &offlineRegTab::onDoubleClick);
	connect(_rightImageBox, &roiImageBox::doubleClick, this, &offlineRegTab::onDoubleClick);

	_leftImageBox->setFileMetaInfo("Load LEFT registration image file", ROITOOLS_KEY_LAST_REG_LEFT_FILE);
	_rightImageBox->setFileMetaInfo("Load RIGHT registration image file", ROITOOLS_KEY_LAST_REG_RIGHT_FILE);

	_leftImageBox->setAlias("LEFT REG");
	_rightImageBox->setAlias("RIGHT REG");
}

offlineRegTab::~offlineRegTab()
{
}

void offlineRegTab::setParameters(roiParamWidget* paramWidget, ProcessParametersPtr params)
{
	_paramWidget = paramWidget;
	_params.reset ( new ProcessParameters(*params));
	
	_Pixel2MM_X = _params->Pixel2MM_X();
	_Pixel2MM_Y = _params->Pixel2MM_Y();

	_originalColorTriplets = _params->ColorArray();

	buildControls();
	setupInitialROIs ();

	_leftImageBox->setpx2mmRatio(_Pixel2MM_X, _Pixel2MM_Y);
	_rightImageBox->setpx2mmRatio(_Pixel2MM_X, _Pixel2MM_Y);
}

void offlineRegTab::onImageLoaded(QString strPath, LandaJune::CORE_ERROR err )
{
	if (err == RESULT_OK )
	{
		_paramWidget->enableControls(true);
	}
	else
	{
		_paramWidget->enableControls(_leftImageBox->hasImage() || _rightImageBox->hasImage() );
	}
}

void offlineRegTab::onPropertyChanged(QString propName, QVariant newVal)
{
	_params->setParamProperty(propName, newVal);
	_params->updateValues();
	updateLeftROIs();
	updateRightROIs();
}


void offlineRegTab::buildControls()
{
	_paramWidget->clear();
	_paramWidget->setProcessParameters(_params);
	
	_colorCounterCombo = _paramWidget->addComboBox("Number of colors");
	_colorCounterCombo->addItem("4");
	_colorCounterCombo->addItem("7");
	_colorCounterCombo->setCurrentText(QString::number(_params->ColorArray().size()));
	connect(_colorCounterCombo, &QComboBox::currentTextChanged, this, &offlineRegTab::onColorCountChanged);
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
					_leftImageBox->setC2CRoisLinedUp(iState == Qt::Checked);
				}
	);
	_paramWidget->addFinalSpacer();

	connect(_paramWidget, &roiParamWidget::propertyChanged, this, &offlineRegTab::onPropertyChanged);
	connect(_paramWidget, &roiParamWidget::done, this, &offlineRegTab::onEditDone);
}

void offlineRegTab::setFullScreen(bool bSet)
{
	emit wantFullScreen (_bFullScreen);
}

void offlineRegTab::onColorCountChanged ( const QString&  newVal )
{
	auto const iVal = newVal.toInt();
	if (iVal != 0)
	{
		QVector<LandaJune::Parameters::COLOR_TRIPLET> tempColorArray  (iVal);
		_params->setColorArray(tempColorArray);
		_params->updateValues();
		updateLeftROIs();
		updateRightROIs();
	}
}

void offlineRegTab::onUnitsChanged ( int oldUnits, int newUnits )
{
	auto targetWidget = ( dynamic_cast<roiImageBox*>(sender()) == _leftImageBox ) ? _rightImageBox : _leftImageBox;
	targetWidget->updateUnits(oldUnits, newUnits);
}

void offlineRegTab::onDoubleClick(QPoint pos)
{
	_bFullScreen = !_bFullScreen;
	setFullScreen(_bFullScreen);
}

void offlineRegTab::onEditDone(bool bApply)
{
	if (bApply)
	{
		_params->setColorArray(_originalColorTriplets);
	}
	emit editDone(bApply);
}

void offlineRegTab::onROIScaleChanged(double glScale, double imageScale)
{
	auto targetWidget = ( dynamic_cast<roiImageBox*>(sender()) == _leftImageBox ) ? _rightImageBox : _leftImageBox;
	targetWidget->updateScaleFromExternal(glScale, imageScale);
}

void offlineRegTab::onROIScrollChanged(int hScroll, int vScroll)
{
	auto targetWidget = ( dynamic_cast<roiImageBox*>(sender()) == _leftImageBox ) ? _rightImageBox : _leftImageBox;
	targetWidget->updateScrollsFromExternal(hScroll, vScroll);
}

void offlineRegTab::onEdgeChanged(const QPoint i2spt, const int edgeX)
{
	recalculateEdgeOffset(i2spt, edgeX);
	updateRightROIs();
}

void offlineRegTab::onI2SROIChanged(const QPoint i2spt)
{
	recalculateI2SOffset(i2spt);
	updateRightROIs();
}

void offlineRegTab::onC2CROIsChanged( const QPoint i2spt, const QVector<QPoint>& c2cPts )
{
	recalculateC2COffsets(i2spt, c2cPts);
	updateRightROIs();
}


void offlineRegTab::updateLeftROIs()
{
	const auto i2sROILeft = _params->I2SRectLeft_px();
	const auto c2cROIsLeft = _params->C2CROIArrayLeft_px();
	const auto leftEdge = _params->LeftOffsetFromPaperEdgeX_px();

		_leftImageBox->updateROIs_Strip(i2sROILeft, c2cROIsLeft, leftEdge,
		{
			_params->I2SMarginX_px(),
			_params->I2SMarginY_px()
		},
		{
			_params->C2CMarginX_px(),
			_params->C2CMarginY_px()
		},
		_params->C2CCircleDiameter_px()
		, true
	);
}

void offlineRegTab::updateRightROIs()
{
	const auto i2sROIRight = _params->I2SRectRight_px();
	const auto c2cROIsRight = _params->C2CROIArrayRight_px();


	_rightImageBox->updateROIs_Strip(i2sROIRight, c2cROIsRight, 0,
	{
		_params->I2SMarginX_px(),
		_params->I2SMarginY_px()
	},
	{
		_params->C2CMarginX_px(),
		_params->C2CMarginY_px()
	},
	_params->C2CCircleDiameter_px()
	, false
	);

}

void offlineRegTab::setupLeftROIs()
{
	const auto i2sROILeft = _params->I2SRectLeft_px();
	const auto c2cROIsLeft = _params->C2CROIArrayLeft_px();
	const auto leftEdge = _params->LeftOffsetFromPaperEdgeX_px();

		_leftImageBox->setInitialROIs_Strip(i2sROILeft, c2cROIsLeft, leftEdge,
		{
			_params->I2SMarginX_px(),
			_params->I2SMarginY_px()
		},
		{
			_params->C2CMarginX_px(),
			_params->C2CMarginY_px()
		},
		_params->C2CCircleDiameter_px()
		, true
	);
}

void offlineRegTab::setupRightROIs()
{
	const auto i2sROIRight = _params->I2SRectRight_px();
	const auto c2cROIsRight = _params->C2CROIArrayRight_px();

	_rightImageBox->setInitialROIs_Strip(i2sROIRight, c2cROIsRight, 0,
	{
		_params->I2SMarginX_px(),
		_params->I2SMarginY_px()
	},
	{
		_params->C2CMarginX_px(),
		_params->C2CMarginY_px()
	},
	_params->C2CCircleDiameter_px()
	, false
	);
}

void offlineRegTab::setupInitialROIs() 
{
	setupLeftROIs();
	setupRightROIs();
}

void offlineRegTab::recalculateEdgeOffset(const QPoint i2spt, int offset ) const
{
	_params->setLeftOffsetFromPaperEdgeX_mm(toMMX(offset));
	_params->setI2SOffsetFromPaperEdgeX_mm(toMMX(i2spt.x() - offset) );
	_params->updateValues();
}

void offlineRegTab::recalculateI2SOffset(const QPoint& pt) const
{
	auto const offsetFromLeftEdge_px = _params->LeftOffsetFromPaperEdgeX_mm();
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
	_params->updateValues();
}

void offlineRegTab::recalculateC2COffsets(const QPoint& i2spt, const QVector<QPoint>& c2cPts)
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
