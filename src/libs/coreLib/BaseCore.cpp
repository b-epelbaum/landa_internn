#include "BaseCore.h"
#include "applog.h"

#include "common/june_exceptions.h"
#include "interfaces/IFrameProvider.h"
#include "interfaces/IAlgorithmRunner.h"


//#include "algorithm_wrappers.h"

#include "BackgroundThreadPool.h"
#include "writequeue.h"
#include "functions.h"
#include "ProcessParameters.h"

#include "FrameRefPool.h"

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
	throw BaseException(toInt(CORE_ENGINE_ERROR::ERR_CORE_NOT_INITIALIZED), ""); \
}


void BaseCore::loadDefaultConfiguration()
{
}

void BaseCore::loadConfiguration(QIODevice& strJSONFile)
{
}

void BaseCore::loadConfiguration(QString strJSON)
{
}

void BaseCore::init()
{
	if (_bInited)
	{
		CORE_SCOPED_WARNING << "Core is already initialized";
		return;
	}
	CORE_SCOPED_LOG << "-------------------------------------------------------------";
	CORE_SCOPED_LOG << " Core Initialization started...";

	// init global parameters
	initGlobalParameters();

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

std::shared_ptr<Parameters::BaseParameters> BaseCore::getProcessParameters()
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
			throw BaseException(toInt(CORE_ENGINE_ERROR::ERR_CORE_PROVIDER_IS_BUSY), "");
		}

		_currentFrameProvider->cleanup();
	}

	_currentFrameProvider = std::move(provider);
	_currentFrameProvider->setProviderParameters(_processParameters);
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

	_currentAlgorithmRunner = std::move(algoRunner);
}

AlgorithmRunnerPtr BaseCore::getSelectedAlgorithmRunner() const
{
	CHECK_IF_INITED
	return _currentAlgorithmRunner;
}

void BaseCore::start() const
{
	CHECK_IF_INITED
	if (!_currentFrameProvider)
	{
		throw BaseException(toInt(CORE_ENGINE_ERROR::ERR_CORE_NO_PROVIDER_SELECTED), "");
	}

	if (!_currentAlgorithmRunner)
	{
		throw BaseException(toInt(CORE_ENGINE_ERROR::ERR_CORE_NO_ALGORITHM_RUNNER_SELECTED), "");
	}

	if (_currentFrameProvider->isBusy())
	{
		throw BaseException(toInt(CORE_ENGINE_ERROR::ERR_CORE_PROVIDER_IS_BUSY), "");
	}

	// call algorithms initialization functions

	try
	{
		_currentAlgorithmRunner->init(_processParameters);
	}
	catch(...)
	{
		throw BaseException(toInt(CORE_ENGINE_ERROR::ERR_CORE_ALGO_RUNNER_THROWN_RUNTIME_EXCEPTION), "");
	}


	initFramePool();

	try
	{
		const auto& err = _currentFrameProvider->init();
		if ( err != FRAME_PROVIDER_ERROR::ERR_NO_ERROR )
		{
			throw BaseException(toInt(CORE_ENGINE_ERROR::ERR_CORE_PROVIDER_FAILED_TO_INIT), "");
		}
	}
	catch (...)
	{
		throw BaseException(toInt(CORE_ENGINE_ERROR::ERR_CORE_PROVIDER_THROWN_RUNTIME_EXCEPTION), "");
	}

	// start file writer
	initFileWriter(true);

	// start one and only frame consuming thread
	// the entry point of this thread is the "frameConsume" static function, which uses image processing and file saving thread pools inside
	// see "frameConsume" implementation
	frameConsumerThread().setThreadFunction(frameConsume, _currentAlgorithmRunner );
	frameConsumerThread().start();

	
	// start one and only frame producing/generation/capture thread
	// the entry point of this thread is the "frameGenerate" static function, which invokes the data generation functions of IAbstractImageProvider object
	// see "frameGenerate" implementation
	frameProducerThread().setThreadFunction(frameGenerate, _currentFrameProvider);
	frameProducerThread().setErrorHandler(providerExceptionHandler, (void*)this);
	frameProducerThread().start();
	
}

void BaseCore::stop( int error )
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
	initFileWriter(false);
	CORE_SCOPED_LOG << "File writer stopped";

	_currentFrameProvider->cleanup();
	FrameRefPool::frameRefPool()->cleanup();
	if ( _currentAlgorithmRunner)
		_currentAlgorithmRunner->cleanup();
		
	emit coreStopped(error);
}

bool BaseCore::isBusy()
{
	if (!_bInited)
		return false;
	if (!_currentFrameProvider)
		return false;

	return _currentFrameProvider->isBusy();
}

void BaseCore::onException(int error)
{
	stop(error);
}

QString BaseCore::getDefaultConfigurationFileName() const
{
	return "";
}

void BaseCore::saveConfiguration()
{
}

void BaseCore::initGlobalParameters()
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
		CORE_SCOPED_ERROR << "Error initializing frame pool; error ID : " << e.error();
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
		fileDumpThread().setThreadFunction(frameSaveData);
		fileDumpThread().start();
		CORE_SCOPED_LOG << "File writer started";
	}
	else
	{
		fileDumpThread().stop();
		fileDumpThread().join();
	}
}

void BaseCore::providerExceptionHandler ( void * pThis, BaseException& ex )
{
	auto bRes = QMetaObject::invokeMethod(static_cast<BaseCore*>(pThis), "onException", Qt::QueuedConnection, Q_ARG(int, ex.error()));
}