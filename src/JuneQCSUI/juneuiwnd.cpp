#include "juneuiwnd.h"
#include <Windows.h>

#include <QInputDialog>
#include <QFileDialog>
#include <QScrollBar>
#include <QTextStream>
#include <QTimer>
#include <QDateTime>
#include <QtGlobal>

#include "interfaces/ICore.h"
#include "interfaces/IFrameProvider.h"
#include "interfaces/IAlgorithmRunner.h"
#include "ProcessParameters.h"
#include "common/june_exceptions.h"
#include "RealTimeStats.h"
#include "applog.h"
#include <QJsonDocument>
#include <QSettings>
#include <QStandardPaths>
#include <QMessageBox>


using namespace LandaJune;
using namespace UI;
using namespace Core;
using namespace Loggers;
using namespace FrameProviders;
using namespace Parameters;

#define CLIENT_SCOPED_LOG PRINT_INFO8 << "[JuneUIWnd] : "
#define CLIENT_SCOPED_WARNING PRINT_WARNING << "[JuneUIWnd] : "
#define CLIENT_SCOPED_ERROR PRINT_ERROR << "[JuneUIWnd] : "

JuneUIWnd::JuneUIWnd( QString runMode, QString logLevel, bool savelogToFile, QString strConfig, QWidget *parent)
	: QMainWindow(parent)
	, _savelogToFile(savelogToFile)
	, _strProcessingConfig(strConfig)
{
	bool bLevelOk = false;
	_logLevel = AppLogger::stringToLevel(logLevel, bLevelOk);

	bool bModeOk = false;
	_runMode = stringToMode(runMode, bModeOk);

	if ( logLevel != LOG_LEVEL_NONE )
		Q_UNUSED(AppLogger::createLogger(_logLevel, savelogToFile));

	CLIENT_SCOPED_LOG << "---------------------------------------------------------";
	CLIENT_SCOPED_LOG << "				Started Landa June QCS Client";
	CLIENT_SCOPED_LOG << "---------------------------------------------------------";

	PRINT_DEBUG <<		"[Console colors : ]";
	PRINT_INFO <<		"\t--- INFO sample";
	PRINT_INFO1 <<		"\t--- INFO 1 sample";
	PRINT_INFO2 <<		"\t--- INFO 2 sample";
	PRINT_INFO3 <<		"\t--- INFO 3 sample";
	PRINT_INFO4 <<		"\t--- INFO 4 sample";
	PRINT_INFO5 <<		"\t--- INFO 5 sample";
	PRINT_INFO6 <<		"\t--- INFO 6 sample";
	PRINT_INFO7 <<		"\t--- INFO 7 sample";
	PRINT_INFO8 <<		"\t--- INFO 8 sample";
	PRINT_DEBUG <<		"\t--- DEBUG sample";
	PRINT_WARNING <<	"\t--- WARNING sample";
	PRINT_ERROR <<		"\t--- ERROR sample";
	PRINT_DEBUG_BREAK;

	if ( !bModeOk)
	{
		CLIENT_SCOPED_WARNING << "command line parameter \"mode\" has invalid value : " << runMode << "; defaulting to UI mode...";
	}

	init();
}


JuneUIWnd::JuneUIWnd( RUN_MODE runMode, LOG_LEVEL logLevel, bool savelogToFile, QString strConfig, QWidget *parent)
	: QMainWindow(parent)
	, _runMode(runMode)
	, _logLevel(logLevel)
	, _savelogToFile(savelogToFile)
	, _strProcessingConfig(strConfig)
{
	if ( logLevel != LOG_LEVEL_NONE )
		Q_UNUSED(AppLogger::createLogger(logLevel, savelogToFile));

	CLIENT_SCOPED_LOG << "---------------------------------------------------------";
	CLIENT_SCOPED_LOG << "				Started Landa June QCS Client";
	CLIENT_SCOPED_LOG << "---------------------------------------------------------";

	PRINT_DEBUG <<		"[Console colors : ]";
	PRINT_INFO <<		"\t--- INFO sample";
	PRINT_INFO1 <<		"\t--- INFO 1 sample";
	PRINT_INFO2 <<		"\t--- INFO 2 sample";
	PRINT_INFO3 <<		"\t--- INFO 3 sample";
	PRINT_INFO4 <<		"\t--- INFO 4 sample";
	PRINT_INFO5 <<		"\t--- INFO 5 sample";
	PRINT_INFO6 <<		"\t--- INFO 6 sample";
	PRINT_INFO7 <<		"\t--- INFO 7 sample";
	PRINT_INFO8 <<		"\t--- INFO 8 sample";
	PRINT_DEBUG <<		"\t--- DEBUG sample";
	PRINT_WARNING <<	"\t--- WARNING sample";
	PRINT_ERROR <<		"\t--- ERROR sample";
	PRINT_DEBUG_BREAK;

	init();
}

RUN_MODE JuneUIWnd::stringToMode(const QString& mode, bool& bOk)
{
	const auto strMode = mode.toLower();

	bOk = true;
	if (strMode == "ui")
	{
		return RUN_UI;
	}

	if (strMode == "noui" || strMode == "console" || strMode == "headless" )
	{
		return RUN_NO_UI;
	}

	if (strMode == "batch")
	{
		return RUN_BATCH;
	}

	bOk = false;
	return RUN_UI;
}


