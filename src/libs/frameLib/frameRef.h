#pragma once
#include <cstdint>
#include <memory>

#include "common/june_exceptions.h"

namespace LandaJune {
	namespace Parameters {
		class ProcessParameter;
	}
}

namespace cv 
{
	class Mat;
}


namespace LandaJune 
{
	namespace Core
	{
		class FrameRef
		{
			friend class FrameRefPool;
		
		public :
			struct GLOBAL_FRAME_DATA
			{
				explicit GLOBAL_FRAME_DATA(std::shared_ptr<Parameters::ProcessParameter> globalParams);

				bool operator == (const GLOBAL_FRAME_DATA& other) const;
				bool isValid(FRAME_REF_ERROR& err) const;

				int _cvImageFormat = 0;
				std::shared_ptr<Parameters::ProcessParameter> _params;
			};

			FrameRef(const FrameRef &) = delete;
			FrameRef(FrameRef &&) = delete;
			~FrameRef() = default;

			const FrameRef & operator = (const FrameRef &) = delete;
			FrameRef & operator = (FrameRef &&) = delete;
	
			explicit FrameRef(const GLOBAL_FRAME_DATA& frameData);
			void reset();

			void setBits(const int32_t imageIdx
				, const int32_t width
				, const int32_t height
				, const size_t receivedSize
				, uint8_t* bits);
		
			uint32_t getIndex() const { return _index; }
			long long getTimeStamp() const { return _frameTimeStamp;  }
			const uint8_t* getBits() const;
			uint8_t* getBits();
			std::shared_ptr<cv::Mat> getImageContainer() const { return _imgContainer;  }
				
			int32_t getWidth() const { return _frameWidth; }
			int32_t getHeight() const { return _frameHeight; }
			int32_t getBitsPerPixel() const { return _bitsPerPixel; }
			size_t getSize() const { return  _sizeInBytes;  }
			int getFormat() const { return _imgCVFormat; }
			std::shared_ptr<Parameters::ProcessParameter> getProcessParams() const { return _processParameters; }

		private:
			int32_t _index = -1;
			std::shared_ptr<cv::Mat> _imgContainer;
				
			int32_t _frameWidth = -1;
			int32_t _frameHeight = -1;
			int32_t _bitsPerPixel = 1;
			size_t _sizeInBytes = 0;
			int _imgCVFormat = 0;
			std::shared_ptr <Parameters::ProcessParameter> _processParameters;
			bool _bInited = false;
			long long _frameTimeStamp = -1;
		};
	
	}
}
