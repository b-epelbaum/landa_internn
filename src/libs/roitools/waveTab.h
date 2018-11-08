#pragma once

#include <QWidget>
#include "ui_waveTab.h"
#include "roiParamWidget.h"
#include "common/type_usings.h"
#include "baseparam.h"

class roiWidget;

class waveTab : public QWidget
{
	Q_OBJECT

public:
	waveTab(QWidget *parent = Q_NULLPTR);
	~waveTab();
	
	void setParameters (roiParamWidget* paramWidget, LandaJune::ProcessParametersPtr params );
	LandaJune::ProcessParametersPtr getEditedParameters () const { return std::move(_params); }

private slots:

	void onImageLoaded ( QString strPath, LandaJune::CORE_ERROR );
	void onPropertyChanged (QString propName, QVariant newVal );

	void onWaveTriangleChanged( QPoint newControlPoint );

	void onColorCountChanged ( const QString&  newVal );

	void onUnitsChanged ( int oldUnits, int newUnits );

	void onDoubleClick( QPoint pos );
	void onEditDone( bool bApply );


signals :

	void editDone( bool bApply );
	void wantFullScreen ( bool bFullScreen );

private:

	void buildControls();
	void setFullScreen ( bool bSet );
	void setupInitialROIs() const;
	void updateROIs() const;

	double toMMX(int val_pxx) const;
	double toMMY(int val_pxy) const;

	void recalculateWaveTriangleOffset(const QPoint& pt) const;

	Ui::waveTab ui;

	roiImageBox * _waveImageBox;
	roiParamWidget * _paramWidget;

	double _Pixel2MM_X = 0.0;
	double _Pixel2MM_Y = 0.0;

	LandaJune::ProcessParametersPtr _params;
	bool _bFullScreen = false;

	QVector<LandaJune::Parameters::COLOR_TRIPLET> _originalColorTriplets;
};
