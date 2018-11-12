#include "avtStripRunner.h"
#include "applog.h"
#include "RealTimeStats.h"

#include <filesystem>

using namespace LandaJune;
using namespace Algorithms;
using namespace Parameters;
using namespace Helpers;
using namespace Core;
using namespace Files;

static const QString AVT_STRIP_RUNNER_NAME = "AVT Strip Runner";
static const QString AVT_STRIP_RUNNER_DESC = "Runner for handling AVT-cut strip images only";

#define AVT_STRIP_RUNNER_SCOPED_LOG PRINT_INFO2 << "[AVTStripRunner] : "
#define AVT_STRIP_RUNNER_SCOPED_ERROR PRINT_ERROR << "[AVTStripRunner] : "
#define AVT_STRIP_RUNNER_SCOPED_WARNING PRINT_WARNING << "[AVTStripRunner] : "

// naming convention
// source : JobID, Frame ID, ImageIndex (%11)
// target folder <root_folder>\JobID\\Frame_<FrameID>_<ImageIndex>_algo_name
// file name : <Frame_ID>_<ImageIndex>_EDGE_LEFT
// file name for ROIs : <Frame_ID>_<ImageIndex>_C2C_LEFT_00_[x,y].bmp


//static int testVal = 0;


avtStripRunner::avtStripRunner()
{
	AVT_STRIP_RUNNER_SCOPED_LOG << "Root object created";
}


std::unique_ptr<IAlgorithmRunner> avtStripRunner::clone()
{
	return std::make_unique<avtStripRunner>(*this);
}

QString avtStripRunner::getName() const
{
	return AVT_STRIP_RUNNER_NAME;
}

QString avtStripRunner::getDescription() const
{
	return AVT_STRIP_RUNNER_DESC;
}


std::string avtStripRunner::getFrameFolderName() 
{
	if (_targetFrameFolder.empty())
	{
		if (!_sourceFramePath.empty() )
		{
			// parse source path of form :
			// d:/Temp/JUNE/june_offline_input/JIG_300_default/1_911_Registration/GeometricRegInf28_15/GeometricRegInf28_15layoutImg.bmp

			QStringList elements  = QDir::fromNativeSeparators(QString::fromStdString(_sourceFramePath)).split('/');


			if (elements.size() < 4)
			{
				THROW_EX_ERR_STR(CORE_ERROR::ALGO_AVT_SOURCE_PATH_INVALID, "AVT source frame path cannot be parsed : " + _sourceFramePath);
			}

			auto const sourceChunk = elements[elements.size() - 3];
			if ( sourceChunk.isEmpty())
			{
				THROW_EX_ERR_STR(CORE_ERROR::ALGO_AVT_SOURCE_PATH_INVALID, "AVT source frame path cannot be parsed : " + _sourceFramePath);

			}

			std::string const sideName = (_bIsLeftStrip) ? "left" : "right";
			_targetFrameFolder = fmt::format("{0}_{1}", sourceChunk.toStdString(), sideName);
		}
		else
			_targetFrameFolder = fmt::format("{0}_{1}_images", _frameIndex, _imageIndex);
	}
	return _targetFrameFolder;
}

void avtStripRunner::parseFrameAndImageIndex()
{
	auto const targetFolder = QString::fromStdString(getFrameFolderName());

	QStringList parts = targetFolder.split('_');
	if (parts.size() < 2)
	{
		THROW_EX_ERR_STR(CORE_ERROR::ALGO_AVT_SOURCE_PATH_INVALID, "AVT source frame index cannot be parsed : " + _targetFrameFolder);
	}

	bool bresult = false;
	_frameIndex = parts[0].toInt(&bresult);
	if (!bresult)
	{
		THROW_EX_ERR_STR(CORE_ERROR::ALGO_AVT_SOURCE_PATH_INVALID, "AVT source frame index cannot be parsed ( numeric part ): " + _targetFrameFolder);
	}

	_imageIndex = _frameIndex % _processParameters->PanelCount();
	if (_imageIndex == 0 )
		_imageIndex = _processParameters->PanelCount();
}