void JuneUIWnd::init()
{
	if (isUIMode() )
	{
		initUI();
		setWindowState(Qt::WindowMaximized);
	}

	initCore();
	if (isUIMode() )
	{
		enumerateFrameProviders();
		enumerateAlgoRunners();
	}


	initProcessParameters();

	if (isUIMode() )
	{
		connect(ui.frameSourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &JuneUIWnd::onFrameProviderComboChanged);
		connect(ui.algoHandlerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &JuneUIWnd::onAlgoRunnerComboChanged);
	}


	if (isBatchMode())
	{
		runAll();
	}
}

void JuneUIWnd::initUI()
{
	CLIENT_SCOPED_LOG << "Initializing UI...";
	_imageBox = new QLabel(this);
	ui.setupUi(this);

	_onRunViewer = ui.oneRunPlaceholder;

	_scrollArea = ui.scrollArea;
	_imageBox->setBackgroundRole(QPalette::Base);
	_imageBox->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	_imageBox->setScaledContents(true);

	_scrollArea->setBackgroundRole(QPalette::Dark);
	_scrollArea->setWidget(_imageBox);
	
	createActions();
	createStatusBar();

	auto const pTabBar = ui.tabWidget->tabBar();

	pTabBar->setFixedHeight(36);
	pTabBar->setIconSize(QSize(24,24));
	pTabBar->setTabIcon(0,  QIcon(":/JuneUIWnd/Resources/proc_params.png"));
	pTabBar->setTabIcon(1,  QIcon(":/JuneUIWnd/Resources/scan.png"));
	pTabBar->setTabIcon(2,  QIcon(":/JuneUIWnd/Resources/algo.png"));

	connect(qApp, &QCoreApplication::aboutToQuit, this, &JuneUIWnd::onAboutToQuit);
	
	_updateStatsTimer = new QTimer(this);
	connect(_updateStatsTimer, SIGNAL(timeout()), this, SLOT(updateStats()));

	_providerParamModel = std::make_unique<ParamPropModel>(this);
	ui.providerPropView->setModel(_providerParamModel.get());
	connect(_providerParamModel.get(), &ParamPropModel::propChanged, this, &JuneUIWnd::onProviderPropChanged);


	_processParamModelEditable = std::make_unique<ParamPropModel>(this);
	ui.batchParamView->setModel(_processParamModelEditable.get());
	connect(_processParamModelEditable.get(), &ParamPropModel::propChanged, this, &JuneUIWnd::onProcessParameterChanged);

	_processParamModelCalculated = std::make_unique<ParamPropModel>(this);
	ui.processParamViewCalculated->setModel(_processParamModelCalculated.get());

	connect(ui.batchParamView->selectionModel(), SIGNAL(currentChanged(const  QModelIndex&,const  QModelIndex&)), this, SLOT(processParamSelectionChanged(const  QModelIndex&,const  QModelIndex&)));

	_progressBarTimer.setSingleShot(false);
	_progressBarTimer.setInterval(150);

	connect(&_progressBarTimer, &QTimer::timeout, this, &JuneUIWnd::onTimerTick);
}

void JuneUIWnd::initCore()
{
	CLIENT_SCOPED_LOG << "Initializing core engine...";
	auto core = ICore::get();

	connect (core.get()->getClassObject(), SIGNAL(coreStopped()), this, SLOT(onCoreStopped()) );
	connect (core.get()->getClassObject(), SIGNAL(coreException(const LandaJune::BaseException&)), this, SLOT(onCoreException(const LandaJune::BaseException&)) );
	connect (core.get()->getClassObject(), SIGNAL(frameData(std::shared_ptr<LandaJune::Core::SharedFrameData>)), this, SLOT(onSharedFrameData(std::shared_ptr<LandaJune::Core::SharedFrameData>)) );
	try
	{
		core->init();
	}
	catch (BaseException& e)
	{
		handleException(e);
	}

	_onRunViewer->setTargetFolder(QString::fromStdString(core->getRootFolderForOneRun()));
	CLIENT_SCOPED_LOG << "Core engine initialized";
}

void JuneUIWnd::onCoreStopped()
{
	if (isUIMode() )
	{
		Helpers::RealTimeStats::rtStats()->reset();
		_updateStatsTimer->stop();
		enableUIForProcessing(true);
	}
	CLIENT_SCOPED_LOG << " ----------- processing stopped --------------";
	_bRunning = false;

	if (isBatchMode())
	{
		CLIENT_SCOPED_LOG << "--------------------------------------------------";
		CLIENT_SCOPED_LOG << "Processing statistics :";
		CLIENT_SCOPED_LOG << Helpers::RealTimeStats::rtStats()->to_string(true).c_str();
		CLIENT_SCOPED_LOG << "--------------------------------------------------";
		qApp->quit();
	}
}

void JuneUIWnd::onCoreException(const BaseException& ex)
{
}

void JuneUIWnd::onSharedFrameData(std::shared_ptr<LandaJune::Core::SharedFrameData> fData)
{
	

}

