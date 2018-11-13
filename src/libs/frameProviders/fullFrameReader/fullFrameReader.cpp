#include "offlineFrameProvider.h"
#include <QDirIterator>
#include <thread>

#include "util.h"
#include "frameRef.h"
#include "ProcessParameters.h"

#include "cyclicGenerator.h"
#include "folderReader.h"


#ifdef _DEBUG
#pragma comment(lib, "opencv_world342d.lib")
#else
#pragma comment(lib, "opencv_world342.lib")
#endif

using namespace LandaJune;
using namespace FrameProviders;
using namespace Helpers;
using namespace Core;
using namespace Parameters;

static const QString OFFLINE_GENERATOR_PROVIDER_NAME = "Offline Frame Provider";
static const QString OFFLINE_GENERATOR_DESC = "Offline Frame Provider performs frame generation based on image files. It can work in two modes : cyclic image generation and reading all images from specified folder";


#define OFFLINE_GENERATORSCOPED_LOG PRINT_INFO7 << "[offlineFrameProvider] : "
#define OFFLINE_GENERATOR_SCOPED_ERROR PRINT_ERROR << "[offlineFrameProvider] : "
#define OFFLINE_GENERATOR_SCOPED_WARNING PRINT_WARNING << "[offlineFrameProvider] : "


offlineFrameProvider::offlineFrameProvider()
{
	_name = OFFLINE_GENERATOR_PROVIDER_NAME;
	_description = OFFLINE_GENERATOR_DESC;
	OFFLINE_GENERATORSCOPED_LOG << "created";
}

offlineFrameProvider::~offlineFrameProvider()
{
	OFFLINE_GENERATORSCOPED_LOG << "destroyed";
}

int offlineFrameProvider::getRecommendedFramePoolSize()
{
	CHECK_INIT

	return _currentOfflineProvider->getRecommendedFramePoolSize();
}

bool offlineFrameProvider::shouldReportSkippedFrame() const
{
	CHECK_INIT

	return _currentOfflineProvider->shouldReportSkippedFrame();
}

CORE_ERROR offlineFrameProvider::prepareData(FrameRef* frameRef)
{
	CHECK_INIT

	return _currentOfflineProvider->prepareData(frameRef);
}

CORE_ERROR offlineFrameProvider::accessData(FrameRef* frameRef)
{
	CHECK_INIT

	return _currentOfflineProvider->accessData(frameRef);
}

void offlineFrameProvider::releaseData(FrameRef* frameRef)
{
	if (_coreObject )
	{
		if (_currentOfflineProvider)
			_currentOfflineProvider->releaseData(frameRef);
	}
}

void offlineFrameProvider::init(BaseParametersPtr parameters, Core::ICore * coreObject, CoreEventCallback callback)
{
	try
	{
		validateParameters(parameters);
		
		_dataCallback = callback;
		_coreObject = coreObject;

		if (_CyclicGeneration)
		{
			_currentOfflineProvider.reset(new cyclicGenerator);
		}
		else
		{
			_currentOfflineProvider.reset(new folderReader);
		}

		return _currentOfflineProvider->init(parameters, coreObject, callback);
	}
	catch(BaseException& bex)
	{
		throw;
	}
	catch ( std::exception& ex)
	{
		RETHROW( CORE_ERROR::ERR_PROVIDER_FAILED_TO_INIT);
	}
}

void offlineFrameProvider::validateParameters(BaseParametersPtr parameters)
{
	// TODO : query BaseParameters for named parameters
	// currently hardcoded

	const auto processParams = std::dynamic_pointer_cast<ProcessParameters>(parameters);
	_SourceFolderPath = processParams->SourceFolderPath();
	_SourceFilePath = processParams->SourceFilePath();
	_ImageMaxCount = processParams->ImageMaxCount();
	_CyclicGeneration = processParams->CycleImage();
	_FrameFrequencyInMSec = processParams->FrameFrequencyInMSec();

	OFFLINE_GENERATORSCOPED_LOG << "Validating provider parameters : ";
	OFFLINE_GENERATORSCOPED_LOG << "---------------------------------------";
	OFFLINE_GENERATORSCOPED_LOG << "_SourceFolderPath = " << _SourceFolderPath;
	OFFLINE_GENERATORSCOPED_LOG << "_SourceFilePath = " << _SourceFilePath;
	OFFLINE_GENERATORSCOPED_LOG << "_ImageMaxCount = " << _ImageMaxCount;
	OFFLINE_GENERATORSCOPED_LOG << "_CyclicGeneration = " << _CyclicGeneration;
	OFFLINE_GENERATORSCOPED_LOG << "_FrameFrequencyInMSec = " << _FrameFrequencyInMSec;

	_providerParameters = parameters;

	if(_currentOfflineProvider)
		_currentOfflineProvider->validateParameters(parameters);
}

void offlineFrameProvider::cleanup()
{
	try
	{
		if (_currentOfflineProvider	)
		{
			_currentOfflineProvider->cleanup();
			_currentOfflineProvider.reset();
		}

		_coreObject = nullptr;
		_dataCallback = nullptr;
		_providerParameters.reset();
	}
	catch(BaseException& bex)
	{
		throw;
	}
	catch ( std::exception& ex)
	{
		RETHROW( CORE_ERROR::ERR_PROVIDER_CLEANUP_FAILED);
	}
}

int32_t offlineFrameProvider::getFrameLifeSpan() const
{
	CHECK_INIT

	return  _currentOfflineProvider ? _currentOfflineProvider->getFrameLifeSpan() : -1;
}
