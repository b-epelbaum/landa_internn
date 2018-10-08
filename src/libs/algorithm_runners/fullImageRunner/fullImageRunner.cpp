#include "fullImageRunner.h"
#include "applog.h"
#include "RealTimeStats.h"

#include <filesystem>

using namespace LandaJune;
using namespace Algorithms;
using namespace Parameters;
using namespace Helpers;
using namespace Core;
using namespace Files;

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
	FULLIMAGE_RUNNER_SCOPED_LOG << "Root object created";
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


std::string fullImageRunner::getFrameFolderName()  const 
{
	if (_bOfflineSource && !_sourceFramePath.empty() )
	{
		auto const strFolder = QFileInfo(QString::fromStdString(_sourceFramePath)).absoluteDir().dirName();
		if ( strFolder.isEmpty() )
			return std::move(QFileInfo(QString::fromStdString(_sourceFramePath)).absoluteDir().dirName().toStdString());
	}
	//\\Frame_<FrameID>_<ImageIndex>_algo_name
	return std::move(fmt::format("Frame_{0}_{1}_full_page", _frameIndex, _imageIndex));
}


BaseParametersPtr fullImageRunner::getParameters() const
{
	return std::static_pointer_cast<BaseParameters>(_processParameters);
}

void fullImageRunner::init(BaseParametersPtr parameters, Core::ICore* coreObject, FrameConsumerCallback callback)
{
	validateProcessParameters(parameters);
	_coreObject = coreObject;
	_callback = callback;

	if (_processParameters->EnableAnyDataSaving() && _processParameters->EnableCSVSaving() &&
		(_processParameters->SaveC2CRegistrationCSV() || _processParameters->SaveI2SPlacementCSV() || _processParameters->SaveWaveCSV())
		)
	{
		_csvOutFolder = createCSVFolder(_processParameters);
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

void fullImageRunner::validateProcessParameters(BaseParametersPtr parameters)
{
	_processParameters = std::dynamic_pointer_cast<ProcessParameters>(parameters);
}

void fullImageRunner::processInternal()
{
	if ( !_processParameters->EnableProcessing() )
		return;

	const auto input = std::make_shared<PARAMS_C2C_SHEET_INPUT>(_frame);

	try
	{
		// fill process parameters
		setupSheetProcessParameters(input);

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

		// process whoe sheet and output data
		processSheetOutput(processSheet(input));
	}
	catch ( BaseException& bex)
	{
		throw;
	}
}


//////////////////////////////////////////////////////////
////////////// PROCESSING OUTPUTS
//////////////////////////////////////////////////////////

void fullImageRunner::processSheetOutput(PARAMS_C2C_SHEET_OUTPUT_PTR sheetOutput)
{
	if ( _processParameters->ProcessWave())
		processWaveOutputs(sheetOutput->_waveOutputs, sheetOutput->_waveTriangleOutput);

}

void fullImageRunner::processStripOutput(PARAMS_C2C_STRIP_OUTPUT_PTR stripOutput )
{
	// update C2C centers with I2S coordinates offsets
	auto const offsetX = stripOutput->_i2sOutput->_triangeCorner._x;
	auto const offsetY = stripOutput->_i2sOutput->_triangeCorner._y;

	auto const updateCoordsLambda = [&](auto &c2cOut) 
	{
		std::for_each(c2cOut->_colorCenters.begin(), c2cOut->_colorCenters.end(), [&](auto &colorCenter)
		{
			colorCenter._x -= offsetX;
			colorCenter._y -= offsetY;
		});
	};


	if (_bParallelCalc)
	{
		parallel_for_each(begin(stripOutput->_c2cROIOutputs), end(stripOutput->_c2cROIOutputs), updateCoordsLambda);
	}
	else
	{
		for_each(begin(stripOutput->_c2cROIOutputs), end(stripOutput->_c2cROIOutputs), updateCoordsLambda);
	}

	stripOutput->_result = 
	std::all_of(begin(stripOutput->_c2cROIOutputs), end(stripOutput->_c2cROIOutputs), 
			[](auto& out)
			{
				return out->_result == ALG_STATUS_SUCCESS;
			})
			? ALG_STATUS_SUCCESS
			: ALG_STATUS_FAILED;

	if (stripOutput->_result != ALG_STATUS_SUCCESS )
	{
		// if strip handling failed, write to log
		logFailedStrip(stripOutput);

		// TODO : dump failed strip to file
	}

	if ( _processParameters->EnableAnyDataSaving() &&  _processParameters->EnableCSVSaving()  )
	{
		processStripOutputCSV (stripOutput);
	}
}

void fullImageRunner::processStripOutputCSV(PARAMS_C2C_STRIP_OUTPUT_PTR stripOutput)
{
	auto const jobID = _processParameters->JobID();
	auto const frameIndex = _frameIndex;
	auto const imgIndex = _imageIndex;
	auto const csvFolder = _csvOutFolder;
	auto const bAsyncWrite = _bAsyncWrite;
	auto const sourceFilePath = _sourceFramePath;

	auto const dumpCSVRegLambda = [=]()
	{
		dumpRegistrationCSV(stripOutput, jobID, frameIndex, imgIndex, csvFolder, sourceFilePath, bAsyncWrite );
	};

	auto const dumpCSVPlacementLambda = [=]()
	{
		dumpPlacementCSV(stripOutput, frameIndex, imgIndex, csvFolder, bAsyncWrite  );
	};

	if (_bParallelCalc)
	{
		if (_processParameters->SaveC2CRegistrationCSV())
		{
			task<void> stripOutTaskReg(dumpCSVRegLambda);
		}

		if (_processParameters->SaveI2SPlacementCSV())
		{
			task<void> stripOutTaskPlacement(dumpCSVPlacementLambda);
		}
	}
	else
	{
		if (_processParameters->SaveC2CRegistrationCSV())
		{
			dumpCSVRegLambda();
		}
		if (_processParameters->SaveI2SPlacementCSV())
		{
			dumpCSVPlacementLambda();
		}
	}
}

void fullImageRunner::sortWaveOutputs( concurrent_vector<PARAMS_WAVE_OUTPUT_PTR> & waveOutputs )
{
	static std::map<std::string, int> colorOrderMap =
	{
		  {"black", 0}
		, {"cyan", 1}
		, {"magenta", 2}
		, {"yellow", 3}
	};

	std::sort(waveOutputs.begin(), waveOutputs.end(),   
			[](const PARAMS_WAVE_OUTPUT_PTR& left, const PARAMS_WAVE_OUTPUT_PTR& right) 
			{
				auto lV (left->_input->_circleColor._colorName);
				auto rV (right->_input->_circleColor._colorName);

				std::transform(lV.begin(), lV.end(), lV.begin(), ::tolower);
				std::transform(rV.begin(), rV.end(), rV.begin(), ::tolower);

				auto const lIt = colorOrderMap.find(lV);
				auto const rIt = colorOrderMap.find(rV);

				return lIt != colorOrderMap.end() && rIt != colorOrderMap.end() 
								? colorOrderMap.find(lV)->second <  colorOrderMap.find(rV)->second 
								: false;
			});
}

void fullImageRunner::processWaveOutputs( concurrent_vector<PARAMS_WAVE_OUTPUT_PTR> & waveOutputs, PARAMS_I2S_OUTPUT_PTR waveTriangleOutput  )
{
	// sort wave outputs
	sortWaveOutputs(waveOutputs);

	const auto sumProcLambda = [&](auto &singleOut) 
	{
		auto & resArray = singleOut->_colorDetectionResults;
		if (static_cast<int>(resArray.size()) > _processParameters->WaveNumberOfColorDotsPerLine())
		{
			singleOut->_result = ALG_STATUS_FAILED;
			return;
		}
			
		if (static_cast<int>(resArray.size()) < _processParameters->WaveNumberOfColorDotsPerLine())
		{
			auto & resPoints = singleOut->_colorCenters;
			resArray.resize(_processParameters->WaveNumberOfColorDotsPerLine(), ALG_STATUS_FAILED );
			resPoints.resize(_processParameters->WaveNumberOfColorDotsPerLine(), {-1,-1 } );
			singleOut->_result = ALG_STATUS_FAILED;
			return;
		}

		singleOut->_result = 
			std::all_of(resArray.begin(), resArray.end(), [](auto& singleResult) { return singleResult == ALG_STATUS_SUCCESS; } )
			? ALG_STATUS_SUCCESS
			: ALG_STATUS_FAILED;
	};


	if (_bParallelCalc)
	{
		parallel_for_each(begin(waveOutputs), end(waveOutputs), sumProcLambda );
	}
	else
	{
		for_each(begin(waveOutputs), end(waveOutputs), sumProcLambda ); 
	}

	const auto allResult = std::all_of(waveOutputs.begin(), waveOutputs.end(), [](auto& singleWave)
	{
		return singleWave->_result == ALG_STATUS_SUCCESS;
	})
			? ALG_STATUS_SUCCESS
			: ALG_STATUS_FAILED;

	if (allResult != ALG_STATUS_SUCCESS )
	{
		// if strip handling failed, write to log
		logFailedWaves(waveOutputs);

		// TODO : dump failed waves to file
	}

	// process and save
	if ( _processParameters->EnableAnyDataSaving() &&  _processParameters->EnableCSVSaving())
	{
		processWaveOutputsCSV (waveOutputs);
	}
}

void fullImageRunner::processWaveOutputsCSV(concurrent_vector<PARAMS_WAVE_OUTPUT_PTR> & waveOutputs )
{
	auto const jobID = _processParameters->JobID();
	auto const frameIndex = _frameIndex;
	auto const imgIndex = _imageIndex;
	auto const csvFolder = _csvOutFolder;
	auto const bAsyncWrite = _bAsyncWrite;
	auto const sourceFilePath = _sourceFramePath;

	const auto saveWaveCSVLambda = [=]()
	{
		dumpWaveCSV(waveOutputs, jobID, frameIndex, imgIndex, csvFolder, sourceFilePath, bAsyncWrite );
	};

	if (_bParallelCalc)
	{
		if (_processParameters->SaveWaveCSV())
		{
			task<void> tWaveCSV(saveWaveCSVLambda);
		}
		else
		{
			if (_processParameters->SaveWaveCSV())
			{
				saveWaveCSVLambda();
			}
		}
	}
}

void fullImageRunner::logFailedStrip(PARAMS_C2C_STRIP_OUTPUT_PTR stripOutput)
{
	std::ostringstream ss;
	ss << "--------------  Strip analysis failed -----------------\r\n"
					<< "\t\t\t\t\tFrame Index  : \t" << _frameIndex			<< "\r\n"
					<< "\t\t\t\t\tSource image : \t" << _sourceFramePath	<< "\r\n";

	FULLIMAGE_RUNNER_SCOPED_ERROR << ss.str().c_str();
}

void fullImageRunner::logFailedWaves(concurrent_vector<PARAMS_WAVE_OUTPUT_PTR> & waveOutputs)
{
	std::ostringstream ss;
	ss << "--------------  Wave analysis failed -----------------\r\n"
					<< "\t\t\t\t\tFrame Index  : \t" << _frameIndex			<< "\r\n"
					<< "\t\t\t\t\tSource image : \t" << _sourceFramePath	<< "\r\n";

	FULLIMAGE_RUNNER_SCOPED_ERROR << ss.str().c_str();
}