void JuneUIWnd::enumerateFrameProviders() 
{
	// UI code
	CLIENT_SCOPED_LOG << "Enumerating frame providers...";
	ui.frameSourceCombo->clear();
	try
	{
		auto listOfProviders = ICore::get()->getFrameProviderList();
		for (auto& provider : listOfProviders)
		{
			CLIENT_SCOPED_LOG << "\tadding provider ==> " << provider->getName();
			ui.frameSourceCombo->addItem(provider->getName(), QVariant::fromValue(provider));
		}
		listOfProviders.empty() ? ui.frameSourceCombo->setCurrentIndex(-1) : ui.frameSourceCombo->setCurrentIndex(0);

		// look for provider in configuration settings 		
		auto frameProvider = ICore::get()->getProcessParameters()->getParamProperty("FrameProviderName").toString();
		if ( frameProvider.isEmpty() )
		{
			CLIENT_SCOPED_LOG << "Configuration frame provider has not been found, setting default value...";
		}
		else
		{
			CLIENT_SCOPED_LOG << "Configuration frame provider ==> " <<  frameProvider;
			if ( ui.frameSourceCombo->findText(frameProvider) != - 1 )
				ui.frameSourceCombo->setCurrentText(frameProvider);
		}

		updateFrameProviderParamsView(ui.frameSourceCombo->currentIndex());
	}
	catch ( BaseException& ex)
	{
		handleException(ex);
	}

	CLIENT_SCOPED_LOG << "Finished frame providers enumeration";

}

void JuneUIWnd::enumerateAlgoRunners()
{
	// UI code
	CLIENT_SCOPED_LOG << "Enumerating algorithm runners...";
	ui.algoHandlerCombo->clear();
	try
	{
		auto listOfAlgo = ICore::get()->getAlgorithmRunnerList();
		for (auto& algo : listOfAlgo)
		{
			CLIENT_SCOPED_LOG << "\tadding algorith runner ==> " << algo->getName();
			ui.algoHandlerCombo->addItem(algo->getName(), QVariant::fromValue(algo));
		}
		listOfAlgo.empty() ? ui.algoHandlerCombo->setCurrentIndex(-1) : ui.algoHandlerCombo->setCurrentIndex(0);

		// look for algorithm runner in configuration settings 		
		auto algoRunner = ICore::get()->getProcessParameters()->getParamProperty("AlgorithmRunner").toString();

		if ( algoRunner.isEmpty() )
		{
			CLIENT_SCOPED_LOG << "Configuration algorithm runner has not been found, setting default value...";
		}
		else
		{
			CLIENT_SCOPED_LOG << "Configuration algorithm runner ==> " <<  algoRunner;
			if ( ui.algoHandlerCombo->findText(algoRunner) != - 1 )
				ui.algoHandlerCombo->setCurrentText(algoRunner);
		}
		updateAlgoRunnerParamsView(ui.algoHandlerCombo->currentIndex());
	}
	catch (BaseException & ex)
	{
		handleException(ex);
	}
	CLIENT_SCOPED_LOG << "Finished algorithm runner enumeration";
}


void JuneUIWnd::initProcessParametersUIMode()
{
	try
	{
		const auto batchParams = ICore::get()->getProcessParameters();

		connect(batchParams.get(), &BaseParameters::loaded, this, &JuneUIWnd::onUpdateProcessParams);
		connect(batchParams.get(), &BaseParameters::updateCalculated, this, &JuneUIWnd::onUpdateCalculatedParams);

		_processParamModelEditable->setupModelData(batchParams->getEditablePropertyList(), false);
		_processParamModelCalculated->setupModelData(batchParams->getReadOnlyPropertyList(), true);

		ui.batchParamView->header()->resizeSection(0, 280);
		ui.providerPropView->header()->resizeSection(0, 280);
		ui.processParamViewCalculated->header()->resizeSection(0, 280);

		ui.batchParamView->expandToDepth(1);
		ui.processParamViewCalculated->expandToDepth(1);

		QString sourceConfig;
		if ( !_strProcessingConfig.isEmpty())
		{
			CLIENT_SCOPED_LOG << "Command line config file is not empty => " << _strProcessingConfig << "; trying to load...";
			sourceConfig = _strProcessingConfig;
		}
		else
		{
			CLIENT_SCOPED_LOG << "Command line config file is empty, looking for saved process parameters configuration file...";
			QSettings settings("Landa Corp", "June QCS");
			auto lastConfigFile = settings.value("UIClient/lastConfigFile").toString();
			if ( !lastConfigFile.isEmpty())
				sourceConfig = lastConfigFile;
		}

		if ( !sourceConfig.isEmpty())
		{
			CLIENT_SCOPED_LOG << "Loading configuration from file : " << sourceConfig;
			QString strError;
			auto const bRes = ICore::get()->getProcessParameters()->load(sourceConfig, strError);
			if ( !bRes)
			{
				CLIENT_SCOPED_ERROR << "Cannot load configuration file " << sourceConfig <<"; error : " << strError;
			}
			else
			{
				statusRecipeName->setText(QFileInfo(sourceConfig).baseName());
				CLIENT_SCOPED_LOG << "Configuration file " << sourceConfig << " has been loaded successfully";
			}
		}
		else
		{
			CLIENT_SCOPED_LOG << "No configuration file to load from. Setting default values...";
		}

	}
	catch (BaseException& ex)
	{
		handleException(ex);
	}
}

