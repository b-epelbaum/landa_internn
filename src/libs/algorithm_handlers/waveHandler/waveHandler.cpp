#include "waveHandler.h"
#include "applog.h"
#include "util.h"

using namespace LandaJune;
using namespace Algorithms;
using namespace Parameters;
using namespace Threading;
using namespace Core;

static const QString wave_HANDLER_NAME = "Offline Reader Wave Handler";
static const QString wave_HANDLER_DESC = "Processes offline wave images";

#define WAVE_HANDLER_SCOPED_LOG PRINT_INFO5 << "[wavePageHandler] : "
#define WAVE_HANDLER_SCOPED_ERROR PRINT_ERROR << "[wavePageHandler] : "
#define WAVE_HANDLER_SCOPED_WARNING PRINT_WARNING << "[wavePageHandler] : "

// naming convention
// source : c:\Temp\offline\offline\11_781_wave\GeometricRegInf85_13\GeometricRegInf85_13layoutImg.bmp 
// target folder <root_folder>\11_Reg_Left
// file name : <Frame_ID>_<ImageIndex>_EDGE_LEFT
// file name for ROIs : <Frame_ID>_<ImageIndex>_C2C_LEFT_00_[x,y].bmp

wavePageHandler::wavePageHandler()
{
	WAVE_HANDLER_SCOPED_LOG << "created";
}

wavePageHandler::~wavePageHandler()
{
	WAVE_HANDLER_SCOPED_LOG << "destroyed";
}

std::unique_ptr<IAlgorithmHandler> wavePageHandler::clone()
{
	return std::move(std::make_unique<wavePageHandler>(*this));
}

QString wavePageHandler::getName() const
{
	return wave_HANDLER_NAME;
}

QString wavePageHandler::getDescription() const
{
	return wave_HANDLER_DESC;
}

std::string wavePageHandler::getFrameFolderName()  const 
{
	//11_Reg_Left
	return std::move(fmt::format("{0}_Wave", _sourceFrameNumber));
}



std::shared_ptr<BaseParameters> wavePageHandler::getParameters() const
{
	return std::static_pointer_cast<BaseParameters>(_processParameters);
}

void wavePageHandler::init(std::shared_ptr<BaseParameters> parameters)
{
	validateProcessParameters(parameters);
	createCSVFolder();

	// template image ( temporary solution )
	QFile templateTif(":/templates/Resources/Template1.tif");
	
	if (templateTif.open(QFile::ReadOnly))
		_processParameters->setCircleTemplateBuffer(templateTif.readAll());

	const INIT_PARAMETER i2sInitParam{ toROIRect(_processParameters->I2SApproximateTriangleRectLeft()) };
	initI2S(i2sInitParam);

	const INIT_PARAMETER waveParam{ toROIRect(_processParameters->WaveTriangleROIRect()) };
	initWave(waveParam);

}

void wavePageHandler::cleanup()
{
	shutdownI2S();
	shutdownWave();
}

void wavePageHandler::validateProcessParameters(std::shared_ptr<BaseParameters> parameters)
{
	_processParameters = std::dynamic_pointer_cast<ProcessParameters>(parameters);
}

void wavePageHandler::process(const FrameRef * frame)
{
	// call general process implementation of parent class
	abstractAlgoHandler::process(frame);

	// get source frame ID from custom parameter passed by provider
	_sourceFrameNumber.clear();
	try
	{
		auto framePath = std::any_cast<std::string>(frame->getNamedParameter("srcPath"));
		
		//c:/temp/offline/10_780_wave/GeometricRegInf85_12/GeometricRegInf85_12layoutImg.bmp

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
		WAVE_HANDLER_SCOPED_ERROR << "Cannot retrieve source image file name. Error : " << e.what() << "; Resetting to default...";
	}

	if ( _sourceFrameNumber.empty())
		_sourceFrameNumber = std::to_string(_frameIndex);
	else
		_frameIndex = std::stoi(_sourceFrameNumber);

	std::vector<PARAMS_WAVE_INPUT> waveInputs;
	IMAGE_REGION_LIST regionList;

	for (const auto& color : _processParameters->ColorArray() )
	{
		PARAMS_WAVE_INPUT input(_frame);
		fillWaveProcessParameters(input);
		input._circleColor = color2HSV(color);
		
		generateWaveRegions(input, regionList, _processParameters->DumpWaveROI() && waveInputs.empty());
		waveInputs.emplace_back(std::move(input));
	}

	// and perform a deep copy
	copyRegions(regionList);

	//process wave ROI n times ( n = number of colors )
	for (const auto& wave : waveInputs )
	const auto output = std::move(processWave(wave));

	// dump C2C results to CSV
	//dumpwaveCSV(output);

	// append I2S results to CSV
	//dumpPlacementCSV(output);
}