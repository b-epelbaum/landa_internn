#include "framezone.h"

#include <QMouseEvent>
#include <QWheelEvent>

#include <opencv2/opencv.hpp>

FrameZone::FrameZone(QWidget *parent)
	: QLabel(parent)
{
	setMouseTracking(true);
}

FrameZone::~FrameZone()
{
}

void FrameZone::updateFrameZone(std::shared_ptr<cv::Mat>& img)
{
	if (!img) {
		return;
	}

	_img = img;

	calculateSize();
	calculatePosition(_frameZonePosition);
	prepareNewFrame();
}

void FrameZone::onNewFrameZonePosition(const QPoint &point, float zoom)
{
	_zoom = zoom;
	calculateSize();
	calculatePosition(point);
	prepareNewFrame();
}

void FrameZone::calculateSize()
{
	const auto windowsSize = size();
	// Frame zone size
	QSize frameZone = windowsSize / _zoom;
	const int frameWidth = frameZone.width() > _img->cols ? _img->cols : frameZone.width();
	const int frameHeight = frameZone.height() > _img->rows ? _img->rows : frameZone.height();
	
	_frameSize = { frameWidth, frameHeight };

	// Window zone size
	const int windowsWidth = windowsSize.width() > _img->cols ? _img->cols : windowsSize.width();
	const int windowsHeight = windowsSize.height() > _img->rows ? _img->rows : windowsSize.height();
	
	_windowsSize = { windowsWidth , windowsHeight };
}

void FrameZone::calculatePosition(const QPoint& newPoint)
{
	int x = newPoint.x() - _frameSize.width() / 2;
	int y = newPoint.y() - _frameSize.height() / 2;

	x = newPoint.x() + _frameSize.width() >= _img->cols
		? _frameZonePosition.x()
		: newPoint.x();

	y = newPoint.y() + _frameSize.height() >= _img->rows
		? _frameZonePosition.y()
		: newPoint.y();

	//for case, when new image is smaller size then prev;
	x = x + _frameSize.width() >= _img->cols ? 0 : x;
	y = y + _frameSize.height() >= _img->rows ? 0 : y;

	_frameZonePosition = { (x < 0 ? 0 : x), (y < 0 ? 0 : y) };
}

void FrameZone::prepareNewFrame()
{
	cv::Mat croppedImg;
	(*_img)(cv::Rect(_frameZonePosition.x(), _frameZonePosition.y(), _frameSize.width(), _frameSize.height())).copyTo(croppedImg);
	if (fabs(fabs(_zoom) - 1.0f) > 0.1f) {
		cv::resize(croppedImg, croppedImg, { _windowsSize.width(), _windowsSize.height() });
	}
	cv::cvtColor(croppedImg, croppedImg, CV_BGR2RGB);
	setPixmap(QPixmap::fromImage(QImage((unsigned char*)croppedImg.data, croppedImg.cols, croppedImg.rows, croppedImg.step, QImage::Format_RGB888)));
}

void FrameZone::mouseMoveEvent(QMouseEvent *event)
{
	return QLabel::mouseMoveEvent(event);
	if (_lastMousePos.isNull()) {
		_lastMousePos = event->pos();
		return;
	}

	const auto newPos = event->pos();

	int dx = newPos.x() - _lastMousePos.x();
	int dy = newPos.y() - _lastMousePos.y();

	if (event->buttons() & Qt::LeftButton) {
		calculatePosition({ _frameZonePosition.x() - dx, _frameZonePosition.y() - dy });
		prepareNewFrame();
	}

	_lastMousePos = newPos;
	QLabel::mouseMoveEvent(event);

	emitPositionChanged();
}

void FrameZone::wheelEvent(QWheelEvent *event)
{
//	PRINT_WARNING << event->delta();
}

void FrameZone::emitPositionChanged()
{
	emit positionChanged(_frameZonePosition, _frameSize);
}