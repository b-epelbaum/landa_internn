#include "navigatorzone.h"

#include <applog.h>

#include <QMouseEvent>
#include <QWheelEvent>
#include <opencv2/opencv.hpp>

#include <cmath>

NavigatorZone::NavigatorZone(QWidget *parent)
	: QLabel(parent)
{
}

NavigatorZone::~NavigatorZone()
{
}

void NavigatorZone::updateImage(std::shared_ptr<cv::Mat>& img)
{
	if (!img) {
		_isValidImage = false;
		return;
	}

	_img = img;

	_sourceWidth = img->cols;
	_sourceHeight = img->rows;
	_imgRatio = static_cast<double>(_sourceWidth) / static_cast<double>(_sourceHeight);
	
	if (_lastFrameImageRatio != _imgRatio)
	{
		_frameDisplayImageSize = (_frameBoxRatio < _imgRatio)
			? QSize(_frameBoxWidth, _frameBoxWidth / _imgRatio)
			: QSize(_frameBoxHeight * _imgRatio, _frameBoxHeight);

		const auto frameBoxRect = (_frameBoxRatio < _imgRatio)
			? QRect(0, +(_frameBoxHeight - _frameDisplayImageSize.height()) / 2, _frameDisplayImageSize.width(), _frameDisplayImageSize.height())
			: QRect((_frameBoxWidth - _frameDisplayImageSize.width()) / 2, 0, _frameDisplayImageSize.width(), _frameDisplayImageSize.height());

		if (frameBoxRect != _frameBoxRect)
		{
			setGeometry(frameBoxRect);
			_frameBoxRect = frameBoxRect;
		}
	}

	_isValidImage = true;
	if (_frameDisplayImageSize.width() == 0 || _frameDisplayImageSize.height() == 0) {
		_isValidImage = false;
		return;
	}

	cv::Mat previewImg;
	cv::resize(*(_img), previewImg, { _frameDisplayImageSize.width(), _frameDisplayImageSize.height() });
	cv::cvtColor(previewImg, previewImg, CV_BGR2RGB);
	setPixmap(QPixmap::fromImage(QImage(static_cast<unsigned char*>(previewImg.data), previewImg.cols, previewImg.rows, previewImg.step, QImage::Format_RGB888)));
}

void NavigatorZone::resizeEvent(QResizeEvent* event)
{
	_frameBoxWidth = parentWidget()->geometry().width();
	_frameBoxHeight = parentWidget()->geometry().height();

	_frameBoxRatio = static_cast<double>(_frameBoxWidth) / static_cast<double>(_frameBoxHeight);
	QLabel::resizeEvent(event);
}

void NavigatorZone::mouseMoveEvent(QMouseEvent *event)
{
	if (!_isValidImage)
		return;

	const auto newPos = [&]() {
		const auto pos = event->pos();
		return QPoint(
			pos.x() < 0 ? 0 : pos.x(), 
			pos.y() < 0 ? 0 : pos.y()
		);
	}();

	_zonePoint = QPoint(
		float(newPos.x()) * (float(_sourceWidth) / float(_frameDisplayImageSize.width())),
		float(newPos.y()) * (float(_sourceHeight) / float(_frameDisplayImageSize.height()))
	);

	if (event->buttons() & Qt::LeftButton) {
		emit newFrameZonePosition(_zonePoint, _zoom);
	}
	
	QLabel::mouseMoveEvent(event);
}

void NavigatorZone::wheelEvent(QWheelEvent *event)
{
	if (event->delta() > 0) {
		if (std::fabs(_zoom - ZOOM_MAX) < 0.1)
			return;

		_zoom = _zoom + ZOOM_STEP;
	}
	else {
		if (std::fabs(_zoom - ZOOM_MIN) < 0.1)
			return;

		_zoom = _zoom - ZOOM_STEP;
	}

	//_zonePoint = _zonePoint * _zoom;

	//_zonePoint.setX(_zonePoint.x() < 0 ? 0 : _zonePoint.x());
	//_zonePoint.setY(_zonePoint.x() < 0 ? 0 : _zonePoint.y());

	emit newFrameZonePosition(_zonePoint, _zoom);
}