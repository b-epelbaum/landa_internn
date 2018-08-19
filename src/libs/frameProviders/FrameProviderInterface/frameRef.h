#pragma once
#include <cstdint>
#include <memory>

#include "ProcessParameter.h"
#include "jutils.h"
#include "common/june_errors.h"
#include "common/june_exceptions.h"

#include <opencv/cv.h> 
#include <opencv2/imgcodecs.hpp>

namespace LandaJune {
	namespace Parameters {
		class ProcessParameter;
	}
}

namespace LandaJune 
{
	namespace FrameProviders
	{
		class FrameRef
		{
			friend class FrameRefPool;
		
		public :
			struct GLOBAL_FRAME_DATA
			{
				explicit GLOBAL_FRAME_DATA (std::shared_ptr<Parameters::ProcessParameter> globalParams)
					: _cvImageFormat (globalParams->OpenCVImageFormat())
					, _params(globalParams)
				{
				}

				bool operator == (const GLOBAL_FRAME_DATA& other) const
				{
					return (
						_cvImageFormat == other._cvImageFormat
						&& _params == other._params);
				};

				bool isValid (FRAME_REF_ERROR& err) const
				{
					err = FRAME_REF_ERROR::ERR_NO_ERROR;
					const auto allOk = (
					  _params
					);
					if (allOk)
						return true;
						err = (!_params) ? FRAME_REF_ERROR::ERR_FRAME_INVALID_BATCH_PARAMS
						: FRAME_REF_ERROR::ERR_FRAME_INVALID_INIT_DATA;

					return false;
				}

				int _cvImageFormat = CV_8UC3;
				std::shared_ptr<Parameters::ProcessParameter> _params;
			};

				FrameRef(const FrameRef &) = delete;
				FrameRef(FrameRef &&) = delete;
				~FrameRef() = default;

				const FrameRef & operator = (const FrameRef &) = delete;
				FrameRef & operator = (FrameRef &&) = delete;
	
				explicit FrameRef(const GLOBAL_FRAME_DATA& frameData)
				{
					FRAME_REF_ERROR err;
					if (!frameData.isValid(err))
					{
						throw FrameRefException(err, "Cannot initialize FrameRef object");
					}

					_imgCVFormat = frameData._cvImageFormat;
					_bitsPerPixel = frameData._params->BitDepth();

					// TODO : think about padding
					_commonBatchParameters = std::move(frameData._params);
					_bInited = true;
				}

				void reset()
				{
					_index = -1;
					_imgContainer.release();
				}

				void setBits(const int32_t imageIdx
					, const int32_t width
					, const int32_t height
					, const size_t receivedSize
					, uint8_t* bits);
		
				uint32_t getIndex() const { return _index; }
				long long getTimeStamp() const { return _frameTimeStamp;  }
				const uint8_t* getBits() const { return static_cast<const uint8_t*>(_imgContainer.data); }
				uint8_t* getBits() { return static_cast<uint8_t*>(_imgContainer.data); }
				const cv::Mat& getImageContainer() const { return _imgContainer;  }
				
				int32_t getWidth() const { return _frameWidth; }
				int32_t getHeight() const { return _frameHeight; }
				int32_t getBitsPerPixel() const { return _bitsPerPixel; }
				size_t getSize() const { return  _sizeInBytes;  }
				int getFormat() const { return _imgCVFormat; }
				std::shared_ptr<Parameters::ProcessParameter> getBatchParams() const { return _commonBatchParameters; }

			private:
				int32_t _index = -1;
				cv::Mat _imgContainer;
				
				int32_t _frameWidth = -1;
				int32_t _frameHeight = -1;
				int32_t _bitsPerPixel = 1;
				size_t _sizeInBytes = 0;
				int _imgCVFormat = CV_8UC3;
				std::shared_ptr <Parameters::ProcessParameter> _commonBatchParameters = nullptr;
				bool _bInited = false;
				long long _frameTimeStamp = -1;
		};
		
		inline void FrameRef::setBits(const int32_t imageIdx
				, const int32_t width
				, const int32_t height
				, const size_t receivedSize
				, uint8_t* bits)
		{
			const auto& t = std::chrono::system_clock::now().time_since_epoch();
			_frameTimeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(t).count();
			_index = imageIdx;

			_frameWidth = width;
			_frameHeight = height;
			_sizeInBytes = receivedSize;
			_imgContainer = { _frameHeight, _frameWidth, _imgCVFormat, bits };
		}
	}
}
