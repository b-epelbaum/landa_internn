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

#include "ProcessParameter.h"

#include "common/june_exceptions.h"
#include "RealTimeStats.h"
#include "applog.h"



using namespace LandaJune::UI;
using namespace LandaJune::Core;
using namespace LandaJune::Loggers;
using namespace LandaJune::FrameProviders;
using namespace LandaJune::Parameters;

#define CLIENT_SCOPED_LOG PRINT_INFO3 << "[JuneUIWnd] : "
#define CLIENT_SCOPED_WARNING PRINT_WARNING << "[JuneUIWnd] : "
#define CLIENT_SCOPED_ERROR PRINT_ERROR << "[JuneUIWnd] : "

//X Press (Y scanner): 0.08660258
//Y Press (X scanner): 0.08466683



JuneUIWnd::JuneUIWnd(QWidget *parent)
	: QMainWindow(parent)
{
	Q_UNUSED(AppLogger::createLogger());
	CLIENT_SCOPED_LOG << "started Landa-June POC";
	init();
}

void JuneUIWnd::init()
{
	initUI();
	initCore();
	enumerateFrameProviders();
	initBatchParameters();
}

void JuneUIWnd::initUI()
{
	_imageBox = new QLabel(this);
	ui.setupUi(this);

	_scrollArea = ui.scrollArea;
	_imageBox->setBackgroundRole(QPalette::Base);
	_imageBox->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	_imageBox->setScaledContents(true);

	_scrollArea->setBackgroundRole(QPalette::Dark);
	_scrollArea->setWidget(_imageBox);
	
	connect(ui.frameSourceCombo,	QOverload<int>::of(&QComboBox::currentIndexChanged), this, &JuneUIWnd::onFrameProviderComboChanged);

	createActions();
	createStatusBar();

	connect(qApp, &QCoreApplication::aboutToQuit, this, &JuneUIWnd::onAboutToQuit);
	
	_updateStatsTimer = new QTimer(this);
	connect(_updateStatsTimer, SIGNAL(timeout()), this, SLOT(updateStats()));

	_providerParamModel = std::make_unique<ProvidePropsModel>(this);
	ui.providerPropView->setModel(_providerParamModel.get());
	connect(_providerParamModel.get(), &ProvidePropsModel::propChanged, this, &JuneUIWnd::onProviderPropChanged);

	_batchParamModel = std::make_unique<ProvidePropsModel>(this);
	ui.batchParamView->setModel(_batchParamModel.get());
	connect(_batchParamModel.get(), &ProvidePropsModel::propChanged, this, &JuneUIWnd::onBatchPropChanged);
	
	//ComboBoxItemDelegate* cbid = new ComboBoxItemDelegate(ui.providerPropView);
	//ui.providerPropView->setItemDelegate(cbid);
}

void JuneUIWnd::initCore()
{
	auto core = ICore::get();
	try
	{
		core->init();
	}
	catch (CoreEngineException& e)
	{

	}
	
	/*
	auto core = CoreEngine::get();
	try
	{
		core->init();
	}
	catch (CoreEngineException& e)
	{

	}
	*/
}

void JuneUIWnd::enumerateFrameProviders() const
{
	ui.frameSourceCombo->clear();
	try
	{
		auto listOfProviders = ICore::get()->getFrameProviderList();
		for (auto& provider : listOfProviders)
		{
			ui.frameSourceCombo->addItem(provider->getName(), QVariant::fromValue(provider));
		}
		listOfProviders.empty() ? ui.frameSourceCombo->setCurrentIndex(-1) : ui.frameSourceCombo->setCurrentIndex(0);
	}
	catch ( CoreEngineException& ex)
	{
		
	}
}


void JuneUIWnd::initBatchParameters() const
{
	try
	{
		const auto batchParams = ICore::get()->getProcessParameters();
		_batchParamModel->setupModelData(batchParams->getPropertyList());
	}
	catch (CoreEngineException& ex)
	{

	}
}


