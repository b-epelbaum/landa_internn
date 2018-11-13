#include "BaseCore.h"
#include "applog.h"
#include "common/june_enums.h"

#include "interfaces/IFrameProvider.h"
#include "interfaces/IAlgorithmRunner.h"

#include "BackgroundThreadPool.h"
#include "writequeue.h"
#include "functions.h"
#include "ProcessParameters.h"
#include "FrameRefPool.h"
#include <filesystem>
#include <iomanip>

#include <QStandardPaths>

namespace fs = std::filesystem;

#ifdef _DEBUG
#pragma comment(lib, "opencv_world342d.lib")
#else
#pragma comment(lib, "opencv_world342.lib")
#endif


using namespace LandaJune;
using namespace Core;
using namespace Parameters;
using namespace FrameProviders;
using namespace Algorithms;
using namespace Threading;
using namespace Functions;

#define CLASSNAME(x) qPrintable(x->GetMetaClassDebugName())
#define MYCLASSNAME qPrintable(GetMetaClassDebugName())

#define CORE_SCOPED_LOG PRINT_INFO2 << "[Core] : "
#define CORE_SCOPED_ERROR PRINT_ERROR << "[Core] : "
#define CORE_SCOPED_WARNING PRINT_WARNING << "[Core] : "

#define CHECK_IF_INITED if (!_bInited ) \
{ \
	THROW_EX_INT(CORE_ERROR::ERR_CORE_NOT_INITIALIZED); \
}


BaseCore::EVENT_MAP BaseCore::_eventParserMap =
{
	  {CoreCallbackType::CALLBACK_PROVIDER_SCANNED_FILES_COUNT,			&BaseCore::on_ProviderScannedFilesCount }
	, {CoreCallbackType::CALLBACK_PROVIDER_FRAME_GENERATED_OK,			&BaseCore::on_ProviderFrameGeneratedOk }
	, {CoreCallbackType::CALLBACK_PROVIDER_FRAME_SKIPPED,				&BaseCore::on_ProviderFrameSkipped }
	, {CoreCallbackType::CALLBACK_PROVIDER_FINISHED,					&BaseCore::on_ProviderFinished }
	, {CoreCallbackType::CALLBACK_PROVIDER_FRAME_IMAGE_DATA,			&BaseCore::on_ProviderFrameImageData }
	, {CoreCallbackType::CALLBACK_PROVIDER_EXCEPTION,					&BaseCore::on_ProviderException }
																		
	, {CoreCallbackType::CALLBACK_RUNNER_FRAME_HANDLED_OK,				&BaseCore::on_RunnerFrameHandledOk }
	, {CoreCallbackType::CALLBACK_RUNNER_FRAME_SKIPPED,					&BaseCore::on_RunnerFrameSkipped }
	, {CoreCallbackType::CALLBACK_RUNNER_DETECTION_OK,					&BaseCore::on_RunnerDetectionSuccess }
	, {CoreCallbackType::CALLBACK_RUNNER_DETECTION_FAILED,				&BaseCore::on_RunnerDetectionFailure }
	, {CoreCallbackType::CALLBACK_RUNNER_EXCEPTION,						&BaseCore::on_RunnerException }

	, {CoreCallbackType::CALLBACK_IMAGE_SAVER_ERROR,					&BaseCore::on_SaverError }
};

void BaseCore::init(bool reportEvents )
{
	_reportEvents = reportEvents;

	qRegisterMetaType<BaseException>("BaseException");
	qRegisterMetaType<std::exception_ptr>("std::exception_ptr");

	if (_bInited)
	{
		CORE_SCOPED_WARNING << "Core is already initialized";
		return;
	}


	CORE_SCOPED_LOG << "-------------------------------------------------------------";
	CORE_SCOPED_LOG << " Core Initialization started...";

	initProcessParameters();

	// scan and init frame providers
	initProviders();
	initAlgorithmRunners();

	_bInited = true;
	CORE_SCOPED_LOG << " Core Initialization finished...";
}

void BaseCore::cleanup()
{
	_bInited = false;
}

const std::list<FrameProviderPtr>& BaseCore::getFrameProviderList() const
{
	CHECK_IF_INITED
	return _providerList;
}

