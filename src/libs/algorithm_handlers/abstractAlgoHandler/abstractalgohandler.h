#pragma once

#include "../interfaces/IAlgorithmHandler.h"

#include "algorithm_parameters.h"
#include "ProcessParameter.h"
#include "frameRef.h"
#include "TaskThreadPool.h"
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
		class abstractAlgoHandler : public QObject, public IAlgorithmHandler
		{
			Q_OBJECT
			
			friend class IAlgorithmHandler;

		public:
			abstractAlgoHandler() = default;
			abstractAlgoHandler(const abstractAlgoHandler &) = delete;
			abstractAlgoHandler(abstractAlgoHandler &&) = delete;
			virtual ~abstractAlgoHandler() = default;

			const abstractAlgoHandler & operator = (const abstractAlgoHandler &) = delete;
			abstractAlgoHandler & operator = (abstractAlgoHandler &&) = delete;

			void init(std::shared_ptr<Parameters::BaseParameter> parameters) override;
			void process(const Core::FrameRef * frame) override;

			std::shared_ptr<Parameters::BaseParameter> getParameters() const override { return _processParameters; }

		protected:

			virtual void validateProcessParameters(std::shared_ptr<Parameters::BaseParameter> parameters) = 0;

			virtual void fillCommonProcessParameters(ABSTRACT_INPUT& input);
			virtual void fillSheetProcessParameters(PARAMS_C2C_SHEET_INPUT& input);
			virtual void fillStripProcessParameters(PARAMS_C2C_STRIP_INPUT& input, SHEET_SIDE side);
			virtual void fillEdgeProcessParameters(PARAMS_PAPEREDGE_INPUT& input, SHEET_SIDE side);
			virtual void fillI2SProcessParameters(PARAMS_I2S_INPUT& input, SHEET_SIDE side);
			virtual void fillC2CProcessParameters(PARAMS_C2C_ROI_INPUT& input, SHEET_SIDE side);
			virtual void fillWaveProcessParameters(PARAMS_WAVE_INPUT& input);

			virtual void generateSheetRegions(PARAMS_C2C_SHEET_INPUT& input, IMAGE_REGION_LIST& regionList) const;
			virtual void generateStripRegions(PARAMS_C2C_STRIP_INPUT& input, IMAGE_REGION_LIST& regionList) const;
			virtual void generateI2SRegions(PARAMS_I2S_INPUT& input, IMAGE_REGION_LIST& regionList) const;
			virtual void generateC2CRegions(PARAMS_C2C_ROI_INPUT& input, IMAGE_REGION_LIST& regionList) const;
			virtual void generateWaveRegions(PARAMS_WAVE_INPUT& input, IMAGE_REGION_LIST& regionList);

			virtual void copyRegions(IMAGE_REGION_LIST& regionList );

			template<typename T>
			void dumpOverlay(T& out)
			{
				if (!_processParameters->DisableAllROISaving() && _processParameters->GenerateOverlays())
				{
					const auto& targetMat = out.overlay();
					if ( targetMat != std::nullopt )
						dumpMatFile(targetMat.value(), generateFullPathForElement<T>(out), false, _bParallelizeCalculations);
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

			template<typename T>
			std::string generateFullPathForElement(T& inout, const std::string& ext = "bmp" )  const
			{
				return generateFullPathForElement(inout.getElementName(), ext);
			}

			virtual void createCSVFolder();

			virtual PARAMS_C2C_SHEET_OUTPUT processSheet(const PARAMS_C2C_SHEET_INPUT& sheetInput);
			virtual PARAMS_C2C_STRIP_OUTPUT processStrip(const PARAMS_C2C_STRIP_INPUT& stripInput, bool detectEdge);

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
			virtual void shutdownWave();
			
			virtual void constructFrameContainer(const Core::FrameRef* frame, int bitsPerPixel);

			const Core::FrameRef* _frame = nullptr;;
			std::unique_ptr<cv::Mat> _frameContainer;
			int _frameIndex = 0;
			int _imageIndex = 0;
			std::string _csvFolder;
			std::shared_ptr<Parameters::ProcessParameter> _processParameters;
			bool _bParallelizeCalculations = false;
		};
	}
}
