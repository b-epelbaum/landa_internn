#pragma once

#include "abstractalgohandler_global.h"
#include "../interfaces/IAlgorithmHandler.h"

#include "algorithm_parameters.h"
#include "ProcessParameter.h"
#include "frameRef.h"
#include "functions.h"
#include "include/format.h"
#include "TaskThreadPool.h"


///////////////
//  OPEN CV
#include <opencv/cv.h> 
#include <opencv2/imgcodecs.hpp>
#include "applog.h"
#include <opencv2/imgcodecs/imgcodecs_c.h>
#include "util.h"

#ifdef _DEBUG
#pragma comment(lib, "opencv_world342d.lib")
#else
#pragma comment(lib, "opencv_world342.lib")
#endif

static const std::string DEFAULT_OUT_FOLDER = "c:\\temp\\june_out";

namespace LandaJune {
	namespace Core {
		class FrameRef;
	}
}

namespace LandaJune {
	namespace Core {
		class FrameRef;
	}
}

namespace LandaJune {
	namespace Parameters {
		class ProcessParameter;
	}
}

namespace LandaJune
{
	namespace Algorithms
	{

		//////////////////////////////////////////////////
		////////////  HELPER FUNCTIONS
		//////////////////////////////////////////////////


		inline APOINT toAPoint(const QPoint& qpt)
		{
			APOINT out;
			out._x = qpt.x();
			out._y = qpt.y();
			return std::move(out);

		}

		inline ASIZE toASize(const QSize& qsz)
		{
			ASIZE out;
			out._width = qsz.width();
			out._height = qsz.height();
			return std::move(out);
		}

		inline ROIRect toROIRect(const QRect& qrc)
		{
			ROIRect out;
			out._pt = toAPoint(qrc.topLeft());
			out._size = toASize(qrc.size());
			return std::move(out);
		}

		inline HSV_SINGLE colorSingle2HSVSingle(const Parameters::COLOR_TRIPLET_SINGLE& color)
		{
			HSV_SINGLE out;
			out._iH = color._iH;
			out._iS = color._iS;
			out._iV = color._iV;
			return std::move(out);
		}

		inline HSV color2HSV(const Parameters::COLOR_TRIPLET& color)
		{
			HSV out;
			out._min = colorSingle2HSVSingle(color._min);
			out._max = colorSingle2HSVSingle(color._max);
			out._colorName = color._colorName;
			return std::move(out);
		}


		inline cv::Rect qrect2cvrect(const QRect& rcSrc)
		{
			return cv::Rect(rcSrc.left(), rcSrc.top(), rcSrc.width(), rcSrc.height());
		}

		inline cv::Rect roirect2cvrect(const ROIRect& rcSrc)
		{
			return cv::Rect(rcSrc.left(), rcSrc.top(), rcSrc.width(), rcSrc.height());
		}

		inline void dumpMatFile (const cv::Mat& img, const std::string& filePath, bool bCloneImage, bool bParallelWrite)
		{
			const auto t0 = Helpers::Utility::now_in_microseconds();
			if (bParallelWrite)
			{
				const auto t0 = Helpers::Utility::now_in_microseconds();
				if (bCloneImage)
					Threading::TaskThreadPools::postJob(Threading::TaskThreadPools::diskDumperThreadPool(), Functions::frameSaveImage, std::move(img.clone()), filePath);
				else
					Threading::TaskThreadPools::postJob(Threading::TaskThreadPools::diskDumperThreadPool(), Functions::frameSaveImage, std::move(img), filePath);

				const auto t1 = Helpers::Utility::now_in_microseconds();
				//const double& perfTime = (static_cast<double>(t1) - static_cast<double>(t0)) / 1000;
				//PRINT_INFO7 << "dumpMatFile !!!!!!!!!!!! " << perfTime << " msec...";
			}
			else
			{
				Functions::frameSaveImage(img, filePath);
			}
		}


		///////////////////////////////////////////////////
		/////////////  REGIONS
		//////////////////////////////////////////////////

		struct CV_COPY_REGION
		{
			const cv::Mat&				_srcMatContainer;
			cv::Mat&					_targetMatContainer;
			cv::Rect					_srcRequestedRect;
			cv::Rect					_srcNormalizedRect;
			bool						_bNeedSaving;
			std::string					_fullSavePath;
			std::shared_ptr<Parameters::ProcessParameter> _params;
			bool						_bParallelize = false;