const std::list<AlgorithmRunnerPtr>& BaseCore::getAlgorithmRunnerList() const
{
	CHECK_IF_INITED
	return _algorithmRunnerList;
}

BaseParametersPtr BaseCore::getProcessParameters()
{
	CHECK_IF_INITED
	return _processParameters;
}


void BaseCore::selectFrameProvider(FrameProviderPtr provider)
{
	CHECK_IF_INITED

	if (_currentFrameProvider == provider)
		return;

	if (_currentFrameProvider)
	{
		if (_currentFrameProvider->isBusy())
		{
			THROW_EX_INT(CORE_ERROR::ERR_CORE_PROVIDER_IS_BUSY);
		}

		_currentFrameProvider->cleanup();
	}

	_currentFrameProvider = std::move(provider);
}

FrameProviderPtr BaseCore::getSelectedFrameProvider() const
{
	CHECK_IF_INITED
	return _currentFrameProvider;
}

void BaseCore::selectAlgorithmRunner(AlgorithmRunnerPtr algoRunner)
{
	CHECK_IF_INITED

	if (_currentAlgorithmRunner == algoRunner)
		return;

	if (_currentAlgorithmRunner)
		_currentAlgorithmRunner->cleanup();

	_currentAlgorithmRunner = std::move(algoRunner);
}

AlgorithmRunnerPtr BaseCore::getSelectedAlgorithmRunner() const
{
	CHECK_IF_INITED
	return _currentAlgorithmRunner;
}

void BaseCore::runOne()
{
	const auto processParams = std::dynamic_pointer_cast<ProcessParameters>(_processParameters);
	std::shared_ptr<ProcessParameters> processParametersOnce;
	processParametersOnce.reset(new ProcessParameters(*processParams.get()));

	// clean target folder
	
	cleanDestinationFolder(QString::fromStdString(getRootFolderForOneRun()));
	
	processParametersOnce->setImageMaxCount(1);
	processParametersOnce->setRootImageOutputFolder(QString::fromStdString(getRootFolderForOneRun()));

	run(processParametersOnce);
}

void BaseCore::runAll()
{
	// init global parameters
	run(_processParameters);
}


void BaseCore::run(BaseParametersPtr params)
{
	CHECK_IF_INITED
	if (!_currentFrameProvider)
	{
		THROW_EX_INT(CORE_ERROR::ERR_CORE_NO_PROVIDER_SELECTED);
	}


	if (!_currentAlgorithmRunner)
	{
		THROW_EX_INT(CORE_ERROR::ERR_CORE_NO_ALGORITHM_RUNNER_SELECTED);
	}

	if (_currentFrameProvider->isBusy())
	{
		THROW_EX_INT(CORE_ERROR::ERR_CORE_PROVIDER_IS_BUSY);
	}

	// TODO : find better solution for waiting flag
	// reset the waiting flag
	_waitingForFirstSkippedHandledFrame = false;

	// initialize frame pool
	initFramePool();


	// start file writer
	initFileWriter(true);

	// start one and only frame consuming thread
	// the entry point of this thread is the "frameConsume" static function, which uses image processing and file saving thread pools inside
	// see "frameConsume" implementation
	// frameRunnerCleanup will run when the consumer thread is going to exit
	// 

	frameConsumerThread().setName ("Frame Handler");
	frameConsumerThread().setThreadFunction(frameConsume, params, _currentAlgorithmRunner, this, coreEventCallback );
	frameConsumerThread().setThreadExitFunction(frameRunnerCleanup,_currentAlgorithmRunner, this, coreEventCallback );
	frameConsumerThread().start();

	
	// start one and only frame producing/generation/capture thread
	// the entry point of this thread is the "frameGenerate" static function, which invokes the data generation functions of IAbstractImageProvider object
	// see "frameGenerate" implementation
	// frameGeneratorCleanup will run when the producer thread is going to exit
	frameProducerThread().setName ("Frame Producer");
	frameProducerThread().setThreadFunction(frameGenerate, params, _currentFrameProvider, this, coreEventCallback);
	frameProducerThread().setThreadExitFunction(frameGeneratorCleanup,_currentFrameProvider, this, coreEventCallback );
	frameProducerThread().start();
}

