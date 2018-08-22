#pragma once

#include <vector>
#include "ProcessParameter.h"

namespace LandaJune
{
	namespace Parameters
	{
		enum SHEET_SIDE { LEFT = 0, RIGHT };
		enum GRABBER_SIDE { FRONT = 0, BACK };

		enum OUT_STATUS
		{
			ALG_STATUS_SUCCESS,
			ALG_STATUS_FAILED,
			ALG_STATUS_CIRCLE_NOT_FOUND,
			ALG_STATUS_CORRUPT_CIRCLE,
			ALG_STATUS_TOO_MANY_CIRCLES,
			ALG_STATUS_NOT_ENOUGH_CIRCLES,
			ALG_STATUS_NUM
		};


		struct APOINT
		{
			APOINT() = default;
			explicit APOINT (const QPoint& qpt ) : _x(qpt.x()), _y(qpt.y()) 
			{}

			APOINT& operator = (const QPoint& qpt)
			{
				_x = qpt.x();
				_y = qpt.y();
				return *this;
			}

			int32_t _x = 0;
			int32_t _y = 0;
		};


		struct ASIZE
		{
			ASIZE() = default;
			ASIZE(const ASIZE& other) = default;
			explicit ASIZE(const QSize& qsz) : _width(qsz.width()), _height(qsz.width())
			{}


			ASIZE& operator = (const QSize& qsz)
			{
				_width = qsz.width();
				_height = qsz.height();
				return *this;
			}

			int32_t _width = 0;
			int32_t _height = 0;
		};

		struct ROIRect
		{
			ROIRect() = default;
			explicit ROIRect(const QRect& qrc)
			{
				_pt = qrc.topLeft();
				_size = qrc.size();
			}

			explicit operator QRect() const { return QRect(_pt._x, _pt._y, _size._width, _size._height); }

			ROIRect& operator = (const QRect& qrc)
			{
				_pt = qrc.topLeft();
				_size = qrc.size();
				return *this;
			}

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
			HSV_SINGLE(const COLOR_TRIPLET_SINGLE& colTripletSingle) : HSV_SINGLE(colTripletSingle._iH, colTripletSingle._iS, colTripletSingle._iV ){}
			HSV_SINGLE(const int32_t iH, const int32_t iS, const int32_t iV)
				: _iH(iH)
				, _iS(iS)
				, _iV(iV)
			{}

			HSV_SINGLE(const HSV_SINGLE& other) = default;

			int32_t	_iH = 0;
			int32_t	_iS = 0;
			int32_t	_iV = 0;
		};

		struct HSV
		{
			HSV() = default;
			HSV(const COLOR_TRIPLET& colTriplet) : HSV(colTriplet._min, colTriplet._max) {}
			HSV(HSV_SINGLE hsvMin, HSV_SINGLE hsvMax )
				: _min(hsvMin)
				, _max(hsvMax)
			{}


			HSV(const HSV& other) = default;
			HSV_SINGLE _min;
			HSV_SINGLE _max;
		};
	}
}




