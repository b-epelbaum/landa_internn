#include "folderReader.h"
#include <QDirIterator>

#include "util.h"
#include "frameRef.h"

#include "ProcessParameters.h"

#include <thread>
#include <opencv2/imgcodecs.hpp>
#include <QDateTime>
#include "common/june_enums.h"

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

struct ImageFileLessFunctor
{
	bool operator() (const QString& left, const QString& right)
	{
		auto leftInfo  = QFileInfo(left);
		auto rightInfo  = QFileInfo(right);

		return leftInfo.lastModified() < rightInfo.lastModified();
	}
};


folderReader::folderReader()
{
	FOLDER_READER_PROVIDER_SCOPED_LOG << "created";
}

folderReader::~folderReader()
{
	FOLDER_READER_PROVIDER_SCOPED_LOG << "destroyed";
}

void folderReader::sortImageFileList()
{
	std::sort(_imagePaths.begin(), _imagePaths.end(), 
			[](const QString& left, const QString& right) 
			{
				auto leftInfo  = QFileInfo(left);
				auto rightInfo  = QFileInfo(right);
				return leftInfo.lastModified() < rightInfo.lastModified();
			});
}

void folderReader::init(BaseParametersPtr parameters, Core::ICore * coreObject, CoreEventCallback callback)
{
	try
	{
		validateParameters(parameters);
		connect (parameters.get(), &BaseParameters::updateCalculated, this, &BaseFrameProvider::onUpdateParameters);
		
		_dataCallback = callback;
		_coreObject = coreObject;
			
		_lastAcquiredImage = -1;
		_imagePaths.clear();

		
		const auto imageFolder = _SourceFolderPath; 
		if ( imageFolder.isEmpty() || !QFileInfo(imageFolder).exists())
		{
			FOLDER_READER_PROVIDER_SCOPED_ERROR << "Source folder " << imageFolder << " invalid or cannot be found. Aborting scan...";
			THROW_EX_ERR_STR(CORE_ERROR::ERR_OFFLINEREADER_SOURCE_FILE_INVALID, "Source folder invalid or cannot be found : " + imageFolder.toStdString() );
		}

		FOLDER_READER_PROVIDER_SCOPED_LOG << "Scanning for source files at " << imageFolder << "...";
		QDirIterator it(imageFolder, QStringList() << "*.bmp" << "*.BMP", QDir::Files, QDirIterator::Subdirectories);

		while (it.hasNext()) 
		{
			_imagePaths.push_back(it.next());
		}
		
		FOLDER_READER_PROVIDER_SCOPED_LOG << "found " << _imagePaths.size() << " files. Sorting...";
		if (_imagePaths.isEmpty())
		{
			FOLDER_READER_PROVIDER_SCOPED_ERROR << "Source folder " << imageFolder << " contains no viable files";
			THROW_EX_ERR_STR(CORE_ERROR::ERR_OFFLINEREADER_NO_FILES_TO_LOAD, "Source folder icontains no files for loading : " + imageFolder.toStdString() );
		}

		sortImageFileList();
		FOLDER_READER_PROVIDER_SCOPED_LOG << "sorting complete.";

		if (_dataCallback)
		{
			int itemsToReport = ( _ImageMaxCount > 0 ) ? qMin(_imagePaths.size(), _ImageMaxCount) : _imagePaths.size();
			_dataCallback(_coreObject, CoreCallbackType::CALLBACK_PROVIDER_SCANNED_FILES_COUNT, std::make_any<int>(itemsToReport));
		}
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

void folderReader::cleanup()
{
	try
	{
		disconnect (_providerParameters.get(), &BaseParameters::updateCalculated, this, &BaseFrameProvider::onUpdateParameters);
		_imagePaths.clear();
		_coreObject = nullptr;
		_dataCallback = nullptr;
		_providerParameters.reset();

		FOLDER_READER_PROVIDER_SCOPED_LOG << "cleaned up";
	}
	catch(BaseException& bex)
	{
		throw;
	}
	catch ( std::exception& ex )
	{
		RETHROW( CORE_ERROR::ERR_PROVIDER_CLEANUP_FAILED);
	}
}


void folderReader::validateParameters(BaseParametersPtr parameters)
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

	if (_ImageMaxCount > 0 &&  _lastAcquiredImage == _ImageMaxCount -1 )
	{
		FOLDER_READER_PROVIDER_SCOPED_LOG << "Reached a maximum number of provided images. Exiting...";
		return CORE_ERROR::ERR_SIMULATOR_REACHED_MAX_COUNT;
	}
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

	// TODO : replace this function/member to derivative of offline/online generated bits ?
	frameRef->setAsyncWrite(false);

	// pass source image path to frame
	frameRef->setNamedParameter(NAMED_PROPERTY_SOURCE_PATH, stdPath);
	return RESULT_OK;
}