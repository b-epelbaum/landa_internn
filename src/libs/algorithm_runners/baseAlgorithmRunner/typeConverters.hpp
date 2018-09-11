#pragma once
//////////////////////////////////////////////////
////////////  HELPER FUNCTIONS
//////////////////////////////////////////////////

namespace LandaJune {
	namespace Core {
		class FrameRef;
	}
}


namespace LandaJune {
	namespace Parameters {
		class ProcessParameters;
	}
}


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

inline HSV_SINGLE colorSingle2HSVSingle(const LandaJune::Parameters::COLOR_TRIPLET_SINGLE& color)
{
	HSV_SINGLE out;
	out._iH = color.H();
	out._iS = color.S();
	out._iV = color.V();
	return std::move(out);
}

inline HSV color2HSV(const LandaJune::Parameters::COLOR_TRIPLET& color)
{
	HSV out;
	out._min = colorSingle2HSVSingle(color.Min());
	out._max = colorSingle2HSVSingle(color.Max());
	out._colorName = color.ColorName().toStdString();
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