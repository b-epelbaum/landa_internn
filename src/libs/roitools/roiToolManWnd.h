#pragma once

#include "ui_roiToolManWnd.h"

class roiToolMainWnd : public QMainWindow
{
	Q_OBJECT

public:
	roiToolMainWnd( LandaJune::ProcessParametersPtr params, QWidget *parent = Q_NULLPTR);

private slots :

	void openFullImageTool();
	void openWaveTool();
	void openStripsTool();

	void onRegOfflineDone(bool bApply);

private:
	Ui::roiToolMainWindow ui{};

	offlineRegTab * _offlineTab		= nullptr;
	waveTab * _waveTab				= nullptr;
	fullImageTab * _fullImageTab	= nullptr;

	LandaJune::ProcessParametersPtr _parameters;
	roiParamWidget * _paramWidget = nullptr;;

	QAction * _dockAction = nullptr;
};
