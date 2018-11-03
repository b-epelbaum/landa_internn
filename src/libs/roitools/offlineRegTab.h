#pragma once

#include <QWidget>
#include "ui_offlineRegTab.h"
#include "common/type_usings.h"

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

	void onROIChanged( const QVector<QPoint> ptArray );
	void onColorCountChanged ( const QString& );

signals :

	void editDone( bool bApply );

private:

	void buildControls();

	void setupInitialROIs();

	void updateLeftROIs();
	void updateRightROIs();

	void setupLeftROIs();
	void setupRightROIs();

	double toMMX(int val_pxx) const;
	double toMMY(int val_pxy) const;

	void recalculateOffsets(const QVector<QPoint>& pts, bool bLeft );
	void recalculateI2SOffset(const QPoint& pt, bool bLeft  );

	Ui::offlineRegTab ui;

	roiImageBox * _leftImageBox;
	roiImageBox * _rightImageBox;
	roiParamWidget * _paramWidget;

	double _Pixel2MM_X = 0.0;
	double _Pixel2MM_Y = 0.0;

	QComboBox * _colorCounterCombo = nullptr;
	
	LandaJune::ProcessParametersPtr _params;
};