void JuneUIWnd::start()
{
	if (ui.frameSourceCombo->currentIndex() == -1)
	{
		CLIENT_SCOPED_WARNING << "[JuneUIWnd] : no valid frame provider selected. Aborted ";
		return;
	}

	try
	{
		const auto selectedProvider = ui.frameSourceCombo->currentData().value<FrameProviderPtr>();
		if (selectedProvider)
		{
			PRINT_DEBUG_DBLINE;
			CLIENT_SCOPED_LOG << "[JuneUIWnd] : Selected frame provider : " << selectedProvider->getName();
			CLIENT_SCOPED_LOG << "starting processing...";
			PRINT_DEBUG_DBLINE;

			ICore::get()->selectFrameProvider(selectedProvider);
			ICore::get()->start();
		}
	}
	catch (CoreEngineException& ex)
	{
		return;
	}

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
	stopAct = new QAction(QIcon(":/JuneUIWnd/Resources/stop.jpg"), tr("&Stop"), this);

	stopAct->setEnabled(false);

	connect(startAct, &QAction::triggered, this, &JuneUIWnd::start);
	connect(stopAct, &QAction::triggered, this, &JuneUIWnd::stop);
	ui.mainToolBar->addAction(startAct);
	ui.mainToolBar->addAction(stopAct);
}

/*
void JuneUIWnd::browseForFolder()
{
	const auto btn = dynamic_cast<QAbstractButton*>(sender());
	const auto sType = btn->property("browseType");
	
	QString strTitle = "Select Images Source Folder";
	QString startFolder = _imageSourceFolder;
	QString * pTargetString = &_imageSourceFolder;
	if (sType == "trg")
	{
		strTitle = "Select Images Target Folder";
		startFolder = _imageTargetFolder;
		pTargetString = &_imageTargetFolder;
	}

	if (startFolder.isEmpty())
		startFolder = QDir::currentPath();

	auto strFolder = QFileDialog::getExistingDirectory(this, strTitle, startFolder);
	if (strFolder.isEmpty())
		return;

	*pTargetString = strFolder;
	recreateIPParams();
}
*/


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

QString JuneUIWnd::createRunTargetFolder(const QString& commonTargetFolder)
{
	auto runFolderPath = commonTargetFolder + QDir::separator() + QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
	if (!QDir(runFolderPath).exists())
	{
		Q_UNUSED(QDir().mkdir(runFolderPath));
	}
	return std::move(runFolderPath);
}


void JuneUIWnd::stop()
{
	CLIENT_SCOPED_LOG << "stopping processing...";
	
	try
	{
		ICore::get()->stop();
	}
	catch (CoreEngineException& ex)
	{
	}


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

	CLIENT_SCOPED_LOG << "[ ---------------- Finished Client -------------]";
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
	_providerParamModel->setupModelData(selectedProvider->getProviderProperties());
}

void JuneUIWnd::updateStats() const
{
	ui.plainTextEdit->clear(); // unless you know the editor is empty
	ui.plainTextEdit->appendPlainText(QString::fromStdString(Helpers::RealTimeStats::rtStats()->to_string()));
}

void JuneUIWnd::onProviderPropChanged(QString propName, const QVariant& newVal)
{
	auto selectedProvider = ui.frameSourceCombo->currentData().value<FrameProviderPtr>();
	if (selectedProvider)
		selectedProvider->setProviderProperty(propName, newVal);
}

void JuneUIWnd::onBatchPropChanged(QString propName, const QVariant& newVal)
{
	try
	{
		const auto batchParams = ICore::get()->getProcessParameters();
		batchParams->setParamProperty(propName, newVal);
	}
	catch (CoreEngineException& ex)
	{
	}

}



///////////////////////////////////////////////////////////////////////
////////////////  PARAMETERS UI
///////////////////////////////////////////////////////////////////////

void JuneUIWnd::enableUIForProcessing(bool bEnable)
{
	startAct->setEnabled(bEnable);
	stopAct->setEnabled(!bEnable);
	ui.groupBox_4->setEnabled(bEnable);
}

