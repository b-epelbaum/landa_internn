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


void BaseCore::init(bool reportEvents )
{
	_reportEvents = reportEvents;

	qRegisterMetaType<BaseException>("BaseException");
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
	
	processParametersOnce->setImageMaxCount(1);
	processParametersOnce->setRootOutputFolder(QString::fromStdString(getRootFolderForOneRun()));

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

	
	_bCanAcceptEvents = true;
	_bCanAcceptExceptions = true;

	// call algorithms initialization functions
	try
	{
		_currentAlgorithmRunner->init(params, this, consumerDataCallback);
	}
	catch(...)
	{
		// TODO : rethrow nested exception
		THROW_EX_INT(CORE_ERROR::ERR_CORE_ALGO_RUNNER_THROWN_RUNTIME_EXCEPTION);
	}

	initFramePool();

	CORE_ERROR initErr;
	try
	{
		initErr = _currentFrameProvider->init(params, this, providerDataCallback);
	}
	catch (...)
	{
		// TODO : rethrow nested exception
		THROW_EX_INT(CORE_ERROR::ERR_CORE_ALGO_RUNNER_THROWN_RUNTIME_EXCEPTION);
	}

	if ( initErr != NO_ERROR )
	{
		THROW_EX_INT(CORE_ERROR::ERR_CORE_PROVIDER_FAILED_TO_INIT);
	}

	// start file writer
	initFileWriter(true);

	// start one and only frame consuming thread
	// the entry point of this thread is the "frameConsume" static function, which uses image processing and file saving thread pools inside
	// see "frameConsume" implementation
	frameConsumerThread().setThreadFunction(frameConsume, _currentAlgorithmRunner, this, consumerDataCallback );
	frameConsumerThread().setErrorHandler(coreExceptionHandler, this);
	frameConsumerThread().start();

	
	// start one and only frame producing/generation/capture thread
	// the entry point of this thread is the "frameGenerate" static function, which invokes the data generation functions of IAbstractImageProvider object
	// see "frameGenerate" implementation
	frameProducerThread().setThreadFunction(frameGenerate, _currentFrameProvider, this, providerDataCallback);
	frameProducerThread().setErrorHandler(coreExceptionHandler, this);
	frameProducerThread().start();
}

void BaseCore::stop()
{
	CHECK_IF_INITED
	if (!_currentFrameProvider)
	{
		return;
	}

	_bCanAcceptEvents = false;
	_bCanAcceptExceptions = false;

	CORE_SCOPED_LOG << "Stopping Frame producer thread...";
	frameProducerThread().stop();
	CORE_SCOPED_LOG << "Stopping Frame consumer thread...";
	frameConsumerThread().stop();

	frameProducerThread().join();
	CORE_SCOPED_LOG << "Frame producer thread finished";
	frameConsumerThread().join();
	CORE_SCOPED_LOG << "Frame runner thread finished";

	// stop file writer
	initFileWriter(false);
	CORE_SCOPED_LOG << "File writer stopped";

	if ( _currentFrameProvider)
		_currentFrameProvider->cleanup();
	FrameRefPool::frameRefPool()->cleanup();
	
	if ( _currentAlgorithmRunner)
		_currentAlgorithmRunner->cleanup();

	emit coreStopped();
}