void JuneUIWnd::initProcessParametersBatchMode()
{
	if (_strProcessingConfig.isEmpty())
	{
		CLIENT_SCOPED_LOG << "Supplied configuration file path is empty. Exiting...";
		qApp->quit();
		return;
	}
	try
	{
		QString strError;
		auto const bRes = ICore::get()->getProcessParameters()->load(_strProcessingConfig, strError);
		if ( !bRes)
		{
			CLIENT_SCOPED_ERROR << "Cannot load configuration file " << _strProcessingConfig <<"; error : " << strError;
			qApp->quit();
		}
		else
		{
			CLIENT_SCOPED_LOG << "Configuration file " << _strProcessingConfig << " has been loaded successfully";
		}

		const auto reqFrameProviderName = ICore::get()->getProcessParameters()->getParamProperty("FrameProviderName").toString();
		const auto reqAlgoRunnerName = ICore::get()->getProcessParameters()->getParamProperty("AlgorithmRunner").toString();

		CLIENT_SCOPED_LOG << "Loading requested frame provider ==> " << reqFrameProviderName;
		auto listOfProviders = ICore::get()->getFrameProviderList(); bool provFound = false;
		for (auto& provider : listOfProviders)
		{
			CLIENT_SCOPED_LOG << "\t\tFound provider : " << provider->getName();
			if ( provider->getName() == reqFrameProviderName )
			{
				CLIENT_SCOPED_LOG << "Requested frame provider found. Selected provider : " << reqFrameProviderName;
				ICore::get()->selectFrameProvider(provider);
				provFound = true;
			}
		}

		if (!provFound)
		{
			CLIENT_SCOPED_ERROR << "Requested frame provider " << reqFrameProviderName << " not found. Exiting...";
			qApp->quit();
		}

		CLIENT_SCOPED_LOG << "Loading requested algorithm runner ==> " << reqAlgoRunnerName;
		auto listOfRunners = ICore::get()->getAlgorithmRunnerList(); bool runnerFound = false;
		for (auto& runner : listOfRunners)
		{
			CLIENT_SCOPED_LOG << "\t\tFound runner : " << runner->getName();
			if ( runner->getName() == reqAlgoRunnerName )
			{
				CLIENT_SCOPED_LOG << "Requested algorithm runner found. Selected runner : " << reqAlgoRunnerName;
				ICore::get()->selectAlgorithmRunner(runner);
				runnerFound = true;
			}
		}

		if (!runnerFound)
		{
			CLIENT_SCOPED_ERROR << "Requested algorithm runner " << reqAlgoRunnerName << " not found. Exiting...";
			qApp->quit();
		}
	}
	catch (BaseException& ex)
	{
		handleException(ex);
	}
}

void JuneUIWnd::initProcessParameters()
{
	if (isUIMode() )
	{
		CLIENT_SCOPED_LOG << "Setting process parameters in UI mode...";
		initProcessParametersUIMode();
		return;
	}
	
	if (isBatchMode())
	{
		CLIENT_SCOPED_LOG << "Setting process parameters in BATCH mode...";
		initProcessParametersBatchMode();
	}
}

void JuneUIWnd::runAll()
{
	run (true);
}

void JuneUIWnd::runOnce()
{
	run (false);
}


void JuneUIWnd::runUIMode( bool bAll )
{
	if (ui.frameSourceCombo->currentIndex() == -1)
	{
		CLIENT_SCOPED_WARNING << "[JuneUIWnd] : no valid frame provider selected. Aborted ";
		return;
	}

	if (ui.algoHandlerCombo->currentIndex() == -1)
	{
		CLIENT_SCOPED_WARNING << "[JuneUIWnd] : no valid algorithm runner selected. Aborted ";
		return;
	}

	try
	{
		const auto selectedProvider = ui.frameSourceCombo->currentData().value<FrameProviderPtr>();
		const auto selectedAlgoRunner = ui.algoHandlerCombo->currentData().value<AlgorithmRunnerPtr>();
		
		if (selectedProvider && selectedAlgoRunner)
		{
			PRINT_DEBUG_DBLINE;
			CLIENT_SCOPED_LOG << "[JuneUIWnd] : Selected frame provider : " << selectedProvider->getName();
			CLIENT_SCOPED_LOG << "[JuneUIWnd] : Selected algorithm runner : " << selectedAlgoRunner->getName();
			CLIENT_SCOPED_LOG << "starting processing...";
			PRINT_DEBUG_DBLINE;

			ICore::get()->selectAlgorithmRunner(selectedAlgoRunner);
			ICore::get()->selectFrameProvider(selectedProvider);
			if ( bAll )
				ICore::get()->runAll();
			else
				ICore::get()->runOne();
		}
	}
	catch (BaseException& ex)
	{
		handleException(ex);
		return;
	}

	enableUIForProcessing(false);
	_bRunning = true;
	_updateStatsTimer->start(100);
}

void JuneUIWnd::runBatchMode( bool bAll )
{
	try
	{
		PRINT_DEBUG_DBLINE;
		CLIENT_SCOPED_LOG << "Selected frame provider : " << ICore::get()->getSelectedFrameProvider()->getName();
		CLIENT_SCOPED_LOG << "Selected algorithm runner : " << ICore::get()->getSelectedAlgorithmRunner()->getName();
		CLIENT_SCOPED_LOG << "starting processing...";
		PRINT_DEBUG_DBLINE;

		ICore::get()->runAll();
	}
	catch (BaseException& ex)
	{
		handleException(ex);
	}
}

void JuneUIWnd::run( bool bAll )
{
	if (isUIMode())
	{
		runUIMode(bAll);
		return;
	}

	if ( isBatchMode())
	{
		runBatchMode(bAll);
		return;
	}
}

