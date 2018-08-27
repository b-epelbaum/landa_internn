#pragma once
#include "common/june_exceptions.h"
#include <any>

namespace LandaJune {
	namespace Parameters {
		class ProcessParameter;
	}
}

namespace LandaJune 
{
	namespace Core
	{
		class FrameRef
		{
			friend class FrameRefPool;
		
		public :
			FrameRef(const FrameRef &) = delete;
			FrameRef(FrameRef &&) = delete;
			~FrameRef() = default;

			const FrameRef & operator = (const FrameRef &) = delete;
			FrameRef & operator = (FrameRef &&) = delete;
	
			explicit FrameRef( int32_t frameRefIndex );
			void reset();

			void setBits(const int32_t imageIdx
				, const int32_t width
				, const int32_t height
				, const size_t receivedSize
				, uint8_t* bits);
		
			uint32_t getIndex() const { return _index; }
			uint32_t getFrameRefIndex() const { return _frameRefIndex; }
			long long getTimeStamp() const { return _frameTimeStamp;  }
			const uint8_t* getBits() const;
			uint8_t* getBits();
			
			int32_t getWidth() const { return _frameWidth; }
			int32_t getHeight() const { return _frameHeight; }
			size_t getSize() const { return  _sizeInBytes;  }

		private:
			int32_t _index = -1;
			int32_t _frameRefIndex = 0;
		
			int32_t _frameWidth = -1;
			int32_t _frameHeight = -1;
			size_t _sizeInBytes = 0;
			long long _frameTimeStamp = -1;
			uint8_t * _bits = nullptr;
		};
	
	}
}
