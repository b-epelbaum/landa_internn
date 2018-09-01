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

// naming convention
// source : JobID, Frame ID, ImageIndex (%11)
// target folder <root_folder>\JobID\\Frame_<FrameID>_<ImageIndex>_algo_name
// file name : <Frame_ID>_<ImageIndex>_EDGE_LEFT
// file name for ROIs : <Frame_ID>_<ImageIndex>_C2C_LEFT_00_[x,y].bmp


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
	return std::move(std::make_unique<fullPageHandler>(*this));
}

QString fullPageHandler::getName() const
{
	return FULL_PAGE_HANDLER_NAME;
}

QString fullPageHandler::getDescription() const
{
	return FULL_PAGE_HANDLER_DESC;
}

std::string fullPageHandler::getFrameFolderName()  const 
{
	//\\Frame_<FrameID>_<ImageIndex>_algo_name
	return std::move(fmt::format("Frame_{0}_{1}_full_page", _frameIndex, _imageIndex));
}


std::shared_ptr<BaseParameters> fullPageHandler::getParameters() const
{
	return std::static_pointer_cast<BaseParameters>(_processParameters);
}

void fullPageHandler::init(std::shared_ptr<BaseParameters> parameters)
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

void fullPageHandler::validateProcessParameters(std::shared_ptr<BaseParameters> parameters)
{
	_processParameters = std::dynamic_pointer_cast<ProcessParameters>(parameters);
}

void fullPageHandler::process(const FrameRef * frame)
{
	// call general process implementation of parent class
	abstractAlgoHandler::process(frame);

	PARAMS_C2C_SHEET_INPUT input(_frame);
	
	// fill process parameters
	fillSheetProcessParameters(input);

	// generate ROIs for all required elements
	IMAGE_REGION_LIST regionList;
	generateSheetRegions(input, regionList);

	// and perform a deep copy
	copyRegions(regionList);

	// process whole sheet
	auto output = processSheet(input);

	// dump C2C results to CSV
	if (_processParameters->ProcessRightSide())
	{
		if (_bParallelizeCalculations)
		{
			TaskThreadPools::postJob(TaskThreadPools::algorithmsThreadPool(), &fullPageHandler::dumpRegistrationCSV, this, output._stripOutputParameterLeft );
		}
		else
			dumpRegistrationCSV(output._stripOutputParameterLeft);
	}

	dumpRegistrationCSV(output._stripOutputParameterLeft);


	// append I2S results to CSV
	dumpPlacementCSV(output._stripOutputParameterLeft);
	if (_processParameters->ProcessRightSide())
	{
		dumpPlacementCSV(output._stripOutputParameterRight);
	}
}