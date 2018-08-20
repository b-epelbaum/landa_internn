#include "fgsimulator.h"
#include <QDirIterator>
#include "util.h"
#include <thread>
#include "frameRef.h"


#ifdef _DEBUG
#pragma comment(lib, "opencv_world342d.lib")
#else
#pragma comment(lib, "opencv_world342.lib")
#endif

using namespace LandaJune::Core;

static const QString FG_SIM_PROVIDER_NAME = "Frame Grabber Simulator";
static const QString FG_SIM_PROVIDER_DESC = "Frame Grabber Simulator loads one or more images and pushes them to queue in specified timeout";


#define FGSIMULATOR_PROVIDER_SCOPED_LOG PRINT_INFO2 << "[FGSimulator] : "
#define FGSIMULATOR_PROVIDER__SCOPED_ERROR PRINT_ERROR << "[FGSimulator] : "
#define FGSIMULATOR_PROVIDER__SCOPED_WARNING PRINT_WARNING << "[FGSimulator] : "

using namespace LandaJune::FrameProviders;
using namespace LandaJune::Helpers;

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

FRAME_PROVIDER_ERROR FGSimulator::dataPreProcess(FrameRef* frameRef)
{
	if ( _images.empty() )
		throw ProviderException(FRAME_PROVIDER_ERROR::ERR_SIMULATOR_HAVE_NO_IMAGES, "FGSimulator has no images loaded. Has you forgot calling init ?");

	std::this_thread::sleep_for(std::chrono::milliseconds(_FrameFrequencyInMSec));
	_next = ++_next % _images.size();

	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}

FRAME_PROVIDER_ERROR FGSimulator::dataAccess(FrameRef* frameRef)
{
	if (_next >= _images.size())
	{
		FGSIMULATOR_PROVIDER_SCOPED_LOG << "_next index has exceeded total frame count";
		return FRAME_PROVIDER_ERROR::ERR_GENERAL_ERROR;
	}



	auto & img = _images.at(_next);

	auto w = img.width();
	auto h = img.height();
	auto s = img.sizeInBytes();

	frameRef->setBits(++_lastAcquiredImage, img.width(), img.height(), img.sizeInBytes(), img.bits());
	FGSIMULATOR_PROVIDER_SCOPED_LOG << "Received frame #" << _lastAcquiredImage;
	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}

FRAME_PROVIDER_ERROR FGSimulator::dataPostProcess(FrameRef* frameRef)
{
	return ERR_NOT_IMPLEMENTED;
}

FRAME_PROVIDER_ERROR FGSimulator::init()
{
	_lastAcquiredImage = -1;
	_images.clear();
	const auto imageFolder = _SourceFolderPath;
	auto images = QDir(imageFolder).entryList(QStringList() << "*.bmp" << "*.BMP", QDir::Files);

	for (auto const& imPath : images)
	{
		const auto t1 = Utility::now_in_millisecond();
		const auto pathName = QString("%1/%2").arg(imageFolder).arg(imPath);
		FGSIMULATOR_PROVIDER_SCOPED_LOG << "found BMP image : " << pathName << "; loading...";

		QImageReader imgReader(pathName);

		// read image to QIMage object
		auto img = imgReader.read();

		// ensure that image has pixels alignment in multiples of 4 ( bitmap requirement )
		// if not, pad pixels to the right

		auto const newWIdth = (img.width() + 3) & ~0x03;

		// make a copy of the image with proper padding 
		auto copy = img.copy(0, 0, newWIdth, img.height()).convertToFormat(QImage::Format_RGB888).rgbSwapped();

		// add copy to array
		_images.push_back(std::move(copy));
		FGSIMULATOR_PROVIDER_SCOPED_LOG << "finished loading file " << imPath << " in " << Utility::now_in_millisecond() - t1 << " msec";
	}
	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}

FRAME_PROVIDER_ERROR FGSimulator::clean()
{
	_images.clear();
	_next = 0;
	FGSIMULATOR_PROVIDER_SCOPED_LOG << "cleaned up";

	return FRAME_PROVIDER_ERROR::ERR_NO_ERROR;
}
