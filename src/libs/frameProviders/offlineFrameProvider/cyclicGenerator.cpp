#include "cyclicGenerator.h"
#include <thread>

#include "util.h"
#include "frameRef.h"
#include "ProcessParameters.h"


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

#define CYCLIC_GENERATOR_SCOPED_LOG PRINT_INFO4 << "[cyclicGenerator] : "
#define CYCLIC_GENERATOR_SCOPED_ERROR PRINT_ERROR << "[cyclicGenerator] : "
#define CYCLIC_GENERATOR_SCOPED_WARNING PRINT_WARNING << "[cyclicGenerator] : "

cyclicGenerator::cyclicGenerator()
{
	CYCLIC_GENERATOR_SCOPED_LOG << "created";
}

cyclicGenerator::~cyclicGenerator()
{
	CYCLIC_GENERATOR_SCOPED_LOG << "destroyed";
}



void cyclicGenerator::init(BaseParametersPtr parameters, Core::ICore * coreObject, CoreEventCallback callback)
{
	try
	{
		validateParameters(parameters);
		connect (parameters.get(), &BaseParameters::updateCalculated, this, &BaseFrameProvider::onUpdateParameters);

		_dataCallback = callback;
		_coreObject = coreObject;

		_lastAcquiredImage = -1;
		_sourceTemplateImage.release();

		if (_SourceFilePath.isEmpty() )
		{
			CYCLIC_GENERATOR_SCOPED_ERROR << "Source file path is empty";
			THROW_EX_ERR_STR(CORE_ERROR::ERR_OFFLINEREADER_SOURCE_FILE_INVALID, "Source file path is empty" );
		}

		const auto t1 = Utility::now_in_millisecond();
		const auto pathName = _SourceFilePath;
		CYCLIC_GENERATOR_SCOPED_LOG << "found source image : " << pathName << "; loading...";
		_sourceTemplateImage = cv::imread(pathName.toStdString());

		if (_sourceTemplateImage.empty() )
		{
			CYCLIC_GENERATOR_SCOPED_ERROR << "cannot load image file from " << pathName;
			THROW_EX_ERR_STR(CORE_ERROR::ERR_OFFLINEREADER_SOURCE_FILE_INVALID, "Cannot load source image from " + pathName.toStdString() );
		}
		CYCLIC_GENERATOR_SCOPED_LOG << "finished loading file " << pathName << " in " << Utility::now_in_millisecond() - t1 << " msec";
	}
	catch(BaseException& bex)
	{
		throw;
	}
	catch ( std::exception& ex )
	{
		RETHROW( CORE_ERROR::ERR_PROVIDER_FAILED_TO_INIT);
	}
}

void cyclicGenerator::validateParameters(BaseParametersPtr parameters)
{
	// TODO : query BaseParameters for named parameters
	// currently hardcoded

	const auto processParams = std::dynamic_pointer_cast<ProcessParameters>(parameters);

	_SourceFilePath = processParams->SourceFilePath();
	_FrameFrequencyInMSec = processParams->FrameFrequencyInMSec();
	_ImageMaxCount = processParams->ImageMaxCount();

	CYCLIC_GENERATOR_SCOPED_LOG << "Validating provider parameters : ";
	CYCLIC_GENERATOR_SCOPED_LOG << "---------------------------------------";
	CYCLIC_GENERATOR_SCOPED_LOG << "_SourceFilePath = " << _SourceFilePath;
	CYCLIC_GENERATOR_SCOPED_LOG << "_FrameFrequencyInMSec = " << _FrameFrequencyInMSec;

	_providerParameters = parameters;
}

void cyclicGenerator::cleanup()
{
	try
	{
		disconnect (_providerParameters.get(), &BaseParameters::updateCalculated, this, &BaseFrameProvider::onUpdateParameters);
		_sourceTemplateImage.release();

		_coreObject = nullptr;
		_dataCallback = nullptr;
		_providerParameters.reset();

		CYCLIC_GENERATOR_SCOPED_LOG << "cleaned up";
	}
	catch(BaseException& bex)
	{
		throw;
	}
	catch ( std::exception& ex )
	{
		RETHROW( CORE_ERROR::ERR_PROVIDER_FAILED_TO_INIT);
	}
}


int32_t cyclicGenerator::getFrameLifeSpan() const
{
	const auto processParams = std::dynamic_pointer_cast<ProcessParameters>(_providerParameters);
	return processParams->FrameFrequencyInMSec();
}

CORE_ERROR cyclicGenerator::prepareData(FrameRef* frameRef)
{
	if ( _sourceTemplateImage.empty() )
		return CORE_ERROR::ERR_SIMULATOR_HAVE_NO_IMAGES;

	if (_ImageMaxCount > 0 &&  _lastAcquiredImage == _ImageMaxCount  - 1)
		return CORE_ERROR::ERR_SIMULATOR_REACHED_MAX_COUNT;

	std::this_thread::sleep_for(std::chrono::milliseconds(_FrameFrequencyInMSec));
	return RESULT_OK;
}

CORE_ERROR cyclicGenerator::accessData(FrameRef* frameRef)
{
	const auto clonedMat = std::make_shared<cv::Mat>(_sourceTemplateImage.size(), _sourceTemplateImage.type());
	_sourceTemplateImage.copyTo(*clonedMat);

	if (!clonedMat->data)            // Check for invalid input
	{
		CYCLIC_GENERATOR_SCOPED_WARNING << "Cannot clone loaded image ";
		return CORE_ERROR::ERR_OFFLINEREADER_SOURCE_FILE_INVALID;
	}

	// push bits to frameRef object
	frameRef->setBits(++_lastAcquiredImage, clonedMat, false);

	// this flag tells the algorithm runner to perform
	// image/CSV saving synchronously 
	// to avoid save queue growing constantly
	// for offline analysis it's not critical to perform saving synchronously

	// TODO : replace this function/member to derivative of offline/online generated bits ?
	frameRef->setAsyncWrite(true);

	// pass shared object to frame to increase reference counter
	//frameRef->setSharedData(clonedMat);
	CYCLIC_GENERATOR_SCOPED_LOG << "Received frame #" << _lastAcquiredImage;
	return RESULT_OK;
}

void cyclicGenerator::releaseData(FrameRef* frameRef)
{
}
