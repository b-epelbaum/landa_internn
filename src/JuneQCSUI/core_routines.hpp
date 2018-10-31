#pragma once

void JuneUIWnd::initCore()
{
	CLIENT_SCOPED_LOG << "Initializing core engine...";
	auto core = ICore::get();

	connect (core.get()->getClassObject(), SIGNAL(coreStopped()), this, SLOT(onCoreStopped()) );
	connect (core.get()->getClassObject(), SIGNAL(coreException(const LandaJune::BaseException&)), this, SLOT(onCoreException(const LandaJune::BaseException&)) );
	connect (core.get()->getClassObject(), SIGNAL(frameData(std::shared_ptr<LandaJune::Core::SharedFrameData>)), this, SLOT(onSharedFrameData(std::shared_ptr<LandaJune::Core::SharedFrameData>)) );
	connect (core.get()->getClassObject(), SIGNAL(offlineFileSourceCount(int)), this, SLOT(onOfflineFileCount(int)) );
	connect (core.get()->getClassObject(), SIGNAL(frameProcessed(int)), this, SLOT(onFrameProcessed(int)) );
	try
	{
		core->init(isUIMode());
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

void JuneUIWnd::onFrameProcessed ( int frameIndex )
{
	if ( statusProgressBar->maximum() != 0)
	{
		statusProgressBar->setValue(statusProgressBar->value() + 1);
		statusFrameCount->setText(QString(" Frames processed : ") + QString::number(statusProgressBar->value()));
	}
	else
	{
		statusFrameCount->setText(QString(" Frames processed : ") + QString::number(frameIndex+1));
	}
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
