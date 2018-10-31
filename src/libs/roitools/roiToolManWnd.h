#pragma once

#include "ui_roiToolManWnd.h"

class roiToolMainWnd : public QMainWindow
{
	Q_OBJECT

public:
	roiToolMainWnd( LandaJune::ProcessParametersPtr params, QWidget *parent = Q_NULLPTR);

private slots :

	void switchTool();

private:
	Ui::roiToolMainWindow ui{};

	QAction * offlineRegAct = nullptr;
	QAction * waveAct = nullptr;
	QAction * fullImageActAct = nullptr;

	offlineRegTab * _offlineTab		= nullptr;
	waveTab * _waveTab				= nullptr;
	fullImageTab * _fullImageTab	= nullptr;

	LandaJune::ProcessParametersPtr _parameters;

	roiParamWidget * _paramWidget = nullptr;;

};