			CV_COPY_REGION(
				const cv::Mat&	srcMat							// source data
				, cv::Mat&			targetMat						// target MAT objects
				, std::shared_ptr<Parameters::ProcessParameter> params
				, const cv::Rect&	srcRect							// rectangle of the source data to copy
				, int				frameIndex
				, const std::string& ROIName						// ROI Name
				, const bool		needSaving = false				// if should be dumped to disk as well
			)
				: _srcMatContainer(srcMat)
				, _targetMatContainer(targetMat)
				, _srcRequestedRect(srcRect)
				, _srcNormalizedRect(srcRect)
				, _bNeedSaving(needSaving)
				, _params(params)
			{
				_bParallelize = params->ParalellizeCalculations();
				_srcNormalizedRect = normalizeRegionRect();
				// if ROI image needs to be saved, 
				if (_bNeedSaving)
				{
					if (ROIName.empty())
					{
						throw AlgorithmException(ALGORITHM_ERROR::ALGO_EMPTY_ROI_NAME_TO_SAVE, "ROI requested for saving, but no name provided");
					}

					//generate saving path
					_fullSavePath = getSavePath(params, ROIName, frameIndex);
				}
			}

			static std::string getSavePath(std::shared_ptr<Parameters::ProcessParameter> params, const std::string& itemName, int frameIndex)
			{
				auto rootPath = params->RootOutputFolder().toStdString();
				if (rootPath.empty())
				{
					rootPath = DEFAULT_OUT_FOLDER;
				}
				return fmt::format("{0}\\{1}\\frame_#{2}\\{3}.bmp"
					, rootPath
					, params->JobID()
					, frameIndex
					, itemName
				);
			}


			cv::Rect normalizeRegionRect() const
			{
				auto const& srcWidth = _srcMatContainer.cols;
				auto const& srcHeight = _srcMatContainer.rows;

				auto const& regReqLeft = _srcRequestedRect.x;
				auto const& regReqTop = _srcRequestedRect.y;
				auto const& regReqRight = _srcRequestedRect.x + _srcRequestedRect.width;
				auto const& regReqBottom = _srcRequestedRect.y + _srcRequestedRect.height;

				// TODO : implement exception handler
				if (_srcRequestedRect.empty() || _srcRequestedRect.width == 0 || _srcRequestedRect.height == 0)
				{
					throw AlgorithmException(ALGORITHM_ERROR::ALGO_ROI_INVALID_RECT, "ROI rectangle is invalid. Batch input parameters init problem ?");
				}
				// todo : think about exceeding frame dimensions
				if (regReqLeft < 0
					|| regReqTop < 0
					|| regReqRight >srcWidth
					|| regReqBottom > srcHeight
					)
				{
					//throw AlgorithmException(ALGORITHM_ERROR::ALGO_ROI_RECT_EXCEEDS_FRAME_RECT, "ROI rectangle limits exceed frame dimensions");
				}

				cv::Rect retValRect;
				// fix actual target rectangle to stay within frame limits
				retValRect.x = ((std::max)(regReqLeft, 0));
				retValRect.y = ((std::max)(regReqTop, 0));

				const auto& normRight = ((std::min)(regReqRight, srcWidth));
				const auto& normBottom = ((std::min)(regReqBottom, srcHeight));

				retValRect.width = normRight - _srcNormalizedRect.x;
				retValRect.height = normBottom - retValRect.y;

				return retValRect;
			}

			static void performCopy(CV_COPY_REGION& rgn)
			{
				// create a new MAT object by making a deep copy from the source MAT
				rgn._targetMatContainer = std::move((rgn._srcMatContainer)(rgn._srcNormalizedRect));

				// dump input regions if needed
				if (!rgn._params->DisableAllROISaving() && rgn._bNeedSaving && !rgn._fullSavePath.empty())
				{
					dumpMatFile(rgn._targetMatContainer, rgn._fullSavePath, true, rgn._bParallelize);
				}
			}
		};

		using CV_COPY_REGION_LIST = std::vector<CV_COPY_REGION>;


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

			virtual void generateSheetRegions(PARAMS_C2C_SHEET_INPUT& input, CV_COPY_REGION_LIST& regionList) const;
			virtual void generateStripRegions(PARAMS_C2C_STRIP_INPUT& input, CV_COPY_REGION_LIST& regionList) const;
			virtual void generateI2SRegions(PARAMS_I2S_INPUT& input, CV_COPY_REGION_LIST& regionList) const;
			virtual void generateC2CRegions(PARAMS_C2C_ROI_INPUT& input, CV_COPY_REGION_LIST& regionList) const;
			virtual void generateWaveRegions(PARAMS_WAVE_INPUT& input, CV_COPY_REGION_LIST& regionList);

			virtual void copyRegions(CV_COPY_REGION_LIST& regionList );
			virtual void dumpOverlays(const cv::Mat& img, const std::string& fileName);
			virtual void dumpFrameCSV(const PARAMS_C2C_STRIP_OUTPUT& stripOut);

			virtual std::string getBatchRootFolder();
			virtual std::string getFrameFolderName();

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
