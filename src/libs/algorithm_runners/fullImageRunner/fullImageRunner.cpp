#include "fullImageRunner.h"
#include "applog.h"
#include "RealTimeStats.h"

using namespace LandaJune;
using namespace Algorithms;
using namespace Parameters;
using namespace Helpers;
using namespace Core;

static const QString FULL_IMAGE_RUNNER_NAME = "Full Image Runner";
static const QString FULL_IMAGE_RUNNER_DESC = "Full Scanned Page Algorithm Set";

#define FULLIMAGE_RUNNER_SCOPED_LOG PRINT_INFO2 << "[fullImageRunner] : "
#define FULLIMAGE_RUNNER_SCOPED_ERROR PRINT_ERROR << "[fullImageRunner] : "
#define FULLIMAGE_RUNNER_SCOPED_WARNING PRINT_WARNING << "[fullImageRunner] : "

// naming convention
// source : JobID, Frame ID, ImageIndex (%11)
// target folder <root_folder>\JobID\\Frame_<FrameID>_<ImageIndex>_algo_name
// file name : <Frame_ID>_<ImageIndex>_EDGE_LEFT
// file name for ROIs : <Frame_ID>_<ImageIndex>_C2C_LEFT_00_[x,y].bmp


fullImageRunner::fullImageRunner()
{
	FULLIMAGE_RUNNER_SCOPED_LOG << "created";
}

fullImageRunner::~fullImageRunner()
{
	//FULLIMAGE_RUNNER_SCOPED_LOG << "destroyed";
}

std::unique_ptr<IAlgorithmRunner> fullImageRunner::clone()
{
	return std::make_unique<fullImageRunner>(*this);
}

QString fullImageRunner::getName() const
{
	return FULL_IMAGE_RUNNER_NAME;
}

QString fullImageRunner::getDescription() const
{
	return FULL_IMAGE_RUNNER_DESC;
}

std::string fullImageRunner::parseSourceFrameIndexString(const std::string& strPath)
{
	return {};
}

std::string fullImageRunner::getFrameFolderName()  const 
{
	//\\Frame_<FrameID>_<ImageIndex>_algo_name
	return std::move(fmt::format("Frame_{0}_{1}_full_page", _frameIndex, _imageIndex));
}


std::shared_ptr<BaseParameters> fullImageRunner::getParameters() const
{
	return std::static_pointer_cast<BaseParameters>(_processParameters);
}

void fullImageRunner::init(std::shared_ptr<BaseParameters> parameters)
{
	validateProcessParameters(parameters);
	if (_processParameters->SaveC2CRegistrationCSV() || _processParameters->SaveI2SPlacementCSV() || _processParameters->SaveWaveCSV())
	{
		_csvFolder = createCSVFolder(_processParameters);
	}

	// C2C template image ( temporary solution ) - read from resources
	QFile templateTifC2C(QString(":/templates/Resources/%1").arg(_processParameters->CircleTemplateResourceC2C()));
	
	if (templateTifC2C.open(QFile::ReadOnly))
		_processParameters->setCircleTemplateBufferC2C(templateTifC2C.readAll());
	else
	{
		FULLIMAGE_RUNNER_SCOPED_WARNING << "C2C Circle template cannot be read : " << _processParameters->CircleTemplateResourceC2C();
	}

	// Wave template image ( temporary solution ) - read from resources
	QFile templateTifWave(QString(":/templates/Resources/%1").arg(_processParameters->CircleTemplateResourceWave()));
	
	if (templateTifWave.open(QFile::ReadOnly))
		_processParameters->setCircleTemplateBufferWave(templateTifWave.readAll());
	else
	{
		FULLIMAGE_RUNNER_SCOPED_WARNING << "Wave Circle template cannot be read : " << _processParameters->CircleTemplateResourceWave();
	}

	const INIT_PARAMETER edgeInitParam ( toROIRect(_processParameters->LeftStripRect()) );
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
		FULLIMAGE_RUNNER_SCOPED_WARNING << "No C2C ROI defined !";
	}

	const INIT_PARAMETER waveInitParam{ toROIRect(_processParameters->WaveROI()) };
	initWave(waveInitParam);
}

void fullImageRunner::cleanup()
{
	shutdownEdge();
	shutdownI2S();
	shutdownC2CRoi();
	shutdownWave();
}

void fullImageRunner::validateProcessParameters(std::shared_ptr<BaseParameters> parameters)
{
	_processParameters = std::dynamic_pointer_cast<ProcessParameters>(parameters);
}

void fullImageRunner::process(const FrameRef * frame)
{
	if ( !_processParameters->EnableProcessing() )
		return;

	// call general process implementation of parent class
	baseAlgorithmRunner::process(frame);

	const auto input = std::make_shared<PARAMS_C2C_SHEET_INPUT>(_frame);
	
	// fill process parameters
	fillSheetProcessParameters(input);

	// generate ROIs for all required elements
	auto tStart = Utility::now_in_microseconds();
	IMAGE_REGION_LIST regionList;
	generateSheetRegions(input, regionList);
	RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_regionsGeneratedOk, (static_cast<double>(Utility::now_in_microseconds()) - static_cast<double>(tStart)) / 1000);

	// and perform a deep copy
	tStart = Utility::now_in_microseconds();
	copyRegions(regionList);
	RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_regionsCopiedOk, (static_cast<double>(Utility::now_in_microseconds()) - static_cast<double>(tStart)) / 1000);

	if ( !_processParameters->EnableAlgorithmProcessing() )
		return;

	// process whole sheet
	auto output = processSheet(input);
}