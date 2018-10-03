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
			baseAlgorithmRunner(const baseAlgorithmRunner & other);

			baseAlgorithmRunner(baseAlgorithmRunner &&) = delete;
			virtual ~baseAlgorithmRunner() = default;

			const baseAlgorithmRunner & operator = (const baseAlgorithmRunner &) = delete;
			baseAlgorithmRunner & operator = (baseAlgorithmRunner &&) = delete;

			void process(const Core::FrameRef * frame) override final;

			BaseParametersPtr getParameters() const override { return _processParameters; }

		protected:
			//////////////////////////////////////////////////////
			//////////////  SETUP FUNCTIONS 

			virtual void validateProcessParameters(BaseParametersPtr parameters) = 0;
			virtual void setupFrameData(const Core::FrameRef* frame);
			virtual void processInternal () = 0;

			virtual void setupCommonProcessParameters	(ABSTRACT_INPUT_PTR input);
			virtual void setupSheetProcessParameters	(PARAMS_C2C_SHEET_INPUT_PTR input);
			virtual void setupStripProcessParameters	(PARAMS_C2C_STRIP_INPUT_PTR input, SHEET_SIDE side);
			virtual void setupEdgeProcessParameters		(PARAMS_PAPEREDGE_INPUT_PTR input, SHEET_SIDE side);
			virtual void setupI2SProcessParameters		(PARAMS_I2S_INPUT_PTR input, SHEET_SIDE side);
			virtual void setupC2CProcessParameters		(PARAMS_C2C_ROI_INPUT_PTR input, SHEET_SIDE side);
			virtual void setupWaveProcessParameters		(std::vector<PARAMS_WAVE_INPUT_PTR>& inputs);

			virtual void generateSheetRegions			(PARAMS_C2C_SHEET_INPUT_PTR input, IMAGE_REGION_LIST& regionList) const;
			virtual void generateStripRegions			(PARAMS_C2C_STRIP_INPUT_PTR input, IMAGE_REGION_LIST& regionList) const;
			virtual void generateI2SRegion				(PARAMS_I2S_INPUT_PTR input, IMAGE_REGION_LIST& regionList) const;
			virtual void generateC2CRegion				(PARAMS_C2C_ROI_INPUT_PTR input, IMAGE_REGION_LIST& regionList) const;
			virtual void generateWaveRegion				(PARAMS_WAVE_INPUT_PTR input, IMAGE_REGION_LIST& regionList, bool bDumpWave ) const;
						
			virtual void copyRegions(IMAGE_REGION_LIST& regionList );

			//////////////////////////////////////////////////////
			//////////////  MAIN PROCESSING FUNCTIONS 

			virtual PARAMS_C2C_SHEET_OUTPUT_PTR processSheet(PARAMS_C2C_SHEET_INPUT_PTR sheetInput);
			virtual PARAMS_C2C_STRIP_OUTPUT_PTR processStrip(PARAMS_C2C_STRIP_INPUT_PTR stripInput);

			virtual void initEdge(const INIT_PARAMETER& initParam) const;
			virtual PARAMS_PAPEREDGE_OUTPUT_PTR processEdge(PARAMS_PAPEREDGE_INPUT_PTR input);
			virtual void shutdownEdge() const;

			virtual void initI2S(const INIT_PARAMETER& initParam) const;
			virtual PARAMS_I2S_OUTPUT_PTR processI2S(PARAMS_I2S_INPUT_PTR input);
			virtual void shutdownI2S() const;
			
			virtual void initC2CRoi(const INIT_PARAMETER& initParam) const;
			virtual PARAMS_C2C_ROI_OUTPUT_PTR processC2CROI(PARAMS_C2C_ROI_INPUT_PTR input);
			virtual void shutdownC2CRoi() const;
			
			virtual void initWave(const INIT_PARAMETER& initParam);
			virtual PARAMS_WAVE_OUTPUT_PTR processWave(PARAMS_WAVE_INPUT_PTR input);
			virtual concurrent_vector<PARAMS_WAVE_OUTPUT_PTR> processWaves(const std::vector<PARAMS_WAVE_INPUT_PTR>& inputs);
			virtual void shutdownWave();

			/////////////////////////////////////////////////////////
			/////////////   OUTPUT PROCESSING FUNCTIONS
			
			// general sheet output processing
			virtual void processSheetOutput(PARAMS_C2C_SHEET_OUTPUT_PTR sheetOutput)						= 0;

			// general strip output processing
			virtual void processStripOutput(PARAMS_C2C_STRIP_OUTPUT_PTR stripOutput)						= 0;
			
			// general wave output processing
			virtual void processWaveOutputs(concurrent_vector<PARAMS_WAVE_OUTPUT_PTR> & waveOutputs )		= 0;
			

			//////////////////////////////////////////////////////
			//////////////  UTILITY FUNCTIONS 

			template<typename T>
			void dumpOverlay(T& out, bool asyncWrite)
			{
				if (_processParameters->EnableAnyDataSaving() && _processParameters->EnableImageSaving() && _processParameters->GenerateOverlays())
				{
					const auto targetMat = out->overlay();
					if ( targetMat != nullptr )
						dumpMatFile(targetMat, generateFullPathForElement<T>(out, "bmp", _processParameters, _frameIndex, _imageIndex, getFrameFolderName()), _bParallelCalc, asyncWrite);
				}
			}

			virtual std::string getFrameFolderName() const;
			virtual void getSourceFrameIndexString();
			virtual std::string parseSourcePathForFrameIndex(const std::string& strPath) { return {}; }

			////////////////////////////////////////
			///////////// MEMBERS

			const Core::FrameRef*		_frame				= nullptr;
			Core::ICore*				_coreObject			= nullptr;
			FrameConsumerCallback		_callback			= nullptr;

			ProcessParametersPtr		_processParameters;
			int							_frameIndex			= 0;
			int							_imageIndex			= 0;
			bool						_bParallelCalc		= false;
			bool						_bAsyncWrite		= true;
			bool						_bOfflineSource		= false;
			
			std::string					_sourceFrameIndexStr;
			std::string					_sourceFramePath;
			std::string					_csvOutFolder;
			std::string					_loadedConfigName;
		};
	}
}
