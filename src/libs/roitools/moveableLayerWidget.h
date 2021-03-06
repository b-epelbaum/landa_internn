#pragma once

#include <QWidget>

class moveableLayerWidget : public QWidget
{
	Q_OBJECT

public:

	enum CROSS_TYPE {CROSS_I2S, CROSS_SPOT_FIRST, CROSS_SPOT_OTHER, CROSS_EDGE };
	moveableLayerWidget(
			  QWidget* parent
			, CROSS_TYPE crossType
			, int width
			, int height
			, int circleDiameter
			, QPoint startPt
			, QColor lineColor
			, Qt::PenStyle penStyle
			, float initScale );

	~moveableLayerWidget();
	
	void showCross(bool bShow);

	QSize originalSize() const { return _originalSize; }
	void enableXChange( bool bEnable ) { _canChangeX = bEnable; }

	void setTopLeftOnOriginalImage (QPoint pt) {_topLeftOnOriginalImage = pt; }
	QPoint topLeftOnOriginalImage() const { return _topLeftOnOriginalImage; }

	void setInitialScale( const float scale ) { _initialScale = scale; }
	float initialScale () const { return _initialScale; }
	
	void mapOnActualImage(double glScale, QPointF pt)
	{
		_scaleRatio = glScale / _initialScale;
		setGeometry( 
				lround(pt.x()), 
				lround(pt.y()), 
				lround(float(_originalSize.width()) * _scaleRatio ), 
				lround(float(_originalSize.height()) * _scaleRatio )
		);
		_topLeftOnActualImageInFloat = pt;
	}
	
	QPoint getCenterPoint () const;
	QPoint getCenterPoint (QPoint topLeft) const;


signals:

	 void movingOver(QPoint pt);
	 void crossMoving(QPoint center);
	 void crossMoved( QPoint topLeft, QPoint center );

protected:

	void paintI2SCross(QPainter& painter) const;
	void paintC2CCross(QPainter& painter) const;
	void paintEdgeCross(QPainter& painter) const;

	void paintEvent(QPaintEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	float	_initialScale = 1.0;
	QPoint _dragPosition { -1, -1 };
	QPoint _lastDropPosition { -1, -1 };

	CROSS_TYPE _crossType = CROSS_I2S;

	QSize	_originalSize = {0,0};
	QSize	_scaledSize = {0,0};

	int		_circleDiameter = 0;

	QPoint	_topLeftOnOriginalImage = {0,0};
	QPoint	_topLeftOnActualImage = {0,0};

	int _staticROIXValue = 0;

	QPointF _topLeftOnActualImageInFloat;
	float	_scaleRatio = 1.0;
	bool _canChangeX = true;

	QColor _lineColor;
	Qt::PenStyle _penStyle = Qt::SolidLine;
};
