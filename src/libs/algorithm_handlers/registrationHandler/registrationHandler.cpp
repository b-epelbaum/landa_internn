#include "registrationHandler.h"
#include "applog.h"


using namespace LandaJune;
using namespace Algorithms;
using namespace Parameters;
using namespace Threading;
using namespace Core;

static const QString REGISTRATION_HANDLER_NAME = "Offline Reader Registration Handler";
static const QString REGISTRATION_HANDLER_DESC = "Processes offline registration images";

#define REGISTRATION_HANDLER_SCOPED_LOG PRINT_INFO5 << "[registrationPageHandler] : "
#define REGISTRATION_HANDLER_SCOPED_ERROR PRINT_ERROR << "[registrationPageHandler] : "
#define REGISTRATION_HANDLER_SCOPED_WARNING PRINT_WARNING << "[registrationPageHandler] : "


registrationPageHandler::registrationPageHandler()
{
	REGISTRATION_HANDLER_SCOPED_LOG << "created";
}

registrationPageHandler::~registrationPageHandler()
{
	REGISTRATION_HANDLER_SCOPED_LOG << "destroyed";
}

std::unique_ptr<IAlgorithmHandler> registrationPageHandler::clone()
{
	auto retVal = std::make_unique<registrationPageHandler>();
	retVal->_processParameters = _processParameters;
	return std::move(retVal);
}

QString registrationPageHandler::getName() const
{
	return REGISTRATION_HANDLER_NAME;
}

QString registrationPageHandler::getDescription() const
{
	return REGISTRATION_HANDLER_DESC;
}

std::shared_ptr<BaseParameter> registrationPageHandler::getParameters() const
{
	return std::static_pointer_cast<BaseParameter>(_processParameters);
}

void registrationPageHandler::init(std::shared_ptr<BaseParameter> parameters)
{
	validateProcessParameters(parameters);
	createCSVFolder();

	// template image ( temporary solution )
	QFile templateTif(":/templates/Resources/Template1.tif");
	
	if (templateTif.open(QFile::ReadOnly))
		_processParameters->setCircleTemplateBuffer(templateTif.readAll());

	const INIT_PARAMETER edgeInitParam{ toROIRect(_processParameters->LeftStripRect()) };
	initEdge(edgeInitParam);

	const INIT_PARAMETER i2sInitParam{ toROIRect(_processParameters->I2SApproximateTriangleRectLeft()) };
	initI2S(i2sInitParam);

	if (!_processParameters->C2CROIArrayLeft().empty())
	{
		const INIT_PARAMETER c2croiInitParam{ toROIRect(_processParameters->C2CROIArrayLeft()[0]) };
		initC2CRoi(c2croiInitParam);
	}
	else
	{
		REGISTRATION_HANDLER_SCOPED_WARNING << "No C2C ROI defined !";
	}
}

void registrationPageHandler::cleanup()
{
	shutdownEdge();
	shutdownI2S();
	shutdownC2CRoi();
}

void registrationPageHandler::validateProcessParameters(std::shared_ptr<BaseParameter> parameters)
{
	_processParameters = std::dynamic_pointer_cast<ProcessParameter>(parameters);
}

void registrationPageHandler::process(const FrameRef * frame)
{
	// call general process implementation of parent class
	abstractAlgoHandler::process(frame);

	try
	{
		_sourceFrameImageName = std::any_cast<std::string>(frame->getNamedParameter("srcPath"));
	}
	catch (const std::bad_any_cast& e)
	{
		REGISTRATION_HANDLER_SCOPED_ERROR << "Cannot retrieve source image file name. Error : " << e.what() << "; Resetting to default...";
		_sourceFrameImageName = fmt::format("source_frame_#{0}", _frameIndex);
	}
		
	PARAMS_C2C_STRIP_INPUT input(_frame, LEFT);
	fillStripProcessParameters(input, LEFT);

	// generate ROIs for all required elements
	CV_COPY_REGION_LIST regionList;
	generateStripRegions(input, regionList);

	// and perform a deep copy
	copyRegions(regionList);

	auto output = processStrip(input, true);
}