#pragma once

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QRubberBand>
#include <QVector>
#include "moveableLayerWidget.h"

class roiRectWidget;

class QScrollBar;
QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram);
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)


class RenderWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

private:
	int ImageWidth;
	int ImageHeight;
	int ImageUpperWidth;
	int ImageUpperHeight;

public:
	RenderWidget(QWidget *parent = 0);
	~RenderWidget();

	void assignScrollBars(QScrollBar *horz, QScrollBar *vert);

	void setScales( float glScale, float imageScale );
	void setScrolls ( int hScrollVal, int vScrollVal );

	void zoomIn() ;
	void zoomOut();

	void showActualPixels ();
	void showFitOnScreen ();
	void setZoom ( int zoom  );

	bool setImage(const QString& file);
	QSize getImageSize () const { return _hasImage ? _imageSize : QSize{}; }
	
	void updateHScroll( int hVal);
	void updateVScroll( int vVal);

signals:

	void cursorPos(QPoint pt, QSize rectSize);
	void scaleChanged( double newGLScale, double newImageScale );

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

	void updateLayers();
	void updateScroll();

	void wheelEvent(QWheelEvent* event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	bool eventFilter(QObject* obj, QEvent* event) override;

	GLfloat _glScale, _imageScale = 1.0;
	GLfloat _offsetX = 0.0;
	GLfloat _offsetY = 0.0;

	QOpenGLTexture* _texture;
	QOpenGLShaderProgram *_program;
	QOpenGLBuffer _vbo;

	QSize _imageSize;
	QSize _startWidgetSize;
	double _imageRatio = 1L;

	bool _hasImage = false;

	int _drawnImageLeft = 0;
	int _drawnImageWidth = 0;
	int _drawnImageTop = 0;
	int _drawnImageHeight = 0;

	QPoint _origin;
	QRubberBand * _rubberBand = nullptr;

	QScrollBar *_horz = nullptr;
	QScrollBar *_vert = nullptr;

	QVector<roiRectWidget*> _roiRectArray;

	moveableLayerWidget * _triangleCross;

};