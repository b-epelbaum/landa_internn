#pragma once

#include <QLabel>

#include <memory>

namespace cv {
	class Mat;
}

constexpr float ZOOM_MIN = 0.5f;
constexpr float ZOOM_MAX = 2.0f;
constexpr float ZOOM_STEP = 0.1f;

class NavigatorZone : public QLabel
{
	Q_OBJECT

public:
	NavigatorZone(QWidget *parent = Q_NULLPTR);
	~NavigatorZone();

	void updateImage(std::shared_ptr<cv::Mat>& img);

signals:
	void newFrameZonePosition(const QPoint &originalPos, float zoom);

protected:
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void wheelEvent(QWheelEvent *event) override;

private:
	std::shared_ptr<cv::Mat> _img;

	QSize _frameDisplayImageSize = { 0,0 };
	QRect _frameBoxRect = { 0,0,0,0 };
	QPoint _zonePoint = {0, 0};
	double _imgRatio = 0.0;
	double _lastFrameImageRatio = 0.0;
	int _frameBoxWidth = 0;
	int _frameBoxHeight = 0;
	int _frameBoxRatio = 0;
	int _sourceWidth = 0;
	int _sourceHeight = 0;

	bool _isValidImage = false;
	float _zoom = 1.0f;
};