void BaseCore::stop()
{
	CHECK_IF_INITED
	if (!_currentFrameProvider)
	{
		return;
	}

	CORE_SCOPED_LOG << "Stopping Frame producer thread...";
	frameProducerThread().stop();
	CORE_SCOPED_LOG << "Stopping Frame consumer thread...";
	frameConsumerThread().stop();

	frameProducerThread().join();
	CORE_SCOPED_LOG << "Frame producer thread finished";
	frameConsumerThread().join();
	CORE_SCOPED_LOG << "Frame runner thread finished";

	// stop file writer
	// TODO : change the waiting behaviour, because writing queue can be still busy
	initFileWriter(false);
	CORE_SCOPED_LOG << "File writer stopped";

	if ( _currentFrameProvider)
		_currentFrameProvider->cleanup();
	FrameRefPool::frameRefPool()->cleanup();
	
	if ( _currentAlgorithmRunner)
		_currentAlgorithmRunner->cleanup();

	_waitingForFirstSkippedHandledFrame = false;
	emit coreStopped();
}

void BaseCore::cleanDestinationFolder (const QString& destFolder )
{
	QDir dir(destFolder);
	//dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot );
	if (!dir.removeRecursively() )
	{
		// iterate on files and rename them 
	}

	/*
	for(auto const& dirFile :  dir.entryList())
	{
		if (!dir.remove(dirFile))
			CORE_SCOPED_ERROR << "Cannot remove file for one run : " << dirFile;
	}
	*/
}

std::string BaseCore::getRootFolderForOneRun() const
{
	const auto processParams = std::dynamic_pointer_cast<ProcessParameters>(_processParameters);
	auto tempRoot =  processParams->RootImageOutputFolder();
	//auto tempRoot = QStandardPaths::standardLocations(QStandardPaths::TempLocation)[0];

	tempRoot += "/OneRun/";

	auto stdPath = tempRoot.toStdString();

	fs::path p{ stdPath };
	auto const parentPath = p.parent_path();
	if (!is_directory(parentPath) || !exists(parentPath))
	{
		try
		{
			create_directories(parentPath); // create src folder
		}
		catch (fs::filesystem_error& er)
		{
			PRINT_ERROR << "Cannot create folder " << stdPath.c_str() << "; exception caught : " << er.what();
			RETHROW_STR (CORE_ERROR::ERR_CORE_CANNOT_CREATE_FOLDER, "Cannot create folder " + stdPath);
		}
	}

	return tempRoot.toStdString();
}

bool BaseCore::isBusy()
{
	if (!_bInited)
		return false;
	if (!_currentFrameProvider)
		return false;

	return _currentFrameProvider->isBusy();
}

QString BaseCore::getDefaultConfigurationFileName() const
{
	return "";
}

void BaseCore::saveConfiguration()
{
}

void BaseCore::initProcessParameters()
{
	_processParameters = std::make_shared<ProcessParameters>();
}

void BaseCore::initFramePool() const
{
	try
	{
		FrameRefPool::frameRefPool()->init(_currentFrameProvider->getRecommendedFramePoolSize());
	}
	catch (BaseException& e)
	{
		CORE_SCOPED_ERROR << "Error initializing frame pool; error ID : " << e.errorID();
	}
}

void BaseCore::initProviders()
{
	_providerList = IFrameProvider::enumerateImageProviders();
}

void BaseCore::initAlgorithmRunners()
{
	_algorithmRunnerList = IAlgorithmRunner::enumerateAlgorithmRunners();
}

void BaseCore::initFileWriter(bool bInit)
{
	if ( bInit )
	{
		const auto processParams = std::dynamic_pointer_cast<ProcessParameters>(_processParameters);
		fileDumpThread().setThreadFunction(frameSaveData, this, coreEventCallback );
		fileDumpThread().setMaxQueueSize(processParams->AsyncSaveQueueMaxSizeGB() * 1024 * 1024 * 1024 );
		fileDumpThread().start();
		CORE_SCOPED_LOG << "File writer started";
	}
	else
	{
		fileDumpThread().stop();
		fileDumpThread().join();
	}
}



