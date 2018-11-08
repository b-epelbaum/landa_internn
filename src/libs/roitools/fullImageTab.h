#pragma once

#include <QWidget>
#include "ui_fullImageTab.h"
#include "roiParamWidget.h"
#include "common/type_usings.h"
#include "baseparam.h"

class roiWidget;

class fullImageTab : public QWidget
{
	Q_OBJECT

public:
	fullImageTab(QWidget *parent = Q_NULLPTR);
	~fullImageTab();

	void setParameters (roiParamWidget* paramWidget, LandaJune::ProcessParametersPtr params );
	LandaJune::ProcessParametersPtr getEditedParameters () const { return std::move(_params); }

private slots:

	void onImageLoaded ( QString strPath, LandaJune::CORE_ERROR );
	void onPropertyChanged (QString propName, QVariant newVal );

	void onLeftStripEdgeChanged( const QPoint i2spt, const int edgeX );
	void onRightStripEdgeChanged( const QPoint i2spt, const int edgeX );
	void onPageEdgeChanged( const QPoint i2spt, const int edgeX );
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
	void setupInitialROIs() const;
	void updateROIs( bool bUpdateBoth ) const;

	double toMMX(int val_pxx) const;
	double toMMY(int val_pxy) const;

	void recalculateC2COffsets(const QPoint& i2spt, const QVector<QPoint>& c2cPts );
	void recalculateI2SOffset(const QPoint& pt ) const;
	void recalculateLeftStripEdgeOffset(const QPoint i2spt, int offset ) const;
	void recalculateRightStripEdgeOffset(const QPoint i2spt, int offset ) const;
	void recalculatePageEdgeOffset(const QPoint i2spt, int offset ) const;

	Ui::fullImageTab ui;

	roiImageBox *		_fullImageBox;
	roiParamWidget *	_paramWidget;

	double _Pixel2MM_X = 0.0;
	double _Pixel2MM_Y = 0.0;

	LandaJune::ProcessParametersPtr _params;
	bool _bFullScreen = false;

	QVector<LandaJune::Parameters::COLOR_TRIPLET> _originalColorTriplets;

};
