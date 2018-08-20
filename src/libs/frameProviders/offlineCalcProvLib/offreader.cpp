#include "offreader.h"
#include <QDirIterator>

#include "util.h"
#include "frameRef.h"


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

using namespace LandaJune::Core;
using namespace LandaJune::FrameProviders;
using namespace LandaJune::Helpers;

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
	if (_imagePaths.empty())
	{
		OFFREADER_PROVIDER_SCOPED_LOG << "No more files to handle. Exiting...";
		return FRAME_PROVIDER_ERROR::ERR_OFFLINEREADER_NO_MORE_FILES;
	}
	
	// read image to QIMage object
	const auto srcFullPath = QString("%1/%2").arg(_SourceFolderPath).arg(_imagePaths.first());
	OFFREADER_PROVIDER_SCOPED_LOG << "found BMP image : " << srcFullPath << "; loading...";
	_imagePaths.pop_front();
	
	QImageReader imgReader(srcFullPath);
	imgReader.read(&_currentImage);
	if (_currentImage.isNull() )
	{
		OFFREADER_PROVIDER_SCOPED_LOG << "Cannot load image " << srcFullPath << "; error : " << imgReader.errorString();
		return FRAME_PROVIDER_ERROR::ERR_OFFLINEREADER_SOURCE_FILE_INVALID;
	}

	++_lastAcquiredImage;
	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}

FRAME_PROVIDER_ERROR OfflineReader::dataAccess(FrameRef* frameRef)
{
	if (_currentImage.isNull() )
	{
		OFFREADER_PROVIDER_SCOPED_LOG << "Currently loaded image is not valid";
		return FRAME_PROVIDER_ERROR::ERR_OFFLINEREADER_SOURCE_FILE_INVALID;
	}
	frameRef->setBits(++_lastAcquiredImage, _currentImage.width(), _currentImage.height(), _currentImage.sizeInBytes(),_currentImage.bits());

	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}

FRAME_PROVIDER_ERROR OfflineReader::dataPostProcess(FrameRef* frameRef)
{
	return ERR_NOT_IMPLEMENTED;
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

	const auto paths = QDir(imageFolder).entryList(QStringList() << "*.bmp" << "*.BMP", QDir::Files);

	_imagePaths = paths.toVector();
	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}

FRAME_PROVIDER_ERROR OfflineReader::clean()
{
	_imagePaths.clear();
	_currentImage = QImage{};
	OFFREADER_PROVIDER_SCOPED_LOG << "cleaned up";
	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}