void JuneUIWnd::stop()
{
	CLIENT_SCOPED_LOG << "stopping processing...";
	
	try
	{
		ICore::get()->stop();
	}
	catch (BaseException& ex)
	{
		handleException(ex);
	}


	Helpers::RealTimeStats::rtStats()->reset();
	_updateStatsTimer->stop();
	enableUIForProcessing(true);
	_bRunning = false;

	CLIENT_SCOPED_LOG << " ----------- processing stopped --------------";
}


void JuneUIWnd::handleException (BaseException& ex)
{
	std::ostringstream ss;
	print_exception(ex, ss);
	CLIENT_SCOPED_ERROR << ss.str().c_str();

	if ( isBatchMode() )
	{
		qApp->quit();
	}
}

void JuneUIWnd::createActions()
{
	auto fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addSeparator();

	auto exitAct = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
	exitAct->setShortcut(tr("Ctrl+Q"));

	auto editMenu = menuBar()->addMenu(tr("&Edit"));
	auto viewMenu = menuBar()->addMenu(tr("&View"));

	zoomInAct = viewMenu->addAction(tr("Zoom &In (25%)"), this, &JuneUIWnd::zoomIn);
	zoomInAct->setShortcut(QKeySequence::ZoomIn);
	zoomInAct->setEnabled(false);

	zoomOutAct = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &JuneUIWnd::zoomOut);
	zoomOutAct->setShortcut(QKeySequence::ZoomOut);
	zoomOutAct->setEnabled(false);

	normalSizeAct = viewMenu->addAction(tr("&Normal Size"), this, &JuneUIWnd::normalSize);
	normalSizeAct->setShortcut(tr("Ctrl+S"));
	normalSizeAct->setEnabled(false);

	viewMenu->addSeparator();

	fitToWindowAct = viewMenu->addAction(tr("&Fit to Window"), this, &JuneUIWnd::fitToWindow);
	fitToWindowAct->setEnabled(false);
	fitToWindowAct->setCheckable(true);
	fitToWindowAct->setShortcut(tr("Ctrl+F"));

	QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

	startAct = new QAction(QIcon(":/JuneUIWnd/Resources/start.png"), tr("&Start processing"), this);
	startOnceAct = new QAction(QIcon(":/JuneUIWnd/Resources/start_once.png"), tr("&Start Single processing"), this);
	stopAct = new QAction(QIcon(":/JuneUIWnd/Resources/stop.png"), tr("&Stop processing"), this);

	loadConfig = new QAction(QIcon(":/JuneUIWnd/Resources/file_open.png"), tr("&Load"), this);
	saveConfig = new QAction(QIcon(":/JuneUIWnd/Resources/file_save.png"), tr("&Save"), this);

	connect(saveConfig, &QAction::triggered, this, &JuneUIWnd::onSaveConfig);
	connect(loadConfig, &QAction::triggered, this, &JuneUIWnd::onLoadConfig);

	ui.saveConfigButt->setDefaultAction(saveConfig);
	ui.loadConfigButt->setDefaultAction(loadConfig);

	stopAct->setEnabled(false);

	connect(startAct, &QAction::triggered, this, &JuneUIWnd::runAll);
	connect(startOnceAct, &QAction::triggered, this, &JuneUIWnd::runOnce);
	connect(stopAct, &QAction::triggered, this, &JuneUIWnd::stop);


	addColor = new QAction(QIcon(":/JuneUIWnd/Resources/color_add.png"), tr("&Add Color"), this);
	removeColor = new QAction(QIcon(":/JuneUIWnd/Resources/color_remove.png"), tr("&Remove Color"), this);

	connect(addColor, &QAction::triggered, this, &JuneUIWnd::onAddColor);
	connect(removeColor, &QAction::triggered, this, &JuneUIWnd::onRemoveColor);

	ui.buttAddParam->setDefaultAction(addColor);
	ui.buttRemoveParam->setDefaultAction(removeColor);

	removeColor->setEnabled(false);
}

