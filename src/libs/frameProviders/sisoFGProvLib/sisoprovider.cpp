#include "sisoprovider.h"
#include "util.h"
#include "frameRef.h"
#include "ProcessParameters.h"

#include <QDirIterator>

#include <opencv2/imgcodecs.hpp>

#ifdef ENABLE_FGRAB
#pragma comment(lib, "fglib5.lib")
#pragma comment(lib, "display_lib.lib")
#endif

#ifdef _DEBUG
#pragma comment(lib, "opencv_world342d.lib")
#else
#pragma comment(lib, "opencv_world342.lib")
#endif

static const QString SISO_PROVIDER_NAME = "Silicon Software Frame Grabber";
static const QString SISO_PROVIDER_DESC = "Silicon Software Frame Grabber";

#define SISO_PROVIDER_SCOPED_LOG PRINT_INFO3 << "[SiSoProvider] : "
#define SISO_PROVIDER__SCOPED_ERROR PRINT_ERROR << "[SiSoProvider] : "
#define SISO_PROVIDER__SCOPED_WARNING PRINT_WARNING << "[SiSoProvider] : "

using namespace LandaJune;
using namespace Core;
using namespace Parameters;
using namespace FrameProviders;
using namespace Helpers;

SiSoProvider::SiSoProvider()
{
	_name = SISO_PROVIDER_NAME;
	_description = SISO_PROVIDER_DESC;
	detectBoards();
	SISO_PROVIDER_SCOPED_LOG << "created";
}


SiSoProvider::~SiSoProvider()
{
	SISO_PROVIDER_SCOPED_LOG << "destroyed";
}

bool SiSoProvider::canContinue(const CORE_ERROR lastError)
{
	if (lastError == RESULT_OK)
		return true;

	auto _canContinue = false;
	switch (lastError)
	{
		case CORE_ERROR::ERR_FRAMEGRABBER_IMAGE_SKIPPED: _canContinue = true;  break;
		default:
			;
	}
	return _canContinue;
}

CORE_ERROR SiSoProvider::prepareData(FrameRef* frameRef)
{
#ifdef ENABLE_FGRAB
	const auto targetChannel = _camPort1;
	auto const imageIndex = Fg_getLastPicNumberBlocking(_fg, _lastAcquiredImage + 1, targetChannel, 10);

	if (imageIndex > -1)
	{
		_lastAcquiredImage = imageIndex;
		return (_lastAcquiredImage % 2 != 0)
			? CORE_ERROR::ERR_FRAMEGRABBER_IMAGE_SKIPPED
			: CORE_ERROR::ERR_NO_ERROR;
	}

	if (imageIndex == FG_TIMEOUT_ERR)
	{
		SISO_PROVIDER_SCOPED_LOG << "Frame grabber is OFF. No images have been captured in 10 sec";
		return CORE_ERROR::ERR_FRAMEGRABBER_IMAGE_TIMEOUT;
	}
#endif	
	return RESULT_NOT_IMPLEMENTED;
}

CORE_ERROR SiSoProvider::accessData(FrameRef* frameRef)
{
#ifdef ENABLE_FGRAB
	const auto targetChannel = _camPort1;

	const auto imageBits = static_cast<uint8_t*>(Fg_getImagePtr(_fg, _lastAcquiredImage, targetChannel));

	if (imageBits == nullptr)
	{
		return CORE_ERROR::ERR_FRAMEGRABBER_IMAGE_INVALID_DATA_POINTER;
	}
	int imageSizeReceived = _lastAcquiredImage;
	Fg_getParameterWithType(_fg, FG_TRANSFER_LEN, &imageSizeReceived, 0, FG_PARAM_TYPE_UINT32_T );
	int32_t width, height;

	Fg_getParameter(_fg, FG_WIDTH, &width, targetChannel);
	Fg_getParameter(_fg, FG_HEIGHT, &height, targetChannel);

	frameRef->setBits(_lastAcquiredImage, width, height, static_cast<size_t>(imageSizeReceived), imageBits);

#endif	
	return RESULT_NOT_IMPLEMENTED;
}

void SiSoProvider::releaseData(FrameRef* frameRef)
{
}

