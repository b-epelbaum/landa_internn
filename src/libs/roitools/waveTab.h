#pragma once

#include <QWidget>
#include "ui_waveTab.h"
#include "common/type_usings.h"

class roiWidget;
class roiParamWidget;


class waveTab : public QWidget
{
	Q_OBJECT

public:
	waveTab(QWidget *parent = Q_NULLPTR);
	~waveTab();
	
	void setParameters (roiParamWidget* paramWidget, LandaJune::ProcessParametersPtr params );

private slots:

	void onPropertyChanged (QString propName, QVariant newVal );

private:

	void buildControls();
	void recalculate ();

	Ui::waveTab ui;
	roiImageBox * _waveImageBox;
	roiParamWidget * _paramWidget;
	LandaJune::ProcessParametersUniquePtr _params;
};
