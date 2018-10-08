#pragma once
#include <vector>


namespace LandaJune
{
	namespace Algorithms
	{
		enum SHEET_SIDE { LEFT = 0, RIGHT, WAVE, NUM_SIDES};
		
		static std::string SIDE_NAMES[] =
		{
			  "Left"
			, "Right"
			, "Wave"
		};

		enum GRABBER_SIDE { FRONT = 0, BACK };

		struct APOINT
		{
			APOINT() = default;
			int32_t _x = 0;
			int32_t _y = 0;
		};


		struct ASIZE
		{
			ASIZE() = default;
			ASIZE(const ASIZE& other) = default;
			int32_t _width = 0;
			int32_t _height = 0;
		};

		struct ROIRect
		{
			ROIRect() = default;
			
			int32_t width() const { return _size._width; }
			int32_t height() const { return _size._height; }

			int32_t left() const { return _pt._x; }
			int32_t top() const { return _pt._y; }
			int32_t right() const { return _pt._x + _size._width; }
			int32_t bottom() const { return _pt._y + _size._height; }

			APOINT _pt;
			ASIZE _size;

		};

		// HSV values
		struct HSV_SINGLE
		{
			HSV_SINGLE() = default;
			HSV_SINGLE(const int32_t iH, const int32_t iS, const int32_t iV, std::string colorName)
				: _iH(iH)
				, _iS(iS)
				, _iV(iV)
				, _colorName(std::move(colorName))
			{}

			HSV_SINGLE(const HSV_SINGLE& other) = default;

			int32_t	_iH = 0;
			int32_t	_iS = 0;
			int32_t	_iV = 0;
			std::string _colorName;
		};

		struct HSV
		{
			HSV() = default;
			HSV(const HSV_SINGLE& hsvMin, const HSV_SINGLE& hsvMax )
				: _min(hsvMin)
				, _max(hsvMax)
				, _colorName(hsvMin._colorName)
			{}


			HSV(const HSV& other) = default;
			HSV_SINGLE _min;
			HSV_SINGLE _max;
			std::string _colorName;
		};
	}
}




