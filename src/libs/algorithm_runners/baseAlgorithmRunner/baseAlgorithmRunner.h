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
			virtual ~baseAlgorithmRunner();

			const baseAlgorithmRunner & operator = (const baseAlgorithmRunner &) = delete;
			baseAlgorithmRunner & operator = (baseAlgorithmRunner &&) = delete;

			void process(const Core::FrameRef * frame) override;

			std::shared_ptr<Parameters::BaseParameters> getParameters() const override { return _processParameters; }

		protected:
			virtual void validateProcessParameters(std::shared_ptr<Parameters::BaseParameters> parameters) = 0;

			virtual void fillCommonProcessParameters(std::shared_ptr<ABSTRACT_INPUT> input);
			virtual void fillSheetProcessParameters(std::shared_ptr<PARAMS_C2C_SHEET_INPUT> input);
			virtual void fillStripProcessParameters(std::shared_ptr<PARAMS_C2C_STRIP_INPUT> input, SHEET_SIDE side);
			virtual void fillEdgeProcessParameters(std::shared_ptr<PARAMS_PAPEREDGE_INPUT> input, SHEET_SIDE side);
			virtual void fillI2SProcessParameters(std::shared_ptr<PARAMS_I2S_INPUT> input, SHEET_SIDE side);
			virtual void fillC2CProcessParameters(std::shared_ptr<PARAMS_C2C_ROI_INPUT> input, SHEET_SIDE side);
			virtual void fillWaveProcessParameters(std::vector<std::shared_ptr<PARAMS_WAVE_INPUT>>& inputs);

			virtual void generateSheetRegions(std::shared_ptr<PARAMS_C2C_SHEET_INPUT> input, IMAGE_REGION_LIST& regionList) const;
			virtual void generateStripRegions(std::shared_ptr<PARAMS_C2C_STRIP_INPUT> input, IMAGE_REGION_LIST& regionList) const;
			virtual void generateI2SRegion(std::shared_ptr<PARAMS_I2S_INPUT> input, IMAGE_REGION_LIST& regionList) const;
			virtual void generateC2CRegion(std::shared_ptr<PARAMS_C2C_ROI_INPUT> input, IMAGE_REGION_LIST& regionList) const;
			virtual void generateWaveRegion(std::shared_ptr<PARAMS_WAVE_INPUT> input, IMAGE_REGION_LIST& regionList, bool bDumpWave ) const;

			virtual void processStripOutput(std::shared_ptr<PARAMS_C2C_STRIP_OUTPUT> stripOutput);

			virtual void copyRegions(IMAGE_REGION_LIST& regionList );

			template<typename T>
			void dumpOverlay(T& out)
			{
				if (_processParameters->EnableAnyDataSaving() && _processParameters->EnableImageSaving() && _processParameters->GenerateOverlays())
				{
					const auto targetMat = out->overlay();
					if ( targetMat != nullptr )
						dumpMatFile(targetMat, generateFullPathForElement<T>(out, "bmp", _processParameters, _frameIndex, _imageIndex, getFrameFolderName()), _bParallelCalc);
				}
			}

			virtual std::string getFrameFolderName() const;
			virtual void getSourceFrameIndexString();
			virtual std::string parseSourceFrameIndexString(const std::string& strPath) = 0;

			virtual std::shared_ptr<PARAMS_C2C_SHEET_OUTPUT> processSheet(std::shared_ptr<PARAMS_C2C_SHEET_INPUT> sheetInput);
			virtual std::shared_ptr<PARAMS_C2C_STRIP_OUTPUT> processStrip(std::shared_ptr<PARAMS_C2C_STRIP_INPUT> stripInput);

			virtual void initEdge(const INIT_PARAMETER& initParam) const;
			virtual std::shared_ptr<PARAMS_PAPEREDGE_OUTPUT> processEdge(std::shared_ptr<PARAMS_PAPEREDGE_INPUT> input);
			virtual void shutdownEdge() const;

			virtual void initI2S(const INIT_PARAMETER& initParam) const;
			virtual std::shared_ptr<PARAMS_I2S_OUTPUT> processI2S(std::shared_ptr<PARAMS_I2S_INPUT> input);
			virtual void shutdownI2S() const;
			
			virtual void initC2CRoi(const INIT_PARAMETER& initParam) const;
			virtual std::shared_ptr<PARAMS_C2C_ROI_OUTPUT> processC2CROI(std::shared_ptr<PARAMS_C2C_ROI_INPUT> input);
			virtual void shutdownC2CRoi() const;
			
			virtual void initWave(const INIT_PARAMETER& initParam);
			virtual std::shared_ptr<PARAMS_WAVE_OUTPUT> processWave(std::shared_ptr<PARAMS_WAVE_INPUT> input);
			virtual concurrent_vector<std::shared_ptr<PARAMS_WAVE_OUTPUT>> processWaves(const std::vector<std::shared_ptr<PARAMS_WAVE_INPUT>>& inputs);
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
