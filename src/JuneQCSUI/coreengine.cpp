#include "coreengine.h"

#include "BackgroundThreadPool.h"
#include "FrameRefPool.h"

#include "frameGenerate.hpp"
#include "frameConsume.hpp"

#include "applog.h"
#include "common/june_exceptions.h"

#include "algorithm_wrappers.h"

using namespace LandaJune::Core;
using namespace LandaJune::Parameters;
using namespace LandaJune::Threading;
using namespace LandaJune::FrameProviders;
using namespace LandaJune::Functions;
using namespace LandaJune::Loggers;

#define CORE_SCOPED_LOG PRINT_INFO6 << "[CoreEngine] : "
#define CORE_SCOPED_WARNING PRINT_WARNING << "[CoreEngine] : "
#define CORE_SCOPED_ERROR PRINT_ERROR << "[CoreEngine] : "

#define CHECK_IF_INITED if (!_bInited ) \
{ \
	throw CoreEngineException(CORE_ENGINE_ERROR::ERR_CORE_NOT_INITIALIZED, ""); \
}

CoreEngine* CoreEngine::get()
{
	if (!_this)
		_this = new CoreEngine();
	return _this;
}

void CoreEngine::init()
{
	if ( _bInited )
	{
		CORE_SCOPED_WARNING << "Core is already initialized";
		return;
	}
	CORE_SCOPED_LOG << "-------------------------------------------------------------";
	CORE_SCOPED_LOG << " Core Initialization started...";

	// init algo functions

	initGlobalParameters();
	initProviders();
	_bInited = true;
	CORE_SCOPED_LOG << " Core Initialization finished...";
}

void CoreEngine::cleanup()
{
	_bInited = false;
}

const QList<IFrameProvider::FrameProviderPtr>& CoreEngine::getFrameProviderList() const
{
	CHECK_IF_INITED
	return _providerList;
}

std::shared_ptr<ProcessParameter> CoreEngine::getBatchParameters() const
{
	CHECK_IF_INITED

	return _commonBatchParameters;
}

void CoreEngine::selectFrameProvider(IFrameProvider::FrameProviderPtr provider)
{
	CHECK_IF_INITED

	if (_currentFrameProvider == provider)
		return;

	if ( _currentFrameProvider )
	{
		if ( _currentFrameProvider->isBusy() )
		{
			throw CoreEngineException(CORE_ENGINE_ERROR::ERR_CORE_PROVIDER_IS_BUSY, "");
		}

		_currentFrameProvider->clean();
	}
	_currentFrameProvider = std::move(provider);
}

IFrameProvider::FrameProviderPtr CoreEngine::getSelectedFrameProvider() const
{
	CHECK_IF_INITED
	return _currentFrameProvider;
}

void CoreEngine::start() const
{
	CHECK_IF_INITED
	if (!_currentFrameProvider )
	{
		throw CoreEngineException(CORE_ENGINE_ERROR::ERR_CORE_NO_PROVIDER_SELECTED, "");
	}

	if (_currentFrameProvider->isBusy())
	{
		throw CoreEngineException(CORE_ENGINE_ERROR::ERR_CORE_PROVIDER_IS_BUSY, "");
	}

	// call algorithms initialization functions
	initAlgorithmsData(_commonBatchParameters);

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

void CoreEngine::stop() const
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

void CoreEngine::initGlobalParameters()
{
	_commonBatchParameters = std::make_shared<ProcessParameter>();
}

void CoreEngine::initFramePool() const
{
	const FrameRef::GLOBAL_FRAME_DATA frameData{ _commonBatchParameters };
	try
	{
		FrameRefPool::frameRefPool()->init(_currentFrameProvider->getRecommendedFramePoolSize(), frameData);
	}
	catch (FrameRefException& e)
	{
		CORE_SCOPED_ERROR << "Error initializing frame pool; error ID : " << toInt(e.error());
	}
}

void CoreEngine::initProviders()
{
	_providerList = IFrameProvider::enumerateImageProviders();
}

CoreEngine::CoreEngine(QObject *parent)
	: QObject(parent)
{
}

CoreEngine::~CoreEngine()
{
}
