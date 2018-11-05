#pragma once

#include <QWidget>
#include "ui_offlineRegTab.h"
#include "common/type_usings.h"
#include "baseparam.h"

class roiWidget;
class roiParamWidget;

class offlineRegTab : public QWidget
{
	Q_OBJECT

public:
	offlineRegTab(QWidget *parent = Q_NULLPTR);
	~offlineRegTab();

	void setParameters (roiParamWidget* paramWidget, LandaJune::ProcessParametersPtr params );
	LandaJune::ProcessParametersPtr getEditedParameters () const { return std::move(_params); }

private slots:

	void onImageLoaded ( QString strPath, LandaJune::CORE_ERROR );
	void onPropertyChanged (QString propName, QVariant newVal );

	void onROIScaleChanged( double glScale, double imageScale );
	void onROIScrollChanged( int hScroll, int vScroll );

	void onEdgeChanged( const QPoint i2spt, const int edgeX );
	void onI2SROIChanged( const QPoint i2spt );
	void onC2CROIsChanged( const QPoint i2spt, const QVector<QPoint>& c2cPts );

	void onColorCountChanged ( const QString& );

	void onUnitsChanged ( int oldUnits, int newUnits );

	void onDoubleClick( QPoint pos );
	void onEditDone( bool bApply );

signals :

	void editDone( bool bApply );
	void wantFullScreen ( bool bFullScreen );

private:

	void buildControls();
	void setFullScreen ( bool bSet );
	void setupInitialROIs();

	void updateLeftROIs();
	void updateRightROIs();

	void setupLeftROIs();
	void setupRightROIs();

	double toMMX(int val_pxx) const;
	double toMMY(int val_pxy) const;

	void recalculateC2COffsets(const QPoint& i2spt, const QVector<QPoint>& c2cPts );
	void recalculateI2SOffset(const QPoint& pt ) const;
	void recalculateEdgeOffset(const QPoint i2spt, int offset ) const;

	Ui::offlineRegTab ui;

	roiImageBox * _leftImageBox;
	roiImageBox * _rightImageBox;
	roiParamWidget * _paramWidget;

	double _Pixel2MM_X = 0.0;
	double _Pixel2MM_Y = 0.0;

	QComboBox * _colorCounterCombo = nullptr;
	
	LandaJune::ProcessParametersPtr _params;
	bool _bFullScreen = false;

	QVector<LandaJune::Parameters::COLOR_TRIPLET> _originalColorTriplets;
};