//////////////////////////////////////////////////////////////////////////////////
//////////////////////////// CALLBACK PARSERS
////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
////////     PROVIDER SLOTS


void BaseCore::_onProviderScannedFilesCount ( int sourceFileCount )
{
	emit providerScannedFilesCount(sourceFileCount);
}

void BaseCore::_onProviderFrameGeneratedOk ( int frameIndex )
{
	emit providerFrameGeneratedOk (frameIndex);
}

void BaseCore::_onProviderFrameSkipped( int frameIndex )
{
	emit providerFrameSkipped (frameIndex);
}

void BaseCore::_onProviderFinished ()
{
	CORE_SCOPED_LOG << "Frame provider finished image generation";
	// don't stop the threads, because consumer can still work on remaining frames
	// wait for the first skipped handled frame

	// TODO : find better solution for waiting flag
	_waitingForFirstSkippedHandledFrame = true;
	emit providerFinished();
}

void BaseCore::_onProviderFrameImageData (std::shared_ptr<LandaJune::Core::SharedFrameData> frameData)
{
	if (frameData)
		emit providerFrameImageData(frameData);
}

void BaseCore::_onProviderException	(std::exception_ptr pex)
{
	processProviderExceptionData(pex);
}


///////////////////////////////////////////////
////////     RUNNER SLOTS


void BaseCore::_onRunnerFrameHandledOk ( int frameIndex )
{	
	emit runnerFrameHandledOk(frameIndex);
}	
	 
void BaseCore::_onRunnerFrameSkipped ( int frameIndex )
{	 
	if (_waitingForFirstSkippedHandledFrame )
		stop();

	_waitingForFirstSkippedHandledFrame = false;
	emit runnerFrameSkipped(frameIndex); 
}	 
	 
void BaseCore::_onRunnerDetectionSuccess( int frameIndex )
{	 
	 emit runnerDetectionSuccess(frameIndex); 
}	 
	 
void BaseCore::_onRunnerDetectionFailure( int frameIndex )
{	
	emit runnerDetectionFailure(frameIndex); 
}	
	
void BaseCore::_onRunnerException	(std::exception_ptr pex)
{
	processRunnerExceptionData(pex);
}


//////////////////////////////////////////
//////////////// saver events

void BaseCore::_onFileSaverFailure	( int intErr )
{
	emit fileSaverFailure (intErr);
}

void BaseCore::processProviderExceptionData(std::exception_ptr pex)
{
	try
	{
		if (pex)
		{
			std::rethrow_exception(pex);
		}
	}
	catch (BaseException& bex)
	{
		std::ostringstream ss;
		print_exception(bex, ss);

		CORE_SCOPED_ERROR << ss.str().c_str();
	}


	// post message for stopping processing
	stop();
	
	//emit coreException(ex);
}

void BaseCore::processRunnerExceptionData(std::exception_ptr pex)
{
	try
	{
		if (pex)
		{
			std::rethrow_exception(pex);
		}
	}
	catch (BaseException& bex)
	{
		std::ostringstream ss;
		print_exception(bex, ss);

		CORE_SCOPED_ERROR << ss.str().c_str();
	}


	// post message for stopping processing
	stop();
	//emit coreException(ex);
}

void BaseCore::coreEventCallback ( ICore * coreObject, CoreCallbackType callbackType, std::any callbackData  )
{
	// core event callback receives all events from 
	// providers and runners
	// and routes them accordingly to even type
	// also, callbackData is treated differently for different event codes

	const auto pCore = dynamic_cast<BaseCore*>(coreObject);
	if ( pCore == nullptr )
		return;

	auto funcIter = _eventParserMap.find(callbackType);
	if ( funcIter == _eventParserMap.end())
	{
		on_UnknownEvent(pCore, callbackType, callbackData);
		return;
	}

	try
	{
		std::invoke (funcIter->second, pCore, callbackData);
	}
	catch (const std::bad_any_cast& e)
	{
		CORE_SCOPED_ERROR << "Bad type cast occured during processng event of type : " << static_cast<int>(callbackType) << " exception error : " << e.what();
	}
}