CORE_ERROR SiSoProvider::init(std::shared_ptr<Parameters::BaseParameters> parameters)
{
	validateParameters(parameters);
	connect (_providerParameters.get(), &BaseParameters::updateCalculated, this, &BaseFrameProvider::onUpdateParameters);

	_lastAcquiredImage = -1;
	//300m_1200m_300rgb_Windows_AMD64.hap
	std::string applet;

#ifdef ENABLE_FGRAB
	switch (Fg_getBoardType(_BoardIndex))
	{
	case PN_MICROENABLE4AS1CL:
		applet = "SingleAreaGray16";
		break;
	case PN_MICROENABLE4AD1CL:
	case PN_MICROENABLE4AD4CL:
	case PN_MICROENABLE4VD1CL:
	case PN_MICROENABLE4VD4CL:
		applet = _AppletFilePath.toStdString();
		break;
	case PN_MICROENABLE4AQ4GE:
	case PN_MICROENABLE4VQ4GE:
		applet = "QuadAreaGray16";
		break;
	case PN_MICROENABLE3I:
		applet = "DualAreaGray";
		break;
	case PN_MICROENABLE3IXXL:
		applet = "DualAreaGray12XXL";
		break;
	default:
		applet = "DualAreaGray16";
		break;
	}

	SISO_PROVIDER_SCOPED_LOG << "Loading applet : " << applet.c_str();
	if ((_fg = Fg_InitEx(applet.c_str(), _BoardIndex, 0)) == nullptr)
	{
		PRINT_ERROR << "[SiSoImageProvider] : Failed loading applet : " << applet.c_str() << ". Error : " << QString(Fg_getLastErrorDescription(nullptr));
		return CORE_ERROR::ERR_FRAMEGRABBER_LOAD_APPLET_FAILED;
	}
	SISO_PROVIDER_SCOPED_LOG << "Applet : " << applet.c_str() << " loaded OK";


	SISO_PROVIDER_SCOPED_LOG << "Loading configuration file : " << _ConfigurationFilePath;
	if ((Fg_loadConfig(_fg, _ConfigurationFilePath.toStdString().c_str())) < 0)
	{
		PRINT_ERROR << "[SiSoImageProvider] : Failed loading configuration file. Error : " << QString(Fg_getLastErrorDescription(_fg));
		Fg_FreeGrabber(_fg);
		return CORE_ERROR::ERR_FRAMEGRABBER_LOAD_CONFIG_FAILED;
	}
	SISO_PROVIDER_SCOPED_LOG << "Configuration file loaded OK";


	SISO_PROVIDER_SCOPED_LOG << "Getting image formats for all channels...";

	Fg_getParameter(_fg, FG_FORMAT, &_format1, _camPort1);
	Fg_getParameter(_fg, FG_FORMAT, &_format2, _camPort2);
	Fg_getParameter(_fg, FG_FORMAT, &_format2, _camPort3);


	size_t bytesPerPixel1 = 1;
	switch (_format1)
	{
	case FG_GRAY:	bytesPerPixel1 = 1; break;
	case FG_GRAY16:	bytesPerPixel1 = 2; break;
	case FG_COL24:	bytesPerPixel1 = 3; break;
	case FG_COL32:	bytesPerPixel1 = 4; break;
	case FG_COL30:	bytesPerPixel1 = 5; break;
	case FG_COL48:	bytesPerPixel1 = 6; break;
	default:
		bytesPerPixel1 = 1;
	}

	size_t bytesPerPixel2 = 1;
	switch (_format2)
	{
	case FG_GRAY:	bytesPerPixel2 = 1; break;
	case FG_GRAY16:	bytesPerPixel2 = 2; break;
	case FG_COL24:	bytesPerPixel2 = 3; break;
	case FG_COL32:	bytesPerPixel2 = 4; break;
	case FG_COL30:	bytesPerPixel2 = 5; break;
	case FG_COL48:	bytesPerPixel2 = 6; break;
	default: bytesPerPixel2 = 1;
	}

	size_t bytesPerPixel3 = 1;
	switch (_format2)
	{
	case FG_GRAY:	bytesPerPixel3 = 1; break;
	case FG_GRAY16:	bytesPerPixel3 = 2; break;
	case FG_COL24:	bytesPerPixel3 = 3; break;
	case FG_COL32:	bytesPerPixel3 = 4; break;
	case FG_COL30:	bytesPerPixel3 = 5; break;
	case FG_COL48:	bytesPerPixel3 = 6; break;
	default: bytesPerPixel3 = 1;
	}

	SISO_PROVIDER_SCOPED_LOG << "Image formats  : \n\tChannel A : "
		<< bytesPerPixel1 * 8
		<< "bpp\n\tChannel B : "
		<< bytesPerPixel2 * 8
		<< "bpp\n\tChannel C : "
		<< bytesPerPixel3 * 8
		<< "bpp";

	SISO_PROVIDER_SCOPED_LOG << "Getting image dimensions for all channels...";
	if (Fg_getParameter(_fg, FG_WIDTH, &_width1, _camPort1) < 0)
	{
		PRINT_ERROR << "[SiSoImageProvider] : Failed getting image WIDTH for channel A. Error : " << QString(Fg_getLastErrorDescription(_fg));
		Fg_FreeGrabber(_fg);
		return CORE_ERROR::ERR_FRAMEGRABBER_CANNOT_GET_PROPERTY;
	}

	SISO_PROVIDER_SCOPED_LOG << "-------------------------------------------------";
	SISO_PROVIDER_SCOPED_LOG << "Channel A image width : " << _width1 << "px";

	if (Fg_getParameter(_fg, FG_HEIGHT, &_height1, _camPort1) < 0)
	{
		PRINT_ERROR << "[SiSoImageProvider] : Failed getting image HEIGHT for channel A. Error : " << QString(Fg_getLastErrorDescription(_fg));
		Fg_FreeGrabber(_fg);
		return CORE_ERROR::ERR_FRAMEGRABBER_CANNOT_GET_PROPERTY;
	}
	SISO_PROVIDER_SCOPED_LOG << "Channel A image height : " << _height1 << "px";
	SISO_PROVIDER_SCOPED_LOG << "-------------------------------------------------";


	if (Fg_getParameter(_fg, FG_WIDTH, &_width2, _camPort2) < 0)
	{
		PRINT_ERROR << "[SiSoImageProvider] : Failed getting image WIDTH for channel B. Error : " << QString(Fg_getLastErrorDescription(_fg));
		Fg_FreeGrabber(_fg);
		return CORE_ERROR::ERR_FRAMEGRABBER_CANNOT_GET_PROPERTY;
	}
	SISO_PROVIDER_SCOPED_LOG << "Channel B image width : " << _width2 << "px";

	if (Fg_getParameter(_fg, FG_HEIGHT, &_height2, _camPort2) < 0)
	{
		PRINT_ERROR << "[SiSoImageProvider] : Failed getting image HEIGHT for channel B. Error : " << QString(Fg_getLastErrorDescription(_fg));
		Fg_FreeGrabber(_fg);
		return CORE_ERROR::ERR_FRAMEGRABBER_CANNOT_GET_PROPERTY;
	}
	SISO_PROVIDER_SCOPED_LOG << "Channel B image height : " << _height2 << "px";
	SISO_PROVIDER_SCOPED_LOG << "-------------------------------------------------";


	if (Fg_getParameter(_fg, FG_WIDTH, &_width3, _camPort3) < 0)
	{
		PRINT_ERROR << "[SiSoImageProvider] : Failed getting image WIDTH for channel C. Error : " << QString(Fg_getLastErrorDescription(_fg));
		Fg_FreeGrabber(_fg);
		return CORE_ERROR::ERR_FRAMEGRABBER_CANNOT_GET_PROPERTY;
	}
	SISO_PROVIDER_SCOPED_LOG << "Channel C image width : " << _width3 << "px";

	if (Fg_getParameter(_fg, FG_HEIGHT, &_height3, _camPort3) < 0)
	{
		PRINT_ERROR << "[SiSoImageProvider] : Failed getting image HEIGHT for channel C. Error : " << QString(Fg_getLastErrorDescription(_fg));
		Fg_FreeGrabber(_fg);
		return CORE_ERROR::ERR_FRAMEGRABBER_CANNOT_GET_PROPERTY;
	}
	SISO_PROVIDER_SCOPED_LOG << "Channel C image height : " << _height1 << "px";
	SISO_PROVIDER_SCOPED_LOG << "-------------------------------------------------";

	const size_t totalBufferSize1 = _width1 * _height1 * _nbBuffers * bytesPerPixel1;
	const size_t totalBufferSize2 = _width2 * _height2 * _nbBuffers * bytesPerPixel2;
	const size_t totalBufferSize3 = _width3 * _height3 * _nbBuffers * bytesPerPixel3;

	PRINT_DEBUG_LINE
		SISO_PROVIDER_SCOPED_LOG << "Allocating memory for buffers...";

	if ((_imgBuf1 = Fg_AllocMem(_fg, totalBufferSize1, _nbBuffers, PORT_A)) == nullptr)
	{
		PRINT_ERROR << "[SiSoImageProvider] : Allocating memory buffer of " << totalBufferSize1 << "bytes for channel 0 FAILED with error :" << QString(Fg_getLastErrorDescription(_fg));
		Fg_FreeGrabber(_fg);
		return CORE_ERROR::ERR_FRAMEGRABBER_MEMORY_ALLOCATION_FAILED;
	}
	SISO_PROVIDER_SCOPED_LOG << "\n-------------------------------------------------";
	SISO_PROVIDER_SCOPED_LOG << totalBufferSize1 << " bytes has been allocated for channel 0";
	SISO_PROVIDER_SCOPED_LOG << "-------------------------------------------------\n";


	if ((_imgBuf2 = Fg_AllocMem(_fg, totalBufferSize2, _nbBuffers, PORT_B)) == nullptr)
	{
		PRINT_ERROR << "[SiSoImageProvider] : Allocating memory buffer of " << totalBufferSize2 << "bytes for channel 1 FAILED with error :" << QString(Fg_getLastErrorDescription(_fg));
		Fg_FreeMem(_fg, PORT_A);
		Fg_FreeGrabber(_fg);
		return CORE_ERROR::ERR_FRAMEGRABBER_MEMORY_ALLOCATION_FAILED;
	}
	SISO_PROVIDER_SCOPED_LOG << "\n-------------------------------------------------";
	SISO_PROVIDER_SCOPED_LOG << totalBufferSize2 << " bytes has been allocated for channel 1";
	SISO_PROVIDER_SCOPED_LOG << "-------------------------------------------------\n";

	if ((_imgBuf3 = Fg_AllocMem(_fg, totalBufferSize3, _nbBuffers, PORT_C)) == nullptr)
	{
		PRINT_ERROR << "[SiSoImageProvider] : Allocating memory buffer of " << totalBufferSize3 << "bytes for channel 2 FAILED with error :" << QString(Fg_getLastErrorDescription(_fg));
		Fg_FreeMem(_fg, PORT_A);
		Fg_FreeMem(_fg, PORT_B);
		Fg_FreeGrabber(_fg);
		return CORE_ERROR::ERR_FRAMEGRABBER_MEMORY_ALLOCATION_FAILED;
	}
	SISO_PROVIDER_SCOPED_LOG << "\n-------------------------------------------------";
	SISO_PROVIDER_SCOPED_LOG << totalBufferSize3 << " bytes has been allocated for channel 2";
	SISO_PROVIDER_SCOPED_LOG << "-------------------------------------------------\n";


	SISO_PROVIDER_SCOPED_LOG << "Starting image acquisition...";

	if ((Fg_Acquire(_fg, _camPort1, _nrOfPicturesToGrab)) < 0)
	{
		PRINT_ERROR << "[SiSoImageProvider] : Image acquisition on channel 0 FAILED with error :" << QString(Fg_getLastErrorDescription(_fg));
		Fg_FreeMem(_fg, PORT_A);
		Fg_FreeMem(_fg, PORT_B);
		Fg_FreeMem(_fg, PORT_C);
		Fg_FreeGrabber(_fg);
		return CORE_ERROR::ERR_FRAMEGRABBER_IMAGE_ACQUISITION_FAILED;
	}
	SISO_PROVIDER_SCOPED_LOG << "Image acquisition started on CHANNEL 0 - SUCCESS";


	if ((Fg_Acquire(_fg, _camPort2, _nrOfPicturesToGrab)) < 0)
	{
		PRINT_ERROR << "[SiSoImageProvider] : Image acquisition on channel 1 FAILED with error :" << QString(Fg_getLastErrorDescription(_fg));
		Fg_FreeMem(_fg, PORT_A);
		Fg_FreeMem(_fg, PORT_B);
		Fg_FreeMem(_fg, PORT_C);
		Fg_FreeGrabber(_fg);
		return CORE_ERROR::ERR_FRAMEGRABBER_IMAGE_ACQUISITION_FAILED;
	}
	SISO_PROVIDER_SCOPED_LOG << "Image acquisition started on CHANNEL 1 - SUCCESS";

	if ((Fg_Acquire(_fg, _camPort3, _nrOfPicturesToGrab)) < 0)
	{
		PRINT_ERROR << "[SiSoImageProvider] : Image acquisition on channel 2 FAILED with error :" << QString(Fg_getLastErrorDescription(_fg));
		Fg_FreeMem(_fg, PORT_A);
		Fg_FreeMem(_fg, PORT_B);
		Fg_FreeMem(_fg, PORT_C);
		Fg_FreeGrabber(_fg);
		return CORE_ERROR::ERR_FRAMEGRABBER_IMAGE_ACQUISITION_FAILED;
	}
	SISO_PROVIDER_SCOPED_LOG << "================================================";
	SISO_PROVIDER_SCOPED_LOG << "Frame grabber has been initialized successfully";
	SISO_PROVIDER_SCOPED_LOG << "================================================";
	return CORE_ERROR::ERR_NO_ERROR;
#endif
	return RESULT_OK;
}

