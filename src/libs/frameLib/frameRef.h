#pragma once
#include "common/june_exceptions.h"
#include <any>
#include <functional>
#include <map>


#define NAMED_PROPERTY_SOURCE_PATH "srcPath"
#define NAMED_PROPERTY_PROVIDER_NAME "providerName"

namespace LandaJune {
	namespace Parameters {
		class ProcessParameters;
	}
}

namespace cv {
		class Mat;
}


namespace LandaJune   
{
	namespace Core
	{
		struct SharedFrameData;

		class FrameRef
		{
			friend class FrameRefPool;
			friend struct SharedFrameData;

		public :
			FrameRef(const FrameRef &) = delete;
			FrameRef(FrameRef &&) = delete;
			~FrameRef() = default;

			const FrameRef & operator = (const FrameRef &) = delete;
			FrameRef & operator = (FrameRef &&) = delete;
	
			explicit FrameRef(uint64_t frameRefIndex );
			void reset();

			void setPostDataFunction(const std::function<void(FrameRef*)>& func)
			{
				_postDataFunc = func;
			}

			void setNamedParameter(const std::string& paramName, std::any paramValue) { _paramsMap[paramName] = std::move(paramValue); }

			std::any getNamedParameter (const std::string& paramName) const
			{
				if (const auto& it = _paramsMap.find(paramName); it == _paramsMap.cend() )
				{
					return std::any{};
				}
				else
					return it->second;
			}

			void setBits(const int32_t imageIdx
				, const int32_t width
				, const int32_t height
				, const int32_t bitsPerPixel
				, const size_t receivedSize
				, uint8_t* bits);

			void setBits(const int32_t imageIdx, std::shared_ptr<cv::Mat> mat);
		
			uint32_t getIndex() const { return _index; }
			uint32_t getFrameRefIndex() const { return _frameRefIndex; }
			long long getTimeStamp() const { return _frameTimeStamp;  }
			
			std::shared_ptr<cv::Mat> image() const { return _img; }
			int32_t getWidth() const { return _frameWidth; }
			int32_t getHeight() const { return _frameHeight; }
			int32_t getBitsPerPixel() const { return _bitsPerPixel; }
			size_t getSize() const { return  _sizeInBytes;  }
			bool getAsyncWrite() const { return _asyncWrite; }
			void setAsyncWrite( bool val) { _asyncWrite = val;  }

		private:
			int32_t _index = -1;
			uint64_t _frameRefIndex = 0;
		
			int32_t _frameWidth = -1;
			int32_t _frameHeight = -1;
			int32_t _bitsPerPixel = 24;
			size_t _sizeInBytes = 0;
			long long _frameTimeStamp = -1;
			bool _asyncWrite = true;
		
			std::shared_ptr<cv::Mat> _img;

			std::map<std::string, std::any> _paramsMap;
			std::function<void(FrameRef*)> _postDataFunc;
		};

		struct SharedFrameData
		{
			explicit SharedFrameData (const FrameRef * frame, int32_t maxLifeSpan)
				: _img(frame->image())
				, _index(frame->_index)
				, _frameRefIndex(frame->_frameRefIndex)
				, _frameTimeStamp(frame->_frameTimeStamp)
				, _lifeSpanMs(maxLifeSpan)
			{
			}

			std::shared_ptr<cv::Mat> _img;
			int32_t		_index = -1;
			uint64_t	_frameRefIndex = 0;
			long long	_frameTimeStamp = -1;
			int32_t		_lifeSpanMs = 0;
		};
	}
}