BaseParametersPtr avtStripRunner::getParameters() const
{
	return std::static_pointer_cast<BaseParameters>(_processParameters);
}

void avtStripRunner::init(BaseParametersPtr parameters, Core::ICore* coreObject, CoreEventCallback callback)
{
	try
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
			AVT_STRIP_RUNNER_SCOPED_WARNING << "C2C Circle template cannot be read : " << _processParameters->CircleTemplateResourceC2C();
			THROW_EX_ERR_STR (CORE_ERROR::ALGO_C2C_IMAGE_TEMPLATE_INVALID, "C2C Circle template cannot be read : " +  _processParameters->CircleTemplateResourceC2C().toStdString() );
		}

		const INIT_PARAMETER edgeInitParam ( toROIRect(_processParameters->LeftStripRect_px()) );
		initEdge(edgeInitParam);

		const INIT_PARAMETER i2sInitParam{ toROIRect(_processParameters->I2SRectLeft_px()) };
		initI2S(i2sInitParam);

		if (!_processParameters->C2CROIArrayLeft_px().empty())
		{
			const INIT_PARAMETER c2croiInitParam{ toROIRect(_processParameters->C2CROIArrayLeft_px()[0]) };
			initC2CRoi(c2croiInitParam);
		}
		else
		{
			AVT_STRIP_RUNNER_SCOPED_WARNING << "No C2C ROI defined !";
		}
	}
	catch(BaseException& bex)
	{
		throw;
	}
	catch ( std::exception& ex)
	{
		RETHROW( CORE_ERROR::ERR_RUNNER_FAILED_TO_INIT);
	}
}

void avtStripRunner::cleanup()
{
	shutdownEdge();
	shutdownI2S();
	shutdownC2CRoi();

	_coreObject = nullptr;
	_callback = nullptr;
	_processParameters.reset();
}

void avtStripRunner::validateProcessParameters(BaseParametersPtr parameters)
{
	_processParameters = std::dynamic_pointer_cast<ProcessParameters>(parameters);
	if (_processParameters == nullptr )
	{
		THROW_EX_ERR(CORE_ERROR::ALGO_INVALID_PARAMETER_TYPE_PASSED);
	}
}

CORE_ERROR avtStripRunner::processInternal()
{
	try
	{
		if ( !_processParameters->EnableProcessing() )
			return RESULT_OK;

		// detect side of the strip
		_bIsLeftStrip = isLeftStrip();

		//generate fram eindex from source path
		parseFrameAndImageIndex();

		const auto input = std::make_shared<PARAMS_C2C_SHEET_INPUT>(_frame);

		// fill process parameters
		// throws an exception in case of error
		setupSheetProcessParameters(input);

		// generate ROIs for all required elements
		// throws an exception in case of error
		auto tStart = Utility::now_in_microseconds();
		IMAGE_REGION_LIST regionList;
		generateSheetRegions(input, regionList);
		RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_regionsGenerated, (static_cast<double>(Utility::now_in_microseconds()) - static_cast<double>(tStart)) / 1000);

		// and perform a deep copy
		// throws an exception in case of error
		tStart = Utility::now_in_microseconds();
		copyRegions(regionList);
		RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_regionsCopied, (static_cast<double>(Utility::now_in_microseconds()) - static_cast<double>(tStart)) / 1000);

		if ( !_processParameters->EnableAlgorithmProcessing() )
			return RESULT_OK;

		// process whole sheet and output data
		return processSheetOutput(processSheet(input));
	}
	catch(BaseException& bex)
	{
		throw;
	}
	catch ( std::exception& ex)
	{
		RETHROW( CORE_ERROR::ALGO_PROCESS_UNCAUGHT_EXCEPTION);
	}
}


//////////////////////////////////////////////////////////
////////////// PROCESSING OUTPUTS
//////////////////////////////////////////////////////////

