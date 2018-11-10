#pragma once

#include <QLabel>
#include <QPoint>

#include <memory>

namespace cv {
	class Mat;
}

class FrameZone : public QLabel
{
	Q_OBJECT

public:
	FrameZone(QWidget *parent = Q_NULLPTR);
	~FrameZone();

	void updateFrameZone(std::shared_ptr<cv::Mat> &img);

public slots:
	void onNewFrameZonePosition(const QPoint &point, float zoom);

signals:
	void positionChanged(QPoint pos, QSize size, float ratio = 1.0f);

protected:
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void wheelEvent(QWheelEvent *event) override;

private:
	void calculateSize();
	void calculatePosition(const QPoint& newPoint);
	void prepareNewFrame();
	void emitPositionChanged();

private:
	QPoint _lastMousePos;
	QPoint _frameZonePosition = { 0, 0 };
	QSize _frameSize = { 0, 0 };
	QSize _windowsSize = { 0, 0 };
	float _zoom = 1.0f;

	std::shared_ptr<cv::Mat> _img;
};
