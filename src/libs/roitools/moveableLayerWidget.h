#pragma once

#include <QWidget>

class moveableLayerWidget : public QWidget
{
	Q_OBJECT

public:

	enum CROSS_TYPE {CROSS_I2S, CROSS_SPOT_FIRST, CROSS_SPOT_OTHER};
	moveableLayerWidget(QWidget* parent, CROSS_TYPE crossType, int width, int height, QPoint startPt );
	~moveableLayerWidget();
	void showCross(bool bShow);

	QSize originalSize() const { return _originalSize; }

	void setTopLeftOnOriginalImage (QPoint pt) {_topLeftOnOriginalImage = pt; }
	QPoint topLeftOnOriginalImage() const { return _topLeftOnOriginalImage; }
	
	void mapOnActualImage(float scale, QPoint pt) { setGeometry(pt.x(), pt.y(), _originalSize.width() * scale, _originalSize.height() * scale);_topLeftOnActualImage = pt; /*move(pt);*/ }


signals:

	 void crossMoved( QPoint topLeft, QPoint center );

protected:

	QPoint getCenterPoint (QPoint topLeft) const;

	void paintI2SCross(QPainter& painter);
	void paintC2CCross(QPainter& painter);

	void paintEvent(QPaintEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	QPoint _dragPosition { -1, -1 };
	QPoint _lastDropPosition { -1, -1 };

	CROSS_TYPE _crossType = CROSS_I2S;

	QSize	_originalSize = {0,0};
	QSize	_scaledSize = {0,0};

	QPoint	_topLeftOnOriginalImage = {0,0};
	QPoint	_topLeftOnActualImage = {0,0};
};
