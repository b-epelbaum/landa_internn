#pragma once
#include "ProcessParameters.h"
#include "saveFunc.h"
#include <opencv/cv.h> 
#include <utility>
#include "common/june_exceptions.h"

namespace LandaJune
{
	namespace Algorithms
	{
		class ImageRegion
		{
			public:

			static ImageRegion createRegion
			(
				  const cv::Mat*								srcMat						// source data
				, std::shared_ptr<cv::Mat>						targetMat					// target MAT objects
				, ProcessParametersPtr							params
				, const cv::Rect&								srcRect						// rectangle of the source data to copy
				, int											frameIndex
				, const std::string&							saveFilePath				// ROI Name
				, const bool									needSaving = false			// if should be dumped to disk as well
			)
			{
				return  ImageRegion(srcMat,	targetMat, params, srcRect,	frameIndex, saveFilePath, needSaving );
			}

			ImageRegion(
				  const cv::Mat*									srcMat						// source data
				, std::shared_ptr<cv::Mat>							targetMat					// target MAT objects
				, ProcessParametersPtr								params
				, const cv::Rect&									srcRect						// rectangle of the source data to copy
				, int												frameIndex
				, const std::string&								saveFilePath				// ROI Name
				, const bool										needSaving = false			// if should be dumped to disk as well
				, const bool										asyncWrite = true			// save asynchronously or not
			)
				: _srcMatContainer(srcMat)
				, _targetMatContainer(std::move(targetMat))
				, _srcRequestedRect(srcRect)
				, _srcNormalizedRect(srcRect)
				, _bNeedSaving(needSaving)
				, _fullSavePath(saveFilePath)
				, _params(params)
				, _bSaveAsync(asyncWrite)
			{
				_bParallelizeCopy = params->ParalellizeCalculations();
				_srcNormalizedRect = normalizeRegionRect();
				// if ROI image needs to be saved, 
				if (_bNeedSaving)
				{
					if (_fullSavePath.empty())
					{
						THROW_EX_ERR_STR(CORE_ERROR::ALGO_EMPTY_ROI_NAME_TO_SAVE, "ROI requested for saving, but no name provided");
					}
				}
			}

			private:
				const cv::Mat*									_srcMatContainer;
				std::shared_ptr<cv::Mat>						_targetMatContainer;
				cv::Rect										_srcRequestedRect;
				cv::Rect										_srcNormalizedRect;
				bool											_bNeedSaving;
				std::string										_fullSavePath;
				ProcessParametersPtr							_params;
				bool											_bParallelizeCopy = false;
				bool											_bSaveAsync = true;

			

			cv::Rect normalizeRegionRect() const
			{
				auto const& srcWidth = _srcMatContainer->cols;
				auto const& srcHeight = _srcMatContainer->rows;

				auto const& regReqLeft = _srcRequestedRect.x;
				auto const& regReqTop = _srcRequestedRect.y;
				auto const& regReqRight = _srcRequestedRect.x + _srcRequestedRect.width;
				auto const& regReqBottom = _srcRequestedRect.y + _srcRequestedRect.height;

				// TODO : implement exception handler
				if (_srcRequestedRect.empty() || _srcRequestedRect.width == 0 || _srcRequestedRect.height == 0)
				{
					THROW_EX_ERR_STR(CORE_ERROR::ALGO_ROI_INVALID_RECT, "ROI rectangle is invalid ( null rect ). Batch input parameters init problem ?");
				}
				// todo : think about exceeding frame dimensions
				if (regReqLeft < 0
					|| regReqTop < 0
					|| regReqRight >srcWidth
					|| regReqBottom > srcHeight
					)
				{
					std::ostringstream ss;
					ss << "ROI rectangle limits exceed frame dimensions; "<< std::endl 
								<<"\t\tSource rect => [0,0," 
								<< srcWidth << "," << srcWidth << "]" << std::endl
								<<"\t\tTarget rect => [" << regReqLeft << "," << regReqTop << ","
								<< regReqRight << "," << regReqBottom << "]" << std::endl;

					//THROW_EX_ERR_STR(CORE_ERROR::ALGO_ROI_RECT_EXCEEDS_FRAME_RECT, ss.str());
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

		public:

			static void performCopy(ImageRegion& rgn)
			{
				try
				{
					// create a new MAT object by making a deep copy from the source MAT
					*rgn._targetMatContainer = std::move((*rgn._srcMatContainer)(rgn._srcNormalizedRect));

					if ( !rgn._params->EnableAnyDataSaving() )
						return;

					if ( !rgn._params->EnableImageSaving() )
						return;

					// dump input regions if needed
					if ( rgn._bNeedSaving && !rgn._fullSavePath.empty())
					{
						Files::dumpMatFile(rgn._targetMatContainer, rgn._fullSavePath, rgn._bParallelizeCopy, rgn._bSaveAsync);
					}
				}
				catch(BaseException& bex)
				{
					throw;
				}
				catch ( std::exception& ex)
				{
					RETHROW (CORE_ERROR::ALGO_PERFORM_COPY_FUNC_FAILED);
				}
			}
		};

		using IMAGE_REGION_LIST = concurrent_vector<ImageRegion>;
	}
}
