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

bool folderReader::canContinue(CORE_ERROR lastError)
{
	auto _canContinue = true;
	switch (lastError) 
	{ 
		case CORE_ERROR::ERR_OFFLINEREADER_NO_MORE_FILES: _canContinue = false;  break;
		default: 
		;
	}
	return _canContinue;
}

CORE_ERROR folderReader::prepareData(FrameRef* frameRef)
{
	if (_imagePaths.empty())
	{
		FOLDER_READER_PROVIDER_SCOPED_LOG << "No more files to handle. Exiting...";
		return CORE_ERROR::ERR_OFFLINEREADER_NO_MORE_FILES;
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	return RESULT_OK;
}

CORE_ERROR folderReader::accessData(FrameRef* frameRef)
{
	// read image to cv::Mat object
	const auto srcFullPath = _imagePaths.first();
	FOLDER_READER_PROVIDER_SCOPED_LOG << "loading BMP registration image : " << srcFullPath << "...";
	_imagePaths.pop_front();

	const auto& stdPath = srcFullPath.toStdString();
	const auto tempMatObject = std::make_shared<cv::Mat>(cv::imread(stdPath));
	if (!tempMatObject->data)            // Check for invalid input
	{
		FOLDER_READER_PROVIDER_SCOPED_WARNING << "Cannot load image " << srcFullPath;
		return CORE_ERROR::ERR_OFFLINEREADER_SOURCE_FILE_INVALID;
	}

	FOLDER_READER_PROVIDER_SCOPED_LOG << "Image " << srcFullPath << " has been loaded successfully to frameRef #" << frameRef->getFrameRefIndex();

	const auto w = tempMatObject->cols;
	const auto h = tempMatObject->rows;
	const auto s = tempMatObject->step[0] * tempMatObject->rows;

	// push bits to frameRef object
	frameRef->setBits(++_lastAcquiredImage, w, h, s, tempMatObject->data);

	// attach a created new image to unknown data
	frameRef->setSharedData(tempMatObject);

	// pass source image path to frame
	frameRef->setNamedParameter(NAMED_PROPERTY_SOURCE_PATH, stdPath);
	return RESULT_OK;
}

void folderReader::releaseData(FrameRef* frameRef)
{
	if ( frameRef )
	{
		auto& sharedData = frameRef->getSharedData();
		if (sharedData.has_value())
		{
			try
			{
				auto pImage = std::any_cast<std::shared_ptr<cv::Mat>>(sharedData);
				pImage->release();
			}
			catch (const std::bad_any_cast& e)
			{
				FOLDER_READER_PROVIDER_SCOPED_ERROR << "Cannot delete shared MAT object. Exception caught : " << e.what();
			}
		}
	}
}

void folderReader::validateParameters(std::shared_ptr<BaseParameters> parameters)
{
	// TODO : query BaseParameters for named parameters
	// currently hardcoded

	const auto _processParameters = std::dynamic_pointer_cast<ProcessParameters>(parameters);
	_SourceFolderPath = _processParameters->SourceFolderPath();
	_ImageMaxCount = _processParameters->ImageMaxCount();

	FOLDER_READER_PROVIDER_SCOPED_LOG << "Validating provider parameters : ";
	FOLDER_READER_PROVIDER_SCOPED_LOG << "---------------------------------------";
	FOLDER_READER_PROVIDER_SCOPED_LOG << "_SourceFolderPath = " << _SourceFolderPath;
	FOLDER_READER_PROVIDER_SCOPED_LOG << "_ImageMaxCount = " << _ImageMaxCount;
}

CORE_ERROR folderReader::init()
{
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
	_imagePaths.clear();
	FOLDER_READER_PROVIDER_SCOPED_LOG << "cleaned up";
	return RESULT_OK;
}
