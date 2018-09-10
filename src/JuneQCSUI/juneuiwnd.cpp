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
#include "interfaces/IAlgorithmHandler.h"
#include "ProcessParameters.h"
#include "common/june_exceptions.h"
#include "RealTimeStats.h"
#include "applog.h"
#include <QJsonDocument>
#include <QSettings>
#include <qstandardpaths.h>


using namespace LandaJune::UI;
using namespace LandaJune::Core;
using namespace LandaJune::Loggers;
using namespace LandaJune::FrameProviders;
using namespace LandaJune::Parameters;

#define CLIENT_SCOPED_LOG PRINT_INFO8 << "[JuneUIWnd] : "
#define CLIENT_SCOPED_WARNING PRINT_WARNING << "[JuneUIWnd] : "
#define CLIENT_SCOPED_ERROR PRINT_ERROR << "[JuneUIWnd] : "

JuneUIWnd::JuneUIWnd(QWidget *parent)
	: QMainWindow(parent)
{
	Q_UNUSED(AppLogger::createLogger());
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

void JuneUIWnd::init()
{
	initUI();
	setWindowState(Qt::WindowMaximized);
	initCore();
	enumerateFrameProviders();
	enumerateAlgoHandlers();
	initProcessParameters();

	connect(ui.frameSourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &JuneUIWnd::onFrameProviderComboChanged);
	connect(ui.algoHandlerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &JuneUIWnd::onAlgoHandlerComboChanged);


}

void JuneUIWnd::initUI()
{
	CLIENT_SCOPED_LOG << "Initializing UI...";
	_imageBox = new QLabel(this);
	ui.setupUi(this);

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
	connect(_processParamModelEditable.get(), &ParamPropModel::propChanged, this, &JuneUIWnd::onBatchPropChanged);

	_processParamModelCalculated = std::make_unique<ParamPropModel>(this);
	ui.processParamViewCalculated->setModel(_processParamModelCalculated.get());

	//connect(ui.btnAddProp, &QPushButton::clicked, this, &JuneUIWnd::onBtnAddPropClicked);
	//connect(ui.btnRemoveProp, &QPushButton::clicked, this, &JuneUIWnd::onBtnRemovePropClicked);

	_progressBarTimer.setSingleShot(false);
	_progressBarTimer.setInterval(300);

	connect(&_progressBarTimer, &QTimer::timeout, this, &JuneUIWnd::onTimerTick);
}

void JuneUIWnd::initCore()
{
	CLIENT_SCOPED_LOG << "Initializing core engine...";
	auto core = ICore::get();
	//try
	//{
		core->init();
	//}
	//catch (CoreEngineException& e)
	//{

	//}
	CLIENT_SCOPED_LOG << "Core engine initialized";
}

void JuneUIWnd::enumerateFrameProviders() 
{
	CLIENT_SCOPED_LOG << "Enumerating frame providers...";
	ui.frameSourceCombo->clear();
	//try
	//{
		auto listOfProviders = ICore::get()->getFrameProviderList();
		for (auto& provider : listOfProviders)
		{
			CLIENT_SCOPED_LOG << "\tadding provider ==> " << provider->getName();
			ui.frameSourceCombo->addItem(provider->getName(), QVariant::fromValue(provider));
		}
		listOfProviders.empty() ? ui.frameSourceCombo->setCurrentIndex(-1) : ui.frameSourceCombo->setCurrentIndex(0);
		QSettings settings("Landa Corp", "June QCS");
		auto lastFrameProvider = settings.value("UIClient/lastSelectedProvider").toString();

		if ( !lastFrameProvider.isEmpty() )
		{
			CLIENT_SCOPED_LOG << "Found last selected frame provider ==> " <<  lastFrameProvider;
			if ( ui.frameSourceCombo->findText(lastFrameProvider) != - 1 )
				ui.frameSourceCombo->setCurrentText(lastFrameProvider);
		}
		else
		{
			CLIENT_SCOPED_LOG << "The last selected frame provider has not been found, setting default value...";
		}

		this->onFrameProviderComboChanged (ui.frameSourceCombo->currentIndex());

	//}
	//catch ( CoreEngineException& ex)
	//{
		
	//}

	CLIENT_SCOPED_LOG << "Finished frame providers enumeration";

}

void JuneUIWnd::enumerateAlgoHandlers()
{
	CLIENT_SCOPED_LOG << "Enumerating algorithm handlers...";
	ui.algoHandlerCombo->clear();
	//try
	//{
		auto listOfAlgo = ICore::get()->getAlgorithmHandlerList();
		for (auto& algo : listOfAlgo)
		{
			CLIENT_SCOPED_LOG << "\tadding algorith handler ==> " << algo->getName();
			ui.algoHandlerCombo->addItem(algo->getName(), QVariant::fromValue(algo));
		}
		listOfAlgo.empty() ? ui.algoHandlerCombo->setCurrentIndex(-1) : ui.algoHandlerCombo->setCurrentIndex(0);

		QSettings settings("Landa Corp", "June QCS");
		auto lastAlgoHandler = settings.value("UIClient/lastSelectedAlgorithm").toString();

		if ( !lastAlgoHandler.isEmpty() )
		{
			CLIENT_SCOPED_LOG << "Found last selected algorithm handler ==> " <<  lastAlgoHandler;
			if ( ui.algoHandlerCombo->findText(lastAlgoHandler) != - 1 )
				ui.algoHandlerCombo->setCurrentText(lastAlgoHandler);
		}
	else
		{
			CLIENT_SCOPED_LOG << "The last selected algorithm handler has not been found, setting default value...";
		}

		onAlgoHandlerComboChanged (ui.algoHandlerCombo->currentIndex());


	//}
	//catch (CoreEngineException& ex)
	//{

	//}
	CLIENT_SCOPED_LOG << "Finished algorithm handlers enumeration";
}


void JuneUIWnd::initProcessParameters() const
{
	CLIENT_SCOPED_LOG << "Setting process parameters...";
	//try
	//{
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

		CLIENT_SCOPED_LOG << "Looking for saved process parameters configuration file...";
		QSettings settings("Landa Corp", "June QCS");
		auto lastConfigFile = settings.value("UIClient/lastConfigFile").toString();

		if ( !lastConfigFile.isEmpty())
		{
			CLIENT_SCOPED_LOG << "Saved process parameters configuration file found : " << lastConfigFile << "; loading...";
			QString strError;
			auto const bRes = ICore::get()->getProcessParameters()->load(lastConfigFile, strError);
			if ( !bRes)
			{
				CLIENT_SCOPED_ERROR << "Cannot load configuration file " << lastConfigFile <<"; error : " << strError;
			}
			else
			{
				CLIENT_SCOPED_LOG << "Configuration file " << lastConfigFile << " has been loaded successfully";
			}
		}
		else
		{
			CLIENT_SCOPED_LOG << "Last configuration file path is empty. Setting default values...";
		}

	//}
	//catch (CoreEngineException& ex)
	//{

	//}
}


void JuneUIWnd::start()
{
	if (ui.frameSourceCombo->currentIndex() == -1)
	{
		CLIENT_SCOPED_WARNING << "[JuneUIWnd] : no valid frame provider selected. Aborted ";
		return;
	}

	if (ui.algoHandlerCombo->currentIndex() == -1)
	{
		CLIENT_SCOPED_WARNING << "[JuneUIWnd] : no valid algorithm handler selected. Aborted ";
		return;
	}


	//try
	//{
		const auto selectedProvider = ui.frameSourceCombo->currentData().value<FrameProviderPtr>();
		const auto selectedAlgoHandler = ui.algoHandlerCombo->currentData().value<AlgorithmHandlerPtr>();
		
		if (selectedProvider && selectedAlgoHandler)
		{
			PRINT_DEBUG_DBLINE;
			CLIENT_SCOPED_LOG << "[JuneUIWnd] : Selected frame provider : " << selectedProvider->getName();
			CLIENT_SCOPED_LOG << "[JuneUIWnd] : Selected algorithm handler : " << selectedAlgoHandler->getName();
			CLIENT_SCOPED_LOG << "starting processing...";
			PRINT_DEBUG_DBLINE;

			ICore::get()->selectAlgorithmHandler(selectedAlgoHandler);
			ICore::get()->selectFrameProvider(selectedProvider);
			ICore::get()->start();
		}
	//}
	//catch (CoreEngineException& ex)
	//{
	//	return;
	//}

	enableUIForProcessing(false);
	_bRunning = true;
	_updateStatsTimer->start(100);
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
}

void JuneUIWnd::createStatusBar()
{
	startAct = new QAction(QIcon(":/JuneUIWnd/Resources/start.png"), tr("&Start"), this);
	stopAct = new QAction(QIcon(":/JuneUIWnd/Resources/stop.png"), tr("&Stop"), this);

	loadConfig = new QAction(QIcon(":/JuneUIWnd/Resources/file_open.png"), tr("&Load"), this);
	saveConfig = new QAction(QIcon(":/JuneUIWnd/Resources/file_save.png"), tr("&Save"), this);

	connect(saveConfig, &QAction::triggered, this, &JuneUIWnd::onSaveConfig);
	connect(loadConfig, &QAction::triggered, this, &JuneUIWnd::onLoadConfig);

	ui.saveConfigButt->setDefaultAction(saveConfig);
	ui.loadConfigButt->setDefaultAction(loadConfig);

	stopAct->setEnabled(false);

	connect(startAct, &QAction::triggered, this, &JuneUIWnd::start);
	connect(stopAct, &QAction::triggered, this, &JuneUIWnd::stop);
	ui.mainToolBar->addAction(startAct);
	ui.mainToolBar->addAction(stopAct);
	ui.mainToolBar->setIconSize(QSize(48, 48));


	statusGeneral = new QLabel(this);
	statusFrameProv = new QLabel(this);
	statusAlgoHandler = new QLabel(this);
	statusFramesHandled = new QLabel(this);
	statusFramesDropped = new QLabel(this);

	statusGeneral->setMinimumWidth(200);
	statusGeneral->setText("Idle");
	
	statusProgressBar = new QProgressBar(this);
	statusProgressBar->setMaximumWidth(200);

	statusProgressBar->setTextVisible(false);

    // add the two controls to the status bar
    ui.statusBar->addPermanentWidget(statusGeneral);
    ui.statusBar->addPermanentWidget(statusProgressBar,1);
	ui.statusBar->addPermanentWidget(statusFrameProv,2);
	ui.statusBar->addPermanentWidget(statusAlgoHandler,3);
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

void JuneUIWnd::stop()
{
	CLIENT_SCOPED_LOG << "stopping processing...";
	
	//try
	//{
		ICore::get()->stop();
	//}
	//catch (CoreEngineException& ex)
	//{
	//}


	Helpers::RealTimeStats::rtStats().reset();
	_updateStatsTimer->stop();
	CLIENT_SCOPED_LOG << " ----------- processing stopped --------------";
	enableUIForProcessing(true);
	_bRunning = false;
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

void JuneUIWnd::onFrameProviderComboChanged(int index)
{
	const auto selectedProvider = ui.frameSourceCombo->itemData(index).value<FrameProviderPtr>();
	if (!selectedProvider)
	{
		CLIENT_SCOPED_ERROR << "[JuneUIWnd] : selected frame provider is invalid. Aborted ";
		return;
	}

	QSettings settings("Landa Corp", "June QCS");
	settings.setValue("UIClient/lastSelectedProvider",selectedProvider->getName() );

	ICore::get()->selectFrameProvider(selectedProvider);
	ui.providerDescEdit->document()->setPlainText(selectedProvider->getDescription());
	_providerParamModel->setupModelData(selectedProvider->getProviderProperties(), true);
}

void JuneUIWnd::onAlgoHandlerComboChanged(int index)
{
	const auto selectedAlgo = ui.algoHandlerCombo->itemData(index).value<AlgorithmHandlerPtr>();
	if (!selectedAlgo)
	{
		CLIENT_SCOPED_ERROR << "[JuneUIWnd] : selected algorithm handler is invalid. Aborted ";
		return;
	}

	QSettings settings("Landa Corp", "June QCS");
	settings.setValue("UIClient/lastSelectedAlgorithm",selectedAlgo->getName() );

	ui.algoDescEdit->document()->setPlainText(selectedAlgo->getDescription());
}


void JuneUIWnd::updateStats() const
{
	ui.statView->clear(); // unless you know the editor is empty
	ui.statView->appendPlainText(QString::fromStdString(Helpers::RealTimeStats::rtStats()->to_string()));
}

void JuneUIWnd::onProviderPropChanged(QString propName, const QVariant& newVal)
{
	auto selectedProvider = ui.frameSourceCombo->currentData().value<FrameProviderPtr>();
	if (selectedProvider)
		selectedProvider->setProviderProperty(propName, newVal);
}

void JuneUIWnd::onBatchPropChanged(QString propName, const QVariant& newVal)
{
	//try
	//{
		const auto batchParams = ICore::get()->getProcessParameters();
		batchParams->setParamProperty(propName, newVal);
	//}
	//catch (CoreEngineException& ex)
	//{
	//}

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
	if ( idx.isValid())
	{
		 ui.batchParamView->selectionModel()->setCurrentIndex(ui.batchParamView->model()->index(idx.row(), idx.column()), QItemSelectionModel::Select);
	}
}

void JuneUIWnd::onUpdateCalculatedParams()
{
	_processParamModelCalculated->setupModelData(ICore::get()->getProcessParameters()->getReadOnlyPropertyList(), true);
	onFrameProviderComboChanged(ui.frameSourceCombo->currentIndex());
	//ui.processParamViewCalculated->expandToDepth(1);
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
}


///////////////////////////////////////////////////////////////////////
////////////////  PARAMETERS UI
///////////////////////////////////////////////////////////////////////

void JuneUIWnd::enableUIForProcessing(bool bEnable)
{
	startAct->setEnabled(bEnable);
	stopAct->setEnabled(!bEnable);
	ui.dockWidgetContents->setEnabled(bEnable);

	if (!bEnable)
	{
		_progressBarTimer.start();
	}
	else
	{
		_progressBarTimer.stop();
		statusProgressBar->setValue(0);
	}
}

void JuneUIWnd::onBtnAddPropClicked() noexcept
{
	const auto list = ui.batchParamView->selectionModel()->selectedIndexes();
	if (!list.isEmpty()) {
		_processParamModelEditable->copyParam(list.first());
	}
}

void JuneUIWnd::onBtnRemovePropClicked() noexcept
{
	const auto list = ui.batchParamView->selectionModel()->selectedIndexes();
	if (!list.isEmpty()) {
		_processParamModelEditable->removeParam(list.first());
	}
}

void JuneUIWnd::onTimerTick()
{
	auto const& val = statusProgressBar->value();
	statusProgressBar->setValue(val == 100 ? 0 : val + 1);
}