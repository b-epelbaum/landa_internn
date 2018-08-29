#pragma once
#include "ProcessParameter.h"
#include "saveFunc.h"
#include <opencv/cv.h> 
#include "common/june_exceptions.h"

namespace LandaJune
{
	namespace Algorithms
	{
		class ImageRegion
		{
			public:

			ImageRegion(
				const cv::Mat&									srcMat						// source data
				, cv::Mat&										targetMat					// target MAT objects
				, std::shared_ptr<Parameters::ProcessParameter> params
				, const cv::Rect&								srcRect						// rectangle of the source data to copy
				, int											frameIndex
				, const std::string&							saveFilePath				// ROI Name
				, const bool									needSaving = false			// if should be dumped to disk as well
			)
				: _srcMatContainer(srcMat)
				, _targetMatContainer(targetMat)
				, _srcRequestedRect(srcRect)
				, _srcNormalizedRect(srcRect)
				, _bNeedSaving(needSaving)
				, _fullSavePath(std::move(saveFilePath))
				, _params(params)
			{
				_bParallelize = params->ParalellizeCalculations();
				_srcNormalizedRect = normalizeRegionRect();
				// if ROI image needs to be saved, 
				if (_bNeedSaving)
				{
					if (_fullSavePath.empty())
					{
						throw AlgorithmException(ALGORITHM_ERROR::ALGO_EMPTY_ROI_NAME_TO_SAVE, "ROI requested for saving, but no name provided");
					}
				}
			}

			private:
				const cv::Mat&									_srcMatContainer;
				cv::Mat&										_targetMatContainer;
				cv::Rect										_srcRequestedRect;
				cv::Rect										_srcNormalizedRect;
				bool											_bNeedSaving;
				std::string										_fullSavePath;
				std::shared_ptr<Parameters::ProcessParameter>	_params;
				bool											_bParallelize = false;

			

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

		public:

			static void performCopy(ImageRegion& rgn)
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

		using IMAGE_REGION_LIST = std::vector<ImageRegion>;
	}
}