void JuneUIWnd::createStatusBar()
{
	_greyLed.load(":/JuneUIWnd/Resources/grey_led.png");
	_greenLed.load(":/JuneUIWnd/Resources/green_led.png");

	ui.mainToolBar->addAction(startOnceAct);
	ui.mainToolBar->addSeparator();
	ui.mainToolBar->addAction(startAct);
	ui.mainToolBar->addAction(stopAct);
	ui.mainToolBar->setIconSize(QSize(48, 48));


	iconGeneral = new QLabel(this);
	iconRecipe = new QLabel(this);
	iconProvider = new QLabel(this);
	iconAlgoRunner = new QLabel(this);
	statusFramesHandled = new QLabel(this);

	iconGeneral->setMinimumSize(QSize(16,16));
	iconGeneral->setMaximumSize(QSize(16,16));
	iconGeneral->setPixmap(_greyLed);

	iconRecipe->setMinimumSize(QSize(16,16));
	iconRecipe->setMaximumSize(QSize(16,16));
	iconRecipe->setPixmap(QPixmap(":/JuneUIWnd/Resources/proc_params.png"));
	iconRecipe->setScaledContents(true);

	iconProvider->setMinimumSize(QSize(16,16));
	iconProvider->setMaximumSize(QSize(16,16));
	iconProvider->setPixmap(QPixmap(":/JuneUIWnd/Resources/scan.png"));
	iconProvider->setScaledContents(true);

	iconAlgoRunner->setMinimumSize(QSize(16,16));
	iconAlgoRunner->setMaximumSize(QSize(16,16));
	iconAlgoRunner->setPixmap(QPixmap(":/JuneUIWnd/Resources/algo.png"));
	iconAlgoRunner->setScaledContents(true);

	statusFramesHandled->setMinimumSize(QSize(16,16));
	statusFramesHandled->setMaximumSize(QSize(16,16));

	
	statusGeneral = new QLabel(this);
	statusRecipeName = new QLabel(this);
	statusFrameProv = new QLabel(this);
	statusAlgoRunner = new QLabel(this);
	statusFramesHandled = new QLabel(this);


	statusGeneral->setMinimumWidth(200);
	statusGeneral->setText("Idle");
	
	statusProgressBar = new QProgressBar(this);
	statusProgressBar->setMaximumWidth(200);

	statusProgressBar->setTextVisible(false);

    // add the two controls to the status bar
	ui.statusBar->addPermanentWidget(iconGeneral,0);
    ui.statusBar->addPermanentWidget(statusGeneral,1);

	ui.statusBar->addPermanentWidget(iconRecipe,2);
    ui.statusBar->addPermanentWidget(statusRecipeName,3);

	ui.statusBar->addPermanentWidget(iconProvider,4);
	ui.statusBar->addPermanentWidget(statusFrameProv,5);

	ui.statusBar->addPermanentWidget(iconAlgoRunner,6);
	ui.statusBar->addPermanentWidget(statusAlgoRunner,7);
	ui.statusBar->addPermanentWidget(statusProgressBar,8);
}

void JuneUIWnd::zoomIn()
{
	/*
	scaleImage(1.25);
	*/
}

void JuneUIWnd::zoomOut()
{
	/*
	scaleImage(0.8);
	*/
}

void JuneUIWnd::normalSize()
{
	_imageBox->adjustSize();
	_scaleFactor = 1.0;
}

void JuneUIWnd::fitToWindow()
{
	const auto fitToWindow = fitToWindowAct->isChecked();
	_scrollArea->setWidgetResizable(fitToWindow);
	if (!fitToWindow)
		normalSize();
	updateActions();

}

void JuneUIWnd::adjustScrollBar(QScrollBar* scrollBar, double factor)
{
	scrollBar->setValue(int(factor * scrollBar->value()
		+ ((factor - 1) * scrollBar->pageStep() / 2)));

}

void JuneUIWnd::scaleImage(double factor)
{
	Q_ASSERT(_imageBox->pixmap());
	_scaleFactor *= factor;
	_imageBox->resize(_scaleFactor * _imageBox->pixmap()->size());

	adjustScrollBar(_scrollArea->horizontalScrollBar(), factor);
	adjustScrollBar(_scrollArea->verticalScrollBar(), factor);

	zoomInAct->setEnabled(_scaleFactor < 3.0);
	zoomOutAct->setEnabled(_scaleFactor > 0.333);
}

void JuneUIWnd::updateActions() const
{
	zoomInAct->setEnabled(!fitToWindowAct->isChecked());
	zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
	normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}

void JuneUIWnd::onAboutToQuit()
{
	if (_bRunning)
	{
		stop();
	}

	CLIENT_SCOPED_LOG << "---------------------------------------------------------";
	CLIENT_SCOPED_LOG << "				Finished Landa June QCS Client";
	CLIENT_SCOPED_LOG << "---------------------------------------------------------";
	AppLogger::closeLogger();
}

void JuneUIWnd::updateFrameProviderParamsView (int index)
{
	const auto selectedProvider = ui.frameSourceCombo->itemData(index).value<FrameProviderPtr>();
	if (!selectedProvider)
	{
		CLIENT_SCOPED_ERROR << "[JuneUIWnd] : selected frame provider is invalid. Aborted ";
		return;
	}

	ICore::get()->selectFrameProvider(selectedProvider);
	ui.providerDescEdit->document()->setPlainText(selectedProvider->getDescription());
	_providerParamModel->setupModelData(selectedProvider->getProviderProperties(), false);
	statusFrameProv->setText(selectedProvider->getName());
}

void JuneUIWnd::updateAlgoRunnerParamsView (int index)
{
	const auto selectedAlgo = ui.algoHandlerCombo->itemData(index).value<AlgorithmRunnerPtr>();
	if (!selectedAlgo)
	{
		CLIENT_SCOPED_ERROR << "[JuneUIWnd] : selected algorithm runner is invalid. Aborted ";
		return;
	}
	ui.algoDescEdit->document()->setPlainText(selectedAlgo->getDescription());
	statusAlgoRunner->setText(selectedAlgo->getName());
}

void JuneUIWnd::onFrameProviderComboChanged(int index)
{
	const auto selectedProvider = ui.frameSourceCombo->itemData(index).value<FrameProviderPtr>();
	if (selectedProvider)
		ICore::get()->getProcessParameters()->setParamProperty("FrameProviderName", selectedProvider->getName());

	updateFrameProviderParamsView (index);
}

void JuneUIWnd::onAlgoRunnerComboChanged(int index)
{
	const auto selectedAlgo = ui.algoHandlerCombo->itemData(index).value<AlgorithmRunnerPtr>();
	if (selectedAlgo)
	{
		ICore::get()->getProcessParameters()->setParamProperty("AlgorithmRunner", selectedAlgo->getName());
	}
	updateAlgoRunnerParamsView(index);
}

