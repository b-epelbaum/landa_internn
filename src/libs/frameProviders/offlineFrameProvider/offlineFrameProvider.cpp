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

bool offlineFrameProvider::canContinue(FRAME_PROVIDER_ERROR lastError)
{
	return _currentOfflineProvider ? _currentOfflineProvider->canContinue(lastError) : false;
}

FRAME_PROVIDER_ERROR offlineFrameProvider::prepareData(FrameRef* frameRef)
{
	return _currentOfflineProvider ? _currentOfflineProvider->prepareData(frameRef) : FRAME_PROVIDER_ERROR::ERR_PROVIDER_INVALID_SELECTED_PROVIDER;
}

FRAME_PROVIDER_ERROR offlineFrameProvider::accessData(FrameRef* frameRef)
{
	return _currentOfflineProvider ? _currentOfflineProvider->accessData(frameRef) : FRAME_PROVIDER_ERROR::ERR_PROVIDER_INVALID_SELECTED_PROVIDER;
}

void offlineFrameProvider::releaseData(FrameRef* frameRef)
{
	if (_currentOfflineProvider)
		_currentOfflineProvider->releaseData(frameRef);
}

FRAME_PROVIDER_ERROR offlineFrameProvider::init()
{
	if (_CyclicGeneration)
	{
		_currentOfflineProvider.reset(new cyclicGenerator);
	}
	else
	{
		_currentOfflineProvider.reset(new folderReader);
	}

	_currentOfflineProvider->validateParameters(_providerParameters);
	return _currentOfflineProvider->init();
}

void offlineFrameProvider::validateParameters(std::shared_ptr<BaseParameters> parameters)
{
	// TODO : query BaseParameters for named parameters
	// currently hardcoded
	
	const auto _processParameters = std::dynamic_pointer_cast<ProcessParameters>(parameters);


	_SourceFolderPath = _processParameters->SourceFolderPath();
	_SourceFilePath = _processParameters->SourceFilePath();
	_ImageMaxCount = _processParameters->ImageMaxCount();
	_CyclicGeneration = _processParameters->CycleImage();
	_FrameFrequencyInMSec = _processParameters->FrameFrequencyInMSec();

	OFFLINE_GENERATORSCOPED_LOG << "Validating provider parameters : ";
	OFFLINE_GENERATORSCOPED_LOG << "---------------------------------------";
	OFFLINE_GENERATORSCOPED_LOG << "_SourceFolderPath = " << _SourceFolderPath;
	OFFLINE_GENERATORSCOPED_LOG << "_SourceFilePath = " << _SourceFilePath;
	OFFLINE_GENERATORSCOPED_LOG << "_ImageMaxCount = " << _ImageMaxCount;
	OFFLINE_GENERATORSCOPED_LOG << "_CyclicGeneration = " << _CyclicGeneration;
	OFFLINE_GENERATORSCOPED_LOG << "_FrameFrequencyInMSec = " << _FrameFrequencyInMSec;

	if(_currentOfflineProvider)
		_currentOfflineProvider->validateParameters(parameters);
}

FRAME_PROVIDER_ERROR offlineFrameProvider::cleanup()
{
	auto retVal =  _currentOfflineProvider ? _currentOfflineProvider->cleanup() : FRAME_PROVIDER_ERROR::ERR_PROVIDER_INVALID_SELECTED_PROVIDER;
	_currentOfflineProvider.reset();
	return  retVal;
}