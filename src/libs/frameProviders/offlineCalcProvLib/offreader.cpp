#include "offreader.h"
#include <QDirIterator>

#include "util.h"
#include "frameRef.h"

#include "ProcessParameter.h"
#include <thread>

#ifdef _DEBUG
#pragma comment(lib, "opencv_world342d.lib")
#else
#pragma comment(lib, "opencv_world342.lib")
#endif


static const QString OFFREADER_PROVIDER_NAME = "Offline Reader";
static const QString OFFREADER_PROVIDER_DESC = "Performs QCS analysis in offline mode";


#define OFFREADER_PROVIDER_SCOPED_LOG PRINT_INFO2 << "[OfflineReader] : "
#define OFFREADER_PROVIDER_SCOPED_ERROR PRINT_ERROR << "[OfflineReader] : "
#define OFFREADER_PROVIDER_SCOPED_WARNING PRINT_WARNING << "[OfflineReader] : "

using namespace LandaJune;
using namespace Core;
using namespace Parameters;
using namespace FrameProviders;
using namespace Helpers;

OfflineReader::OfflineReader()
{
	_name = OFFREADER_PROVIDER_NAME;
	_description = OFFREADER_PROVIDER_DESC;
	OFFREADER_PROVIDER_SCOPED_LOG << "created";
}

OfflineReader::~OfflineReader()
{
	OFFREADER_PROVIDER_SCOPED_LOG << "destroyed";
}

bool OfflineReader::canContinue(FRAME_PROVIDER_ERROR lastError)
{
	auto _canContinue = true;
	switch (lastError) 
	{ 
		case FRAME_PROVIDER_ERROR::ERR_OFFLINEREADER_NO_MORE_FILES: _canContinue = false;  break;
		default: 
		;
	}
	return _canContinue;
}

FRAME_PROVIDER_ERROR OfflineReader::prepareData(FrameRef* frameRef)
{
	if (_imagePaths.empty())
	{
		OFFREADER_PROVIDER_SCOPED_LOG << "No more files to handle. Exiting...";
		return FRAME_PROVIDER_ERROR::ERR_OFFLINEREADER_NO_MORE_FILES;
	}

	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}

FRAME_PROVIDER_ERROR OfflineReader::accessData(FrameRef* frameRef)
{
	// read image to cv::Mat object
	const auto srcFullPath = _imagePaths.first();
	OFFREADER_PROVIDER_SCOPED_LOG << "loading BMP registration image : " << srcFullPath << "...";
	_imagePaths.pop_front();

	const auto& stdPath = srcFullPath.toStdString();
	const auto &tempObject = cv::imread(stdPath);
	if (!tempObject.data)            // Check for invalid input
	{
		OFFREADER_PROVIDER_SCOPED_WARNING << "Cannot load image " << srcFullPath;
		return FRAME_PROVIDER_ERROR::ERR_OFFLINEREADER_SOURCE_FILE_INVALID;
	}

	auto newImage = new cv::Mat(std::move(tempObject));
	OFFREADER_PROVIDER_SCOPED_LOG << "Image " << srcFullPath << " has been loaded successfully to frameRef #" << frameRef->getFrameRefIndex();
	++_lastAcquiredImage;
	
	const auto w = newImage->cols;
	const auto h = newImage->rows;
	const auto s = newImage->step[0] * newImage->rows;

	// push bits to frameRef object
	frameRef->setBits(++_lastAcquiredImage, w, h, s, newImage->data);

	// attach a created new image to unknown data
	frameRef->setUnknownData(newImage);

	// pass source image path to frame
	frameRef->setNamedParameter("srcPath", stdPath);
	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}

void OfflineReader::releaseData(FrameRef* frameRef)
{
	if ( frameRef )
	{
		auto& unknownData = frameRef->getUnknownData();
		if (unknownData.has_value())
		{
			try
			{
				const auto pImage = std::any_cast<cv::Mat*>(unknownData);
				delete pImage;
			}
			catch (const std::bad_any_cast& e)
			{
				OFFREADER_PROVIDER_SCOPED_ERROR << "Cannot delete stored MAT object. Exception cought : " << e.what();
			}
		}
	}
}

void OfflineReader::setProviderParameters(std::shared_ptr<BaseParameter> parameters)
{
	validateParameters(parameters);
	_providerParameters = parameters;
}

void OfflineReader::validateParameters(std::shared_ptr<BaseParameter> parameters)
{
	// TODO : query BaseParameter for named parameters
	// currently hardcoded

	const auto _processParameters = std::dynamic_pointer_cast<ProcessParameter>(parameters);
	_SourceFolderPath = _processParameters->Off_SourceFolderPath();
	_ImageMaxCount = _processParameters->Off_ImageMaxCount();
}

FRAME_PROVIDER_ERROR OfflineReader::init()
{
	_lastAcquiredImage = -1;
	_imagePaths.clear();

	const auto imageFolder = _SourceFolderPath;
	if ( !QFileInfo(imageFolder).exists())
	{
		return FRAME_PROVIDER_ERROR::ERR_OFFLINEREADER_SOURCE_FOLDER_INVALID;
	}

	QDirIterator it(imageFolder, QStringList() << "*.bmp" << "*.BMP", QDir::Files, QDirIterator::Subdirectories);

	while (it.hasNext()) {
		_imagePaths.push_back(it.next());
	}
	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}

FRAME_PROVIDER_ERROR OfflineReader::cleanup()
{
	_imagePaths.clear();
	OFFREADER_PROVIDER_SCOPED_LOG << "cleaned up";
	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}
