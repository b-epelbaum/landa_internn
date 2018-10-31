#include "roiToolManWnd.h"


roiToolMainWnd::roiToolMainWnd(LandaJune::ProcessParametersPtr params, QWidget *parent ) 
	: QMainWindow(parent)
{
	ui.setupUi(this);

	_parameters = params;
	_paramWidget = ui.paramWidget;

	offlineRegAct = new QAction(QIcon(":/roiTools/Resources/strips.png"), tr("&Offline Registration Tool"), this);
	waveAct = new QAction(QIcon(":/roiTools/Resources/wave.png"), tr("&Wave tool"), this);
	fullImageActAct = new QAction(QIcon(":/roiTools/Resources/full.png"), tr("&Full Image Tool"), this);
	
	offlineRegAct->setCheckable(true);
	waveAct->setCheckable(true);
	fullImageActAct->setCheckable(true);

	connect(offlineRegAct, &QAction::triggered, this, &roiToolMainWnd::switchTool);
	connect(waveAct, &QAction::triggered, this, &roiToolMainWnd::switchTool);
	connect(fullImageActAct, &QAction::triggered, this, &roiToolMainWnd::switchTool);

	auto toolGroup = new QActionGroup(this);
	toolGroup->setExclusive(true);

    toolGroup->addAction(offlineRegAct);
    toolGroup->addAction(waveAct);
    toolGroup->addAction(fullImageActAct);

	QList<QAction*> acList;
	acList << offlineRegAct << waveAct << fullImageActAct;
	ui.mainToolBar->addActions(acList);

	offlineRegAct->setChecked(true);

	_offlineTab = ui.offlineTool;
	_waveTab = ui.waveTool;
	_fullImageTab = ui.fullImageTool;

	_offlineTab->setParameters(_paramWidget, _parameters);
}

void roiToolMainWnd::switchTool()
{
	auto action = static_cast<QAction*>(sender());

	if ( action == offlineRegAct)
	{
		ui.imageStackWidget->setCurrentIndex(0);
		_offlineTab->setParameters(_paramWidget, _parameters);
	}
	else if ( action == waveAct)
	{
		ui.imageStackWidget->setCurrentIndex(1);
		_waveTab->setParameters(_paramWidget, _parameters);
	}
	else if ( action == fullImageActAct)
	{
		ui.imageStackWidget->setCurrentIndex(2);
		_fullImageTab->setParameters(_paramWidget, _parameters);
	}
}
