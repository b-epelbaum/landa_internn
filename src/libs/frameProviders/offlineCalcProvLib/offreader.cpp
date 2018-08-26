#include "offreader.h"
#include <QDirIterator>

#include "util.h"
#include "frameRef.h"

#include "ProcessParameter.h"

#ifdef _DEBUG
#pragma comment(lib, "opencv_world342d.lib")
#else
#pragma comment(lib, "opencv_world342.lib")
#endif


static const QString OFFREADER_PROVIDER_NAME = "Offline Reader";
static const QString OFFREADER_PROVIDER_DESC = "Performs QCS analysis in offline mode";


#define OFFREADER_PROVIDER_SCOPED_LOG PRINT_INFO2 << "[OfflineReader] : "
#define OFFREADER_PROVIDER__SCOPED_ERROR PRINT_ERROR << "[OfflineReader] : "
#define OFFREADER_PROVIDER__SCOPED_WARNING PRINT_WARNING << "[OfflineReader] : "

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

FRAME_PROVIDER_ERROR OfflineReader::dataPreProcess(FrameRef* frameRef)
{
	_currentImage.release();
	if (_imagePaths.empty())
	{
		OFFREADER_PROVIDER_SCOPED_LOG << "No more files to handle. Exiting...";
		return FRAME_PROVIDER_ERROR::ERR_OFFLINEREADER_NO_MORE_FILES;
	}
	
	// read image to cv::Mat object
	const auto srcFullPath = _imagePaths.first();
	OFFREADER_PROVIDER_SCOPED_LOG << "loading BMP registration image : " << srcFullPath << "...";
	_imagePaths.pop_front();
	
	_currentImage = cv::imread(srcFullPath.toStdString());
	if (!_currentImage.data)                              // Check for invalid input
	{
		OFFREADER_PROVIDER__SCOPED_WARNING << "Cannot load image " << srcFullPath;
		return FRAME_PROVIDER_ERROR::ERR_OFFLINEREADER_SOURCE_FILE_INVALID;
	}

	++_lastAcquiredImage;
	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}

FRAME_PROVIDER_ERROR OfflineReader::dataAccess(FrameRef* frameRef)
{
	if (!_currentImage.data )
	{
		OFFREADER_PROVIDER_SCOPED_LOG << "Currently loaded image is not valid";
		return FRAME_PROVIDER_ERROR::ERR_OFFLINEREADER_SOURCE_FILE_INVALID;
	}
	const auto w = _currentImage.cols;
	const auto h = _currentImage.rows;
	const auto s = _currentImage.step[0] * _currentImage.rows;

	frameRef->setBits(++_lastAcquiredImage, w, h, s,_currentImage.data);

	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}

FRAME_PROVIDER_ERROR OfflineReader::dataPostProcess(FrameRef* frameRef)
{
	return ERR_NOT_IMPLEMENTED;
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

	auto _processParameters = std::dynamic_pointer_cast<ProcessParameter>(parameters);
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
	_currentImage.release();
	OFFREADER_PROVIDER_SCOPED_LOG << "cleaned up";
	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}
