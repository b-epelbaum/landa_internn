#include "fgsimulator.h"
#include <QDirIterator>
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

static const QString FG_SIM_PROVIDER_NAME = "Frame Grabber Simulator";
static const QString FG_SIM_PROVIDER_DESC = "Frame Grabber Simulator loads one or more images and pushes them to queue in specified timeout";


#define FGSIMULATOR_PROVIDER_SCOPED_LOG PRINT_INFO2 << "[FGSimulator] : "
#define FGSIMULATOR_PROVIDER__SCOPED_ERROR PRINT_ERROR << "[FGSimulator] : "
#define FGSIMULATOR_PROVIDER__SCOPED_WARNING PRINT_WARNING << "[FGSimulator] : "

FGSimulator::FGSimulator()
{
	_name = FG_SIM_PROVIDER_NAME;
	_description = FG_SIM_PROVIDER_DESC;
	FGSIMULATOR_PROVIDER_SCOPED_LOG << "created";
}

FGSimulator::~FGSimulator()
{
	FGSIMULATOR_PROVIDER_SCOPED_LOG << "destroyed";
}

bool FGSimulator::canContinue(FRAME_PROVIDER_ERROR lastError)
{
	return false;
}

FRAME_PROVIDER_ERROR FGSimulator::prepareData(FrameRef* frameRef)
{
	if ( _images.empty() )
		throw ProviderException(FRAME_PROVIDER_ERROR::ERR_SIMULATOR_HAVE_NO_IMAGES, "FGSimulator has no images loaded. Has you forgot calling init ?");

	std::this_thread::sleep_for(std::chrono::milliseconds(_FrameFrequencyInMSec));
	_next = ++_next % _images.size();

	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}

FRAME_PROVIDER_ERROR FGSimulator::accessData(FrameRef* frameRef)
{
	if (_next >= _images.size())
	{
		FGSIMULATOR_PROVIDER_SCOPED_LOG << "_next index has exceeded total frame count";
		return FRAME_PROVIDER_ERROR::ERR_GENERAL_ERROR;
	}

	
	auto & img = _images.at(_next);

	const auto w = img.cols;
	const auto h = img.rows;
	const auto s = img.step[0] * img.rows;

	frameRef->setBits(++_lastAcquiredImage, w, h, s, img.data);
	FGSIMULATOR_PROVIDER_SCOPED_LOG << "Received frame #" << _lastAcquiredImage;
	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}

void FGSimulator::releaseData(FrameRef* frameRef)
{
}

FRAME_PROVIDER_ERROR FGSimulator::init()
{
	_lastAcquiredImage = -1;
	_images.clear();

	auto images = QDir(_SourceFolderPath).entryList(QStringList() << "*.bmp" << "*.BMP", QDir::Files);

	if ( images.empty() )
	{
		FGSIMULATOR_PROVIDER__SCOPED_WARNING << "No valid BMP files found in folder " << _SourceFolderPath;
		return FRAME_PROVIDER_ERROR::ERR_FGSIMULATOR_NO_FILE_FOUND;
	}

	for (auto const& imPath : images)
	{
		const auto t1 = Utility::now_in_millisecond();
		const auto pathName = QString("%1/%2").arg(_SourceFolderPath).arg(imPath);
		FGSIMULATOR_PROVIDER_SCOPED_LOG << "found BMP image : " << pathName << "; loading...";
		_images.emplace_back(cv::imread(pathName.toStdString()));
		FGSIMULATOR_PROVIDER_SCOPED_LOG << "finished loading file " << imPath << " in " << Utility::now_in_millisecond() - t1 << " msec";
	}
	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}

void FGSimulator::validateParameters(std::shared_ptr<BaseParameters> parameters)
{
	// TODO : query BaseParameters for named parameters
	// currently hardcoded

	const auto _processParameters = std::dynamic_pointer_cast<ProcessParameters>(parameters);
	_SourceFolderPath = _processParameters->FGS_SourceFolderPath();
	_SourceFilePath = _processParameters->FGS_SourceFilePath();
	_FrameFrequencyInMSec = _processParameters->FGS_FrameFrequencyInMSec();
}

FRAME_PROVIDER_ERROR FGSimulator::cleanup()
{
	_images.clear();
	_next = 0;
	FGSIMULATOR_PROVIDER_SCOPED_LOG << "cleaned up";

	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}