void SiSoProvider::validateParameters(std::shared_ptr<BaseParameters> parameters)
{
	// TODO : query BaseParameters for named parameters
	// currently hardcoded

	const auto processParams = std::dynamic_pointer_cast<ProcessParameters>(parameters);

	setAppletFilePath (processParams->SISO_AppletFilePath() );
	setConfigurationFilePath (processParams->SISO_ConfigurationFilePath());
	setOutputImageFormat(processParams->SISO_OutputImageFormat());
	setBoardList(processParams->SISO_BoardList());
	setBoardIndex(processParams->SISO_BoardIndex());

	_providerParameters = parameters;
}


CORE_ERROR SiSoProvider::cleanup()
{
#ifdef ENABLE_FGRAB

	//if (_displayId != -1)
	//	CloseDisplay(_displayId);

	SISO_PROVIDER_SCOPED_LOG << "Stopping aqcuisition on all channels...";
	Fg_stopAcquire(_fg, _camPort1);
	Fg_stopAcquire(_fg, _camPort2);
	Fg_stopAcquire(_fg, _camPort3);

	Fg_FreeMem(_fg, _camPort1);
	Fg_FreeMem(_fg, _camPort2);
	Fg_FreeMem(_fg, _camPort3);
	Fg_FreeGrabber(_fg);
	SISO_PROVIDER_SCOPED_LOG << "Aqcuisition stopped on all channels";
#endif
	disconnect (_providerParameters.get(), &BaseParameters::updateCalculated, this, &BaseFrameProvider::onUpdateParameters);
	SISO_PROVIDER_SCOPED_LOG << "================================================";
	SISO_PROVIDER_SCOPED_LOG << "Frame grabber has been cleaned up";
	SISO_PROVIDER_SCOPED_LOG << "================================================";
	return RESULT_OK;
}


void SiSoProvider::detectBoards()
{
#ifdef ENABLE_FGRAB
	SISO_PROVIDER_SCOPED_LOG << "Detecting SiSo board...";

	auto boards = detectSiSoBoard();
	if (boards.empty())
	{
		PRINT_WARNING << "No SiSo board has been detected.";
	}

	QStringList qBoardList;
	for (const auto& val : boards)
	{
		qBoardList << QString::fromStdString(val);
	}

	setProviderProperty("BoardList", qBoardList);
#endif
}

void SiSoProvider::createDisplay(int imageFormat, int width, int height)
{
#ifdef ENABLE_FGRAB
	// creating display on parameters for channel 0 ?
	_displayId = ::CreateDisplay(getNoOfBitsFromImageFormat(imageFormat), width, height);
	SetBufferWidth(_displayId, width, height);
#endif
}


void SiSoProvider::dumpTestImageSync(uint8_t * imageBits, int width, int height, int imageCVFormat) const
{
	const auto bytesPerSample = 3;
	static auto _counter = 0;

	const QString& path = "c:\\temp\\sistdump\\img.bmp";
	cv::Mat temp = { height, width, imageCVFormat, imageBits };
	cv::imwrite(path.toStdString().c_str(), temp);
}