void JuneUIWnd::updateStats() const
{
	ui.statView->clear(); // unless you know the editor is empty
	ui.statView->appendPlainText(QString::fromStdString(Helpers::RealTimeStats::rtStats()->to_string()));
}

void JuneUIWnd::onProviderPropChanged(QString propName, const QVariant& newVal)
{
	const auto processParams = ICore::get()->getProcessParameters();
	processParams->setParamProperty(propName, newVal);
	onUpdateProcessParams();
}

void JuneUIWnd::onProcessParameterChanged(QString propName, const QVariant& newVal)
{
	try
	{
		const auto processParams = ICore::get()->getProcessParameters();
		processParams->setParamProperty(propName, newVal);
	}
	catch (BaseException& ex)
	{
		handleException (ex);
	}

}

void JuneUIWnd::saveExpandedOnLevel(const QModelIndex& index, QSet<int> & nodes, QTreeView * view, int& iLevel ) const
{
	iLevel++;
    if(view->isExpanded(index)) 
	{
        if(index.isValid())
            nodes.insert(iLevel * 100 + index.row());
        for(int row = 0; row < view->model()->rowCount(index); ++row)
            saveExpandedOnLevel(index.child(row,0), nodes, view, iLevel );
    }
	iLevel--;
}

void JuneUIWnd::restoreExpandedOnLevel(const QModelIndex& index, QSet<int> & nodes, QTreeView * view, int& iLevel) const
{
	iLevel++;
    if(nodes.contains(iLevel * 100 + index.row())) 
	{
        view->setExpanded(index, true);
        for(int row = 0; row < view->model()->rowCount(index); ++row)
            restoreExpandedOnLevel(index.child(row,0), nodes, view, iLevel);
    }
	iLevel--;
}

void JuneUIWnd::saveExpandedState(QSet<int>& nodes, QTreeView * view) const
{
	int iLevel = 0;
	for(int row = 0; row < view->model()->rowCount(); ++row)
        saveExpandedOnLevel(view->model()->index(row,0), nodes, view, iLevel);
}

void JuneUIWnd::restoreExpandedState(QSet<int>& nodes, QTreeView * view) const
{
    view->setUpdatesEnabled(false);
	int iLevel = 0;
    for(int row = 0; row < view->model()->rowCount(); ++row)
        restoreExpandedOnLevel(view->model()->index(row,0), nodes, view, iLevel);

    view->setUpdatesEnabled(true);
}

void JuneUIWnd::onUpdateProcessParams()
{
	QSet<int> nodes;
	saveExpandedState(nodes, ui.batchParamView );
	QModelIndex idx = ui.batchParamView->selectionModel()->currentIndex();

	_processParamModelEditable->setupModelData(ICore::get()->getProcessParameters()->getEditablePropertyList(), false);

	restoreExpandedState(nodes, ui.batchParamView);

	/*
	if ( idx.isValid() && idx.row() < ui.batchParamView->model()->rowCount())
	{
		ui.batchParamView->scrollTo(idx);
		ui.batchParamView->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Select);
	}
	*/
}

void JuneUIWnd::onUpdateCalculatedParams()
{
	QSet<int> nodes;
	saveExpandedState(nodes, ui.processParamViewCalculated );
	QModelIndex idx = ui.processParamViewCalculated->selectionModel()->currentIndex();

	_processParamModelCalculated->setupModelData(ICore::get()->getProcessParameters()->getReadOnlyPropertyList(), true);

	restoreExpandedState(nodes, ui.processParamViewCalculated);
	updateFrameProviderParamsView(ui.frameSourceCombo->currentIndex());
	updateAlgoRunnerParamsView(ui.algoHandlerCombo->currentIndex());
}

void JuneUIWnd::onSaveConfig()
{
	QSettings settings("Landa Corp", "June QCS");
	auto lastConfigFile = settings.value("UIClient/lastConfigFile").toString();

	if ( lastConfigFile.isEmpty() )
	{
		auto docRoot = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0];
		docRoot += "/Landa Corp/QCS Configuration";
		lastConfigFile = docRoot;
		(void)QDir().mkpath(lastConfigFile);
		lastConfigFile += "/default.jconfig";
	}


	auto fileName = QFileDialog::getSaveFileName(this,
    tr("Save configuration file as..."), lastConfigFile, tr("QCS Config Files (*.qconfig)"));
						
	if (fileName.isEmpty())
		return;

	settings.setValue("UIClient/lastConfigFile", fileName);
	statusRecipeName->setText(fileName);
	
	auto const jObj = ICore::get()->getProcessParameters()->toJson();
	QFile jsonFile(fileName);
    jsonFile.open(QFile::WriteOnly);

	QJsonDocument doc(jObj);
    jsonFile.write(doc.toJson(QJsonDocument::Indented));
}