std::string BaseCore::getRootFolderForOneRun() const
{
	auto tempRoot = QStandardPaths::standardLocations(QStandardPaths::TempLocation)[0];

	tempRoot += "/Landa/OneRun/";

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
			RETHROW (CORE_ERROR::ERR_CORE_CANNOT_CREATE_FOLDER, "Cannot create folder " + stdPath);
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

void BaseCore::onCriticalException(BaseException ex)
{
	stop();
	emit coreException(ex);
}

void BaseCore::onFrameData (std::shared_ptr<LandaJune::Core::SharedFrameData> fData)
{
	emit frameData(fData);
}

void BaseCore::onFileCountData ( int sourceFileCount )
{
	emit offlineFileSourceCount(sourceFileCount);
}

void BaseCore::onFrameProcessed ( int frameIndex )
{
	emit frameProcessed(frameIndex);
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

void BaseCore::initFileWriter(bool bInit) const
{
	if ( bInit )
	{
		const auto processParams = std::dynamic_pointer_cast<ProcessParameters>(_processParameters);
		fileDumpThread().setThreadFunction(frameSaveData);
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

void BaseCore::processExceptionData(BaseException& ex)
{
	{
		autolock lock(_mutex);
		if ( !_bCanAcceptExceptions )
			return;
	}

	std::ostringstream ss;
	print_exception(ex, ss);
	CORE_SCOPED_ERROR << ss.str().c_str();

	//TODO : we should analyze exception information to decide whether we have to stop
	// processing or continue 

	bool bShouldStop = true;

	if (bShouldStop)
	{
		_bCanAcceptExceptions = false;
		_bCanAcceptEvents = false;
		// post message for stopping processing
		auto bRes = QMetaObject::invokeMethod(this, "onCriticalException", Qt::QueuedConnection, Q_ARG(BaseException, ex));
	}
}

void BaseCore::coreExceptionHandler ( ICore * coreObject, BaseException& ex ) noexcept
{
	const auto pCore = dynamic_cast<BaseCore*>(coreObject);
	if ( pCore == nullptr )
		return;

	pCore->processExceptionData(ex);
}

void BaseCore::providerDataCallback ( ICore * coreObject, FrameProviderDataCallbackType callbackType, std::any callbackData  )
{
	const auto pCore = dynamic_cast<BaseCore*>(coreObject);
	if ( pCore == nullptr )
		return;

	{
		autolock lock(pCore->_mutex);
		if ( !pCore->_bCanAcceptEvents )
			return;
	}

	std::string typeName = "unknown";
	try
	{
		switch (callbackType)
		{
			case FrameProviderDataCallbackType::CALLBACK_FRAME_DATA :
			{
				typeName = "FrameProviderDataCallbackType::CALLBACK_FRAME_DATA";
				if (pCore->_reportEvents )
					auto bRes = QMetaObject::invokeMethod(pCore, "onFrameData", Qt::QueuedConnection, Q_ARG(std::shared_ptr<LandaJune::Core::SharedFrameData>, std::any_cast<std::shared_ptr<LandaJune::Core::SharedFrameData>>(callbackData)));
			}
			break;

			case FrameProviderDataCallbackType::CALLBACK_SCANNED_FILES_COUNT :
			{
				typeName = "FrameProviderDataCallbackType::CALLBACK_SCANNED_FILES_COUNT";
				if (pCore->_reportEvents )
					auto bRes = QMetaObject::invokeMethod(pCore, "onFileCountData", Qt::QueuedConnection, Q_ARG(qint32, (qint32)std::any_cast<int>(callbackData)));
			}
			break;
			default: ;
		}
	}
	catch (const std::bad_any_cast& e)
	{
		CORE_SCOPED_ERROR << "Provider callback type : " << typeName.c_str() << " bad cast exception : " << e.what();
	}
}


void BaseCore::consumerDataCallback ( ICore * coreObject, FrameConsumerDataCallbackType callbackType, std::any callbackData  )
{
	const auto pCore = dynamic_cast<BaseCore*>(coreObject);
	if ( pCore == nullptr )
		return;

	{
		autolock lock(pCore->_mutex);
		if ( !pCore->_bCanAcceptEvents )
			return;
	}

	std::string typeName = "unknown";
	try
	{
		switch (callbackType)
		{
			case FrameConsumerDataCallbackType::CALLBACK_FRAME_HANDLED :
			{
				typeName = "FrameProviderDataCallbackType::CALLBACK_FRAME_HANDLED";
				if (pCore->_reportEvents )
					auto bRes = QMetaObject::invokeMethod(pCore, "onFrameProcessed", Qt::QueuedConnection, Q_ARG(qint32, static_cast<qint32>(std::any_cast<int>(callbackData))));
			}
			break;
			default: ;
		}
	}
	catch (const std::bad_any_cast& e)
	{
		CORE_SCOPED_ERROR << "Consumer callback type : " << typeName.c_str() << " bad cast exception : " << e.what();
	}
}