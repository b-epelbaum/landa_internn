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

bool cyclicGenerator::canContinue(CORE_ERROR lastError)
{
	return false;
}

CORE_ERROR cyclicGenerator::prepareData(FrameRef* frameRef)
{
	if ( _sourceImage.empty() )
		return CORE_ERROR::ERR_SIMULATOR_HAVE_NO_IMAGES;

	if (_ImageMaxCount > 0 &&  _lastAcquiredImage >= _ImageMaxCount)
		return CORE_ERROR::ERR_SIMULATOR_REACHED_MAX_COUNT;

	std::this_thread::sleep_for(std::chrono::milliseconds(_FrameFrequencyInMSec));
	return RESULT_OK;
}

CORE_ERROR cyclicGenerator::accessData(FrameRef* frameRef)
{
	const auto w = _sourceImage.cols;
	const auto h = _sourceImage.rows;
	const auto s = _sourceImage.step[0] * _sourceImage.rows;

	frameRef->setBits(++_lastAcquiredImage, w, h, s, _sourceImage.data);
	CYCLIC_GENERATOR_SCOPED_LOG << "Received frame #" << _lastAcquiredImage;
	return RESULT_OK;
}

void cyclicGenerator::releaseData(FrameRef* frameRef)
{
}

CORE_ERROR cyclicGenerator::init()
{
	_lastAcquiredImage = -1;
	_sourceImage.release();

	const auto t1 = Utility::now_in_millisecond();
	const auto pathName = _SourceFilePath;
	CYCLIC_GENERATOR_SCOPED_LOG << "found BMP image : " << pathName << "; loading...";
	_sourceImage = cv::imread(pathName.toStdString());

	if (_sourceImage.empty() )
	{
		CYCLIC_GENERATOR_SCOPED_ERROR << "cannot load image file from " << pathName;
		return CORE_ERROR::ERR_OFFLINEREADER_SOURCE_FILE_INVALID;
	}
	else
		CYCLIC_GENERATOR_SCOPED_LOG << "finished loading file " << pathName << " in " << Utility::now_in_millisecond() - t1 << " msec";
	
	return RESULT_OK;
}

void cyclicGenerator::validateParameters(std::shared_ptr<BaseParameters> parameters)
{
	// TODO : query BaseParameters for named parameters
	// currently hardcoded

	const auto _processParameters = std::dynamic_pointer_cast<ProcessParameters>(parameters);
	_SourceFilePath = _processParameters->SourceFilePath();
	_FrameFrequencyInMSec = _processParameters->FrameFrequencyInMSec();
	_ImageMaxCount = _processParameters->ImageMaxCount();

	CYCLIC_GENERATOR_SCOPED_LOG << "Validating provider parameters : ";
	CYCLIC_GENERATOR_SCOPED_LOG << "---------------------------------------";
	CYCLIC_GENERATOR_SCOPED_LOG << "_SourceFilePath = " << _SourceFilePath;
	CYCLIC_GENERATOR_SCOPED_LOG << "_FrameFrequencyInMSec = " << _FrameFrequencyInMSec;
}

CORE_ERROR cyclicGenerator::cleanup()
{
	_sourceImage.release();
	CYCLIC_GENERATOR_SCOPED_LOG << "cleaned up";
	return RESULT_OK;
}