void JuneUIWnd::onLoadConfig()
{
	CLIENT_SCOPED_LOG << "Loading configuration file...";
	QSettings settings("Landa Corp", "June QCS");
	auto lastConfigFile = settings.value("UIClient/lastConfigFile").toString();

	if ( lastConfigFile.isEmpty() )
	{
		auto docRoot = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0];
		docRoot += "/Landa Corp/QCS Configuration";
		lastConfigFile = docRoot;
		(void)QDir().mkpath(lastConfigFile);
	}
	else
	{
		CLIENT_SCOPED_LOG << "Last configuration file found : " << lastConfigFile;
	}


	auto fileName = QFileDialog::getOpenFileName(this,
    tr("Load configuration file"), lastConfigFile, tr("QCS Config Files (*.qconfig)"));

	if (fileName.isEmpty())
	{
		CLIENT_SCOPED_LOG << "Loading configuration file cancelled";
		return;
	}

	CLIENT_SCOPED_LOG << "Loading configuration file  : " << fileName;
	settings.setValue("UIClient/lastConfigFile", fileName);
	QString strError;
	auto const bRes = ICore::get()->getProcessParameters()->load(fileName, strError);

	if ( !bRes)
	{
		CLIENT_SCOPED_ERROR << "Cannot load configuration file " << fileName <<"; error : " << strError;
	}
	else
	{
		CLIENT_SCOPED_LOG << "Configuration file " << fileName << " has been loaded successfully";
	}

	statusRecipeName->setText(fileName);
}

///////////////////////////////////////////////////////////////////////
////////////////  PARAMETERS UI
///////////////////////////////////////////////////////////////////////

void JuneUIWnd::enableUIForProcessing(bool bEnable)
{
	if (!isUIMode())
		return;

	startAct->setEnabled(bEnable);
	startOnceAct->setEnabled(bEnable);
	stopAct->setEnabled(!bEnable);
	ui.dockWidgetContents->setEnabled(bEnable);

	if (!bEnable)
	{
		_progressBarTimer.start();
		statusGeneral->setText("Working...");
	}
	else
	{
		_progressBarTimer.stop();
		statusProgressBar->setValue(0);
		statusGeneral->setText("Idle");
		statusFramesHandled->setText("");
		iconGeneral->setPixmap(_greyLed);
	}
}

void JuneUIWnd::onAddColor()
{
	bool ok;
    QString text = QInputDialog::getText(this, tr("Add C2C color)"),
                                         tr("Color name"), QLineEdit::Normal,
                                         "Black", &ok);
    if (ok && !text.isEmpty())
        addNewColor(text);
}

void JuneUIWnd::onRemoveColor()
{
	auto const& idx = ui.batchParamView->selectionModel()->currentIndex();
	if ( idx.isValid() )
	{
		const auto typeName = _processParamModelEditable->itemType(idx);
		if ( typeName == "LandaJune::Parameters::COLOR_TRIPLET" )
		{
			const auto&[name, colorVal, editable] = _processParamModelEditable->getPropertyTuple(idx);
			if ( QMessageBox::question(this, "Remove C2C color", QString("Are you sure to remove color %1 ?").arg(colorVal.value<COLOR_TRIPLET>().ColorName())) == QMessageBox::Yes )
			{
				// get parent of index we are going to remove
				const auto parent = _processParamModelEditable->parent(idx);

				// remove the item
				_processParamModelEditable->removeRows(idx.row(), 1, _processParamModelEditable->parent(idx));

				// update remaining color triplets
				QVector<COLOR_TRIPLET> newColorArray;
				const auto rowCount = _processParamModelEditable->rowCount(parent);
				for (auto i = 0; i < rowCount; i++ )
				{
					auto const& newIdx = _processParamModelEditable->index(i,0, parent);
					const auto&[name, colorVal, editable] = _processParamModelEditable->getPropertyTuple(newIdx);
					newColorArray.push_back(colorVal.value<COLOR_TRIPLET>());
				}

				const auto batchParams = ICore::get()->getProcessParameters();
				batchParams->setParamProperty("ColorArray", QVariant::fromValue(newColorArray));
			}
		}
	}
}

void JuneUIWnd::addNewColor(const QString& colorName )
{
	auto idxList = 	_processParamModelEditable->findItem("ColorArray");
	if (!idxList.isEmpty() )
	{
		auto const& parentIdx = idxList[0];

		// update remaining color triplets
		QVector<COLOR_TRIPLET> newColorArray;
		const auto rowCount = _processParamModelEditable->rowCount(parentIdx);
		for ( int i = 0; i < rowCount; i++ )
		{
			auto const& newIdx = _processParamModelEditable->index(i,0, parentIdx);
			const auto&[name, colorVal, editable] = _processParamModelEditable->getPropertyTuple(newIdx);
			newColorArray.push_back(colorVal.value<COLOR_TRIPLET>());
		}

		COLOR_TRIPLET newTriplet;
		newTriplet.Min().setColorName(colorName);
		newTriplet.Max().setColorName(colorName);
		newTriplet.setColorName(colorName);
		newColorArray.push_back(newTriplet);

		const auto batchParams = ICore::get()->getProcessParameters();
		batchParams->setParamProperty("ColorArray", QVariant::fromValue(newColorArray));

		onUpdateProcessParams();
	}
}

void JuneUIWnd::onTimerTick()
{
	auto const& val = statusProgressBar->value();
	statusProgressBar->setValue(val == 100 ? 0 : val + 1);

	const auto& ledPx = (statusProgressBar->value() & 1 ) ? _greyLed : _greenLed;
	iconGeneral->setPixmap(ledPx);
}

void JuneUIWnd::processParamSelectionChanged(const  QModelIndex& current, const  QModelIndex& prev)
{
	if ( !current.isValid())
	{
		removeColor->setEnabled(false);
		return;
	}

	const auto typeName = _processParamModelEditable->itemType(current);
	removeColor->setEnabled( typeName == "LandaJune::Parameters::COLOR_TRIPLET" );
}