#include "fullpagehandler.h"
#include "applog.h"

using namespace LandaJune;
using namespace Algorithms;
using namespace Parameters;
using namespace Threading;
using namespace Core;

static const QString FULL_PAGE_HANDLER_NAME = "Full Page Handler";
static const QString FULL_PAGE_HANDLER_DESC = "Full Scanned Page Algorithm Set";

#define FULLPAGE_HANDLER_SCOPED_LOG PRINT_INFO2 << "[fullPageHandler] : "
#define FULLPAGE_HANDLER_SCOPED_ERROR PRINT_ERROR << "[fullPageHandler] : "
#define FULLPAGE_HANDLER_SCOPED_WARNING PRINT_WARNING << "[fullPageHandler] : "


fullPageHandler::fullPageHandler()
{
	FULLPAGE_HANDLER_SCOPED_LOG << "created";
}

fullPageHandler::~fullPageHandler()
{
	FULLPAGE_HANDLER_SCOPED_LOG << "destroyed";
}

std::unique_ptr<IAlgorithmHandler> fullPageHandler::clone()
{
	auto retVal = std::make_unique<fullPageHandler>();
	retVal->_processParameters = _processParameters;
	return std::move(retVal);
}

QString fullPageHandler::getName() const
{
	return FULL_PAGE_HANDLER_NAME;
}

QString fullPageHandler::getDescription() const
{
	return FULL_PAGE_HANDLER_DESC;
}

std::shared_ptr<BaseParameter> fullPageHandler::getParameters() const
{
	return std::static_pointer_cast<BaseParameter>(_processParameters);
}

void fullPageHandler::init(std::shared_ptr<BaseParameter> parameters)
{
	validateProcessParameters(parameters);
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
		FULLPAGE_HANDLER_SCOPED_WARNING << "No C2C ROI defined !";
	}
	initWave(INIT_PARAMETER{});
}

void fullPageHandler::cleanup()
{
	shutdownEdge();
	shutdownI2S();
	shutdownC2CRoi();
	shutdownWave();
}

void fullPageHandler::validateProcessParameters(std::shared_ptr<BaseParameter> parameters)
{
	_processParameters = std::dynamic_pointer_cast<ProcessParameter>(parameters);
}

void fullPageHandler::process(const FrameRef * frame)
{
	// call general process implementation of parent class
	abstractAlgoHandler::process(frame);

	PARAMS_C2C_SHEET_INPUT input(_frame);
	
	// fill process parameters
	fillSheetProcessParameters(input);

	// generate ROIs for all required elements
	CV_COPY_REGION_LIST regionList;
	generateSheetRegions(input, regionList);

	// and perform a deep copy
	copyRegions(regionList);

	// process whole sheet
	auto output = processSheet(input);
}