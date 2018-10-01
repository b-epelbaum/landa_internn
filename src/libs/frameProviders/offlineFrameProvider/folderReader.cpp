#include "folderReader.h"
#include <QDirIterator>

#include "util.h"
#include "frameRef.h"

#include "ProcessParameters.h"

#include <thread>
#include <opencv2/imgcodecs.hpp>

#ifdef _DEBUG
#pragma comment(lib, "opencv_world342d.lib")
#else
#pragma comment(lib, "opencv_world342.lib")
#endif


#define FOLDER_READER_PROVIDER_SCOPED_LOG PRINT_INFO2 << "[folderReader] : "
#define FOLDER_READER_PROVIDER_SCOPED_ERROR PRINT_ERROR << "[folderReader] : "
#define FOLDER_READER_PROVIDER_SCOPED_WARNING PRINT_WARNING << "[folderReader] : "

using namespace LandaJune;
using namespace Core;
using namespace Parameters;
using namespace FrameProviders;
using namespace Helpers;

folderReader::folderReader()
{
	FOLDER_READER_PROVIDER_SCOPED_LOG << "created";
}

folderReader::~folderReader()
{
	FOLDER_READER_PROVIDER_SCOPED_LOG << "destroyed";
}

CORE_ERROR folderReader::init(std::shared_ptr<BaseParameters> parameters)
{
	validateParameters(parameters);
	connect (parameters.get(), &BaseParameters::updateCalculated, this, &BaseFrameProvider::onUpdateParameters);
		
	_lastAcquiredImage = -1;
	_imagePaths.clear();

	const auto imageFolder = _SourceFolderPath; 
	if ( !QFileInfo(imageFolder).exists())
	{
		return CORE_ERROR::ERR_OFFLINEREADER_SOURCE_FOLDER_INVALID;
	}

	QDirIterator it(imageFolder, QStringList() << "*.bmp" << "*.BMP", QDir::Files, QDirIterator::Subdirectories);

	while (it.hasNext()) {
		_imagePaths.push_back(it.next());
	}
	return RESULT_OK;
}

CORE_ERROR folderReader::cleanup()
{
	disconnect (_providerParameters.get(), &BaseParameters::updateCalculated, this, &BaseFrameProvider::onUpdateParameters);
	_imagePaths.clear();
	FOLDER_READER_PROVIDER_SCOPED_LOG << "cleaned up";
	return RESULT_OK;
}


void folderReader::validateParameters(std::shared_ptr<BaseParameters> parameters)
{
	// TODO : query BaseParameters for named parameters
	// currently hardcoded

	const auto processParams = std::dynamic_pointer_cast<ProcessParameters>(parameters);

	_SourceFolderPath = processParams->SourceFolderPath();
	_ImageMaxCount = processParams->ImageMaxCount();
		
	FOLDER_READER_PROVIDER_SCOPED_LOG << "Validating provider parameters : ";
	FOLDER_READER_PROVIDER_SCOPED_LOG << "---------------------------------------";
	FOLDER_READER_PROVIDER_SCOPED_LOG << "_SourceFolderPath = " << _SourceFolderPath;
	FOLDER_READER_PROVIDER_SCOPED_LOG << "_ImageMaxCount = " << _ImageMaxCount;

	_providerParameters = parameters;
}

bool folderReader::canContinue(CORE_ERROR lastError)
{
	auto _canContinue = true;
	switch (lastError) 
	{ 
		case CORE_ERROR::ERR_OFFLINEREADER_NO_MORE_FILES:
		case CORE_ERROR::ERR_SIMULATOR_REACHED_MAX_COUNT:
			_canContinue = false;  
		break;
		default: 
		;
	}
	return _canContinue;
}

int32_t folderReader::getFrameLifeSpan() const
{
	const auto processParams = std::dynamic_pointer_cast<ProcessParameters>(_providerParameters);
	return processParams->FrameFrequencyInMSec();
}

CORE_ERROR folderReader::prepareData(FrameRef* frameRef)
{
	if (_imagePaths.empty())
	{
		FOLDER_READER_PROVIDER_SCOPED_LOG << "No more files to handle. Exiting...";
		return CORE_ERROR::ERR_OFFLINEREADER_NO_MORE_FILES;
	}

	if (_ImageMaxCount > 0 &&  _lastAcquiredImage >= _ImageMaxCount)
		return CORE_ERROR::ERR_SIMULATOR_REACHED_MAX_COUNT;


	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	return RESULT_OK;
}

CORE_ERROR folderReader::accessData(FrameRef* frameRef)
{
	// read image to cv::Mat object
	const auto srcFullPath = _imagePaths.first();
	FOLDER_READER_PROVIDER_SCOPED_LOG << "loading source file " << srcFullPath << "...";
	_imagePaths.pop_front();

	const auto& stdPath = srcFullPath.toStdString();

	const auto tempMatObject = std::make_shared<cv::Mat>(cv::imread(stdPath));
	if (!tempMatObject->data)            // Check for invalid input
	{
		FOLDER_READER_PROVIDER_SCOPED_WARNING << "Cannot load image " << srcFullPath;
		return CORE_ERROR::ERR_OFFLINEREADER_SOURCE_FILE_INVALID;
	}

	FOLDER_READER_PROVIDER_SCOPED_LOG << "Image " << srcFullPath << " has been loaded successfully to frameRef #" << frameRef->getFrameRefIndex();
	frameRef->setBits(++_lastAcquiredImage, tempMatObject);

	// this flag tells the algorithm runner to perform
	// image/CSV saving synchronously 
	// to avoid save queue growing constantly
	// for offline analysis it's not critical to perform saving synchronously
	//frameRef->setAsyncWrite(false);

	// pass source image path to frame
	frameRef->setNamedParameter(NAMED_PROPERTY_SOURCE_PATH, stdPath);
	return RESULT_OK;
}