#include "BaseCore.h"
#include "applog.h"

#include "common/june_exceptions.h"
#include "interfaces/IFrameProvider.h"


#include "algorithm_wrappers.h"
#include "TaskThreadPool.h"
#include "BackgroundThreadPool.h"
#include "functions.h"
#include "ProcessParameter.h"

#include "FrameRefPool.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>


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
	throw CoreEngineException(CORE_ENGINE_ERROR::ERR_CORE_NOT_INITIALIZED, ""); \
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

ProcessParameterPtr BaseCore::getProcessParameters()
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
			throw CoreEngineException(CORE_ENGINE_ERROR::ERR_CORE_PROVIDER_IS_BUSY, "");
		}

		_currentFrameProvider->clean();
	}
	_currentFrameProvider = std::move(provider);
}

FrameProviderPtr BaseCore::getSelectedFrameProvider() const
{
	CHECK_IF_INITED
	return _currentFrameProvider;
}

void BaseCore::start() const
{
	CHECK_IF_INITED
	if (!_currentFrameProvider)
	{
		throw CoreEngineException(CORE_ENGINE_ERROR::ERR_CORE_NO_PROVIDER_SELECTED, "");
	}

	if (_currentFrameProvider->isBusy())
	{
		throw CoreEngineException(CORE_ENGINE_ERROR::ERR_CORE_PROVIDER_IS_BUSY, "");
	}

	// call algorithms initialization functions
	initAlgorithmsData(_processParameters);

	initFramePool();
	_currentFrameProvider->init();

	// start two thread pools
	// one for frame processing
	(void)TaskThreadPools::algorithmsThreadPool();

	// and another one for saving bitmaps or any other files
	(void)TaskThreadPools::diskDumperThreadPool();


	// start one and only frame consunimg thread
	// the entry point of this thread is the "frameConsume" static function, which uses image processing and file saving thread pools inside
	// see "frameConsume" implementation
	frameConsumerThread().setThreadFunction(frameConsume);
	frameConsumerThread().start();

	// start one and only frame producing/generation/capture thread
	// the entry point of this thread is the "frameGenerate" static function, which invokes the data generation functions of IAbstractImageProvider object
	// see "frameGenerate" implementation
	frameProducerThread().setThreadFunction(frameGenerate, _currentFrameProvider);
	frameProducerThread().start();
}

void BaseCore::stop() const
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
	CORE_SCOPED_LOG << "Frame handler thread finished";

	_currentFrameProvider->clean();
	FrameRefPool::frameRefPool()->clear();
	clearAlgorithmsData();
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

void BaseCore::initGlobalParameters()
{
	_processParameters = std::make_shared<ProcessParameter>();
}

void BaseCore::initFramePool() const
{
	const FrameRef::GLOBAL_FRAME_DATA frameData{ _processParameters };
	
	try
	{
		FrameRefPool::frameRefPool()->init(_currentFrameProvider->getRecommendedFramePoolSize(), frameData);
	}
	catch (FrameRefException& e)
	{
		CORE_SCOPED_ERROR << "Error initializing frame pool; error ID : " << toInt(e.error());
	}
}

void BaseCore::initProviders()
{
	_providerList = IFrameProvider::enumerateImageProviders();
}
