#pragma once

#include "../interfaces/IAlgorithmRunner.h"

#include "algorithm_parameters.h"
#include "ProcessParameters.h"
#include "frameRef.h"
#include "typeConverters.hpp"


///////////////
//  OPEN CV
#include <opencv/cv.h> 
#include <opencv2/imgcodecs.hpp>
#include "applog.h"
#include <opencv2/imgcodecs/imgcodecs_c.h>
#include "util.h"
#include "ImageRegion.h"


#ifdef _DEBUG
#pragma comment(lib, "opencv_world342d.lib")
#else
#pragma comment(lib, "opencv_world342.lib")
#endif

namespace LandaJune {
	namespace Core {
		class FrameRef;
	}
}

namespace LandaJune
{
	namespace Algorithms
	{
		class baseAlgorithmRunner : public QObject, public IAlgorithmRunner
		{
			Q_OBJECT
			
			friend class IAlgorithmRunner;

		public:
			baseAlgorithmRunner() = default;
			baseAlgorithmRunner(const baseAlgorithmRunner & other)
				: _frame (other._frame)
				, _frameIndex (other._frameIndex)
				, _imageIndex (other._imageIndex)
				, _csvFolder (other._csvFolder)
				, _processParameters (other._processParameters)
				, _bParallelCalc (other._bParallelCalc)
			{
			}

			baseAlgorithmRunner(baseAlgorithmRunner &&) = delete;
			virtual ~baseAlgorithmRunner() = default;

			const baseAlgorithmRunner & operator = (const baseAlgorithmRunner &) = delete;
			baseAlgorithmRunner & operator = (baseAlgorithmRunner &&) = delete;

			void init(std::shared_ptr<Parameters::BaseParameters> parameters) override;
			void process(const Core::FrameRef * frame) override;

			std::shared_ptr<Parameters::BaseParameters> getParameters() const override { return _processParameters; }

		protected:

			virtual void validateProcessParameters(std::shared_ptr<Parameters::BaseParameters> parameters) = 0;

			virtual void fillCommonProcessParameters(ABSTRACT_INPUT& input);
			virtual void fillSheetProcessParameters(PARAMS_C2C_SHEET_INPUT& input);
			virtual void fillStripProcessParameters(PARAMS_C2C_STRIP_INPUT& input, SHEET_SIDE side);
			virtual void fillEdgeProcessParameters(PARAMS_PAPEREDGE_INPUT& input, SHEET_SIDE side);
			virtual void fillI2SProcessParameters(PARAMS_I2S_INPUT& input, SHEET_SIDE side);
			virtual void fillC2CProcessParameters(PARAMS_C2C_ROI_INPUT& input, SHEET_SIDE side);
			virtual void fillWaveProcessParameters(std::vector<PARAMS_WAVE_INPUT>& inputs);

			virtual void generateSheetRegions(PARAMS_C2C_SHEET_INPUT& input, IMAGE_REGION_LIST& regionList) const;
			virtual void generateStripRegions(PARAMS_C2C_STRIP_INPUT& input, IMAGE_REGION_LIST& regionList) const;
			virtual void generateI2SRegion(PARAMS_I2S_INPUT& input, IMAGE_REGION_LIST& regionList) const;
			virtual void generateC2CRegion(PARAMS_C2C_ROI_INPUT& input, IMAGE_REGION_LIST& regionList) const;
			virtual void generateWaveRegion(PARAMS_WAVE_INPUT& input, IMAGE_REGION_LIST& regionList, bool bDumpWave ) const;

			virtual void copyRegions(IMAGE_REGION_LIST& regionList );

			template<typename T>
			void dumpOverlay(T& out)
			{
				if (_processParameters->EnableAnyDataSaving() && _processParameters->EnableImageSaving() && _processParameters->GenerateOverlays())
				{
					const auto& targetMat = out.overlay();
					if ( targetMat != std::nullopt )
						dumpMatFile(targetMat.value(), generateFullPathForElement<T>(out), _bParallelCalc);
				}
			}

			virtual void dumpRegistrationCSV(const PARAMS_C2C_STRIP_OUTPUT& stripOut);
			virtual void dumpPlacementCSV(const PARAMS_C2C_STRIP_OUTPUT& stripOut);

			virtual std::string getBatchRootFolder() const;
			virtual std::string getFrameFolderName() const;
			virtual std::string getElementPrefix() const;
			virtual std::string generateFullPathForElement(const std::string& elementName, const std::string& ext = "bmp" ) const;
			virtual std::string generateFullPathForRegCSV(const PARAMS_C2C_STRIP_OUTPUT& out) const;
			virtual std::string generateFullPathForPlacementCSV(SHEET_SIDE side) const;
			virtual void getSourceFrameIndexString();
			virtual std::string parseSourceFrameIndexString(const std::string& strPath) = 0;

			template<typename T>
			std::string generateFullPathForElement(T& inout, const std::string& ext = "bmp" )  const
			{
				return generateFullPathForElement(inout.getElementName(), ext);
			}

			virtual void createCSVFolder();

			virtual PARAMS_C2C_SHEET_OUTPUT processSheet(const PARAMS_C2C_SHEET_INPUT& sheetInput);
			virtual PARAMS_C2C_STRIP_OUTPUT processStrip(const PARAMS_C2C_STRIP_INPUT& stripInput);

			virtual void initEdge(const INIT_PARAMETER& initParam) const;
			virtual PARAMS_PAPEREDGE_OUTPUT processEdge(const PARAMS_PAPEREDGE_INPUT& input);
			virtual void shutdownEdge() const;

			virtual void initI2S(const INIT_PARAMETER& initParam) const;
			virtual PARAMS_I2S_OUTPUT processI2S(const PARAMS_I2S_INPUT& input);
			virtual void shutdownI2S() const;
			
			virtual void initC2CRoi(const INIT_PARAMETER& initParam) const;
			virtual PARAMS_C2C_ROI_OUTPUT processC2CROI(const PARAMS_C2C_ROI_INPUT& input);
			virtual void shutdownC2CRoi() const;
			
			virtual void initWave(const INIT_PARAMETER& initParam);
			virtual PARAMS_WAVE_OUTPUT processWave(const PARAMS_WAVE_INPUT& input);
			virtual concurrent_vector<PARAMS_WAVE_OUTPUT> processWaves(const std::vector<PARAMS_WAVE_INPUT>& inputs);
			virtual void shutdownWave();
			
			virtual void constructFrameContainer(const Core::FrameRef* frame, int bitsPerPixel);

			const Core::FrameRef* _frame = nullptr;;
			std::unique_ptr<cv::Mat> _frameContainer;
			int _frameIndex = 0;
			int _imageIndex = 0;
			std::string _csvFolder;
			std::string _sourceFrameIndexStr;
			std::shared_ptr<Parameters::ProcessParameters> _processParameters;
			bool _bParallelCalc = false;
		};
	}
}