void BaseCore::on_UnknownEvent( BaseCore *coreObject, CoreCallbackType callbackType, std::any& callbackData )
{
	CORE_SCOPED_WARNING << "Received event with unknown ( still unmapped ? )  callback type : " << static_cast<int>(callbackType);
}

/////////////////////////////////////////////////////////
// provider callbacks

void BaseCore::on_ProviderScannedFilesCount( BaseCore *coreObject, std::any& callbackData )
{
	QMetaObject::invokeMethod(coreObject, "_onProviderScannedFilesCount", Qt::QueuedConnection, Q_ARG(qint32, static_cast<qint32>(std::any_cast<int>(callbackData))));
}

void BaseCore::on_ProviderFrameGeneratedOk( BaseCore *coreObject, std::any& callbackData )
{
	auto bRes = QMetaObject::invokeMethod(coreObject, "_onProviderFrameGeneratedOk", Qt::QueuedConnection,  Q_ARG(qint32, static_cast<qint32>(std::any_cast<int>(callbackData))));
}

void BaseCore::on_ProviderFrameSkipped( BaseCore *coreObject, std::any& callbackData )
{
	auto bRes = QMetaObject::invokeMethod(coreObject, "_onProviderFrameSkipped",  Q_ARG(qint32, static_cast<qint32>(std::any_cast<int>(callbackData))));
}

void BaseCore::on_ProviderFinished( BaseCore *coreObject, std::any& callbackData )
{
	auto bRes = QMetaObject::invokeMethod(coreObject, "_onProviderFinished", Qt::QueuedConnection );
}

void BaseCore::on_ProviderFrameImageData( BaseCore *coreObject, std::any& callbackData )
{
	auto bRes = QMetaObject::invokeMethod(coreObject, "_onProviderFrameImageData", Qt::QueuedConnection, Q_ARG(std::shared_ptr<LandaJune::Core::SharedFrameData>, std::any_cast<std::shared_ptr<LandaJune::Core::SharedFrameData>>(callbackData)));
}

void BaseCore::on_ProviderException( BaseCore *coreObject, std::any& callbackData )
{
	QMetaObject::invokeMethod(coreObject, "_onProviderException", Qt::QueuedConnection, Q_ARG(std::exception_ptr, std::any_cast<std::exception_ptr>(callbackData)));
}


/////////////////////////////////////////////////////////
// runner callbacks


void BaseCore::on_RunnerFrameHandledOk( BaseCore *coreObject, std::any& callbackData )
{
	QMetaObject::invokeMethod(coreObject, "_onRunnerFrameHandledOk", Qt::QueuedConnection, Q_ARG(qint32, static_cast<qint32>(std::any_cast<int>(callbackData))));
}

void BaseCore::on_RunnerFrameSkipped( BaseCore *coreObject, std::any& callbackData )
{
	QMetaObject::invokeMethod(coreObject, "_onRunnerFrameSkipped", Qt::QueuedConnection, Q_ARG(qint32, static_cast<qint32>(std::any_cast<int>(callbackData))));
}


void BaseCore::on_RunnerDetectionSuccess( BaseCore *coreObject, std::any& callbackData )
{
	QMetaObject::invokeMethod(coreObject, "_onRunnerDetectionSuccess", Qt::QueuedConnection, Q_ARG(qint32, static_cast<qint32>(std::any_cast<int>(callbackData))));
}

void BaseCore::on_RunnerDetectionFailure( BaseCore *coreObject, std::any& callbackData )
{
	QMetaObject::invokeMethod(coreObject, "_onRunnerDetectionFailure", Qt::QueuedConnection, Q_ARG(qint32, static_cast<qint32>(std::any_cast<int>(callbackData))));
}

void BaseCore::on_RunnerException( BaseCore *coreObject, std::any& callbackData )
{
	QMetaObject::invokeMethod(coreObject, "_onRunnerException", Qt::QueuedConnection, Q_ARG(std::exception_ptr, std::any_cast<std::exception_ptr>(callbackData)));
}

void BaseCore::on_SaverError( BaseCore *coreObject, std::any& callbackData )
{
	auto re = QMetaObject::invokeMethod(coreObject, "_onFileSaverFailure", Qt::QueuedConnection, Q_ARG(int, (int)std::any_cast<CORE_ERROR>(callbackData)));
}

