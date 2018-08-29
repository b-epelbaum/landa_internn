#include "registrationHandler.h"
#include "applog.h"
#include "util.h"


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

// naming convention
// source : c:\Temp\offline\offline\11_781_Registration\GeometricRegInf85_13\GeometricRegInf85_13layoutImg.bmp 
// target folder <root_folder>\11_Reg_Left
// file name : <Frame_ID>_<ImageIndex>_EDGE_LEFT
// file name for ROIs : <Frame_ID>_<ImageIndex>_C2C_LEFT_00_[x,y].bmp

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
	retVal->_csvFolder = _csvFolder;
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

std::string registrationPageHandler::getFrameFolderName()  const 
{
	//11_Reg_Left
	return std::move(fmt::format("{0}_Reg_{1}", _sourceFrameNumber, SIDE_NAMES[_regSide]));
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
	_sourceFrameNumber.clear();
	try
	{
		auto framePath = std::any_cast<std::string>(frame->getNamedParameter("srcPath"));
		
		//c:/temp/offline/10_780_Registration/GeometricRegInf85_12/GeometricRegInf85_12layoutImg.bmp

		auto splittedStrVec = Helpers::Utility::split_string (framePath, "\\/");
		if ( splittedStrVec.size() >= 4 )
		{
			const auto& rawName = splittedStrVec[splittedStrVec.size() - 3];
			auto splittedNameVec = Helpers::Utility::split_string (rawName, "_");
			if ( splittedNameVec.size() > 1 )
			{
				_sourceFrameNumber = splittedNameVec[0];
			}
		}
	}
	catch (const std::bad_any_cast& e)
	{
		REGISTRATION_HANDLER_SCOPED_ERROR << "Cannot retrieve source image file name. Error : " << e.what() << "; Resetting to default...";
	}

	if ( _sourceFrameNumber.empty())
		_sourceFrameNumber = std::to_string(_frameIndex);
	else
		_frameIndex = std::stoi(_sourceFrameNumber);
	
	PARAMS_C2C_STRIP_INPUT input(_frame, LEFT);
	fillStripProcessParameters(input, LEFT);

	// generate ROIs for all required elements
	IMAGE_REGION_LIST regionList;
	generateStripRegions(input, regionList);

	// and perform a deep copy
	copyRegions(regionList);

	//process the whole strip
	const auto output = std::move(processStrip(input, true));

	// dump C2C results to CSV
	dumpRegistrationCSV(output);

	// append I2S results to CSV
	dumpPlacementCSV(output);
}