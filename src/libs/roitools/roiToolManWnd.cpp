#include "roiToolManWnd.h"
#include <QAction>


roiToolMainWnd::roiToolMainWnd(LandaJune::ProcessParametersPtr params, QWidget *parent ) 
	: QMainWindow(parent)
{
	ui.setupUi(this);

	_parameters = params;
	_paramWidget = ui.paramWidget;

	connect(ui.fullImageToolButt,	&QPushButton::clicked, this, &roiToolMainWnd::openFullImageTool);
	connect(ui.waveToolButt,		&QPushButton::clicked, this, &roiToolMainWnd::openWaveTool);
	connect(ui.stripsToolButt,		&QPushButton::clicked, this, &roiToolMainWnd::openStripsTool);

	_offlineTab = ui.offlineTool;
	_waveTab = ui.waveTool;
	_fullImageTab = ui.fullImageTool;

	ui.toolStacks->setCurrentIndex(0);
	ui.dock->hide();
	_dockAction =  ui.dock->toggleViewAction();
}

void roiToolMainWnd::openFullImageTool()
{
	_dockAction->toggle();
	ui.dock->show();
	ui.toolStacks->setCurrentIndex(1);
	_fullImageTab->setParameters(_paramWidget, _parameters);
}

void roiToolMainWnd::openWaveTool()
{
	_dockAction->toggle();
	ui.dock->show();
	ui.toolStacks->setCurrentIndex(2);
	_waveTab->setParameters(_paramWidget, _parameters);
}

void roiToolMainWnd::openStripsTool()
{
	_dockAction->toggle();
	ui.dock->show();
	ui.toolStacks->setCurrentIndex(3);
	_offlineTab->setParameters(_paramWidget, _parameters);
	connect(_offlineTab, &offlineRegTab::editDone, this, &roiToolMainWnd::onRegOfflineDone);
	connect(_offlineTab, &offlineRegTab::wantFullScreen, this, &roiToolMainWnd::onWantsFullScreen);
}

void roiToolMainWnd::onRegOfflineDone(bool bApply)
{
	*_parameters = *_offlineTab->getEditedParameters();
	close();
}

void roiToolMainWnd::onWantsFullScreen(bool bWantsFullScreen )
{
	if ( bWantsFullScreen )
	{
		showMaximized();
	}
	else
	{
		showNormal();
	}
	ui.dock->setFloating(bWantsFullScreen);
	
	//showFullScreen();
}