CORE_ERROR avtStripRunner::processSheetOutput(PARAMS_C2C_SHEET_OUTPUT_PTR sheetOutput)
{
	try
	{
		auto processResult = sheetOutput->_result;
		return ( processResult == ALG_STATUS_SUCCESS ) ? RESULT_OK : CORE_ERROR::ERR_RUNNER_ANALYSIS_FAILED;
	}
	catch(BaseException& bex)
	{
		throw;
	}
	catch ( std::exception& ex)
	{
		RETHROW( CORE_ERROR::ALGO_PROCESS_SHEET_OUTPUT_FAILED);
	}
}

void avtStripRunner::processStripOutput(PARAMS_C2C_STRIP_OUTPUT_PTR stripOutput )
{
	try
	{
		// update C2C centers with I2S coordinates offsets
		// Both I2S and C2C ROIs passed as pixel units, so offset should be relative ( in microns )
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
			parallel_for_each(std::begin(stripOutput->_c2cROIOutputs), std::end(stripOutput->_c2cROIOutputs), updateCoordsLambda);
		}
		else
		{
			std::for_each(std::begin(stripOutput->_c2cROIOutputs), std::end(stripOutput->_c2cROIOutputs), updateCoordsLambda);
		}

		// now update I2S corner ( on left side only )
		if ( _bIsLeftStrip)
		{
			 stripOutput->_i2sOutput->_triangeCorner._x -= _processParameters->ScanStartToPaperEdgeOffset_mm() * 1000;
		}

		stripOutput->_result = 
		std::all_of(std::begin(stripOutput->_c2cROIOutputs), std::end(stripOutput->_c2cROIOutputs), 
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
	catch(BaseException& bex)
	{
		throw;
	}
	catch ( std::exception& ex)
	{
		RETHROW( CORE_ERROR::ALGO_PROCESS_STRIP_OUTPUT_FAILED);
	}
}

bool avtStripRunner::shouldProcessLeftStrip() const
{
	return _bIsLeftStrip;
}

bool avtStripRunner::shouldProcessRightStrip() const
{
	return !_bIsLeftStrip;
}

void avtStripRunner::processStripOutputCSV(PARAMS_C2C_STRIP_OUTPUT_PTR stripOutput)
{
	try
	{
		auto const jobID = _processParameters->JobID();
		auto const frameIndex = _frameIndex;
		auto const imgIndex = _imageIndex;
		
		auto const csvFolder = _csvOutFolder;
		auto const bAsyncWrite = _bAsyncWrite;
		auto const sourceFilePath = _sourceFramePath;

		auto const dumpCSVRegLambda = [=]()
		{
			dumpRegistrationCSV(stripOutput, jobID.toStdString(), frameIndex, imgIndex, csvFolder, sourceFilePath, bAsyncWrite );
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
	catch(BaseException& bex)
	{
		throw;
	}
	catch ( std::exception& ex)
	{
		RETHROW( CORE_ERROR::ALGO_PROCESS_STRIP_OUTPUT_CSV_FAILED);
	}
}

void avtStripRunner::logFailedStrip(PARAMS_C2C_STRIP_OUTPUT_PTR stripOutput)
{
	std::ostringstream ss;
	ss << "--------------  Strip analysis failed -----------------\r\n"
					<< "\t\t\t\t\tFrame Index  : \t" << _frameIndex			<< "\r\n"
					<< "\t\t\t\t\tSource image : \t" << _sourceFramePath	<< "\r\n";

	AVT_STRIP_RUNNER_SCOPED_ERROR << ss.str().c_str();
}

bool avtStripRunner::isLeftStrip()
{
	if  (!_frame->hasNamedParameter(NAMED_PROPERTY_FRAME_PARITY) )
		return true;

	auto frameParity = 1;
	try
	{
		frameParity = std::any_cast<int>(_frame->getNamedParameter(NAMED_PROPERTY_FRAME_PARITY));
	}
	catch (const std::bad_any_cast& e)
	{
	}

	return frameParity == 1;
}