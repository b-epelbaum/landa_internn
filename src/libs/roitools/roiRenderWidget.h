#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QRubberBand>
#include <QVector>
#include <QVector4D>
#include <QMatrix4x4>
#include "moveableLayerWidget.h"
#include "common/june_errors.h"

class roiRectWidget;

class QScrollBar;
QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram);
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)


class roiRenderWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

private:
	int ImageWidth{};
	int ImageHeight{};
	int ImageUpperWidth{};
	int ImageUpperHeight{};

public:
	roiRenderWidget(QWidget *parent = 0);
	~roiRenderWidget();

	bool hasImage() const { return _hasImage; }
	void setInitialROIs(const QRect& is2sRc, const QVector<QRect>& c2cRects, QSize i2sMargins, QSize c2cMargins, int c2cCircleDiameter );
	void updateROIs(const QRect& is2sRc, const QVector<QRect>& c2cRects);
	void redrawAll();

	void assignScrollBars(QScrollBar *horz, QScrollBar *vert);

	void updateScaleFromExternal( double glScale, double imageScale );

	void zoomIn() ;
	void zoomOut();

	void showActualPixels ();
	void showFitOnScreen ();
	void setZoom ( int zoom  );

	LandaJune::CORE_ERROR setImage(const QString& file);
	QSize getImageSize () const { return _hasImage ? _imageSize : QSize{}; }

	
	void addRect(const QRect & rc) { _c2cROIRects.push_back(rc); }
	void cleanRects(void) { _i2sROIRect = {}; _c2cROIRects.clear(); }

signals:

	void cursorPos(QPoint pt, QSize rectSize);
	void scaleChanged( double newGLScale, double newImageScale );

	void i2sPosChanged(QPoint pt);
	void c2cPosChanged( int idx, QPoint pt);

private slots:

	void onCrossMovingOver( QPoint pt );
	void onCrossMoving( QPoint centerPos );
	void onI2SCrossMoved( QPoint topLeft, QPoint centerPos );
	void onC2CrossMoved( QPoint topLeft, QPoint centerPos );

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

	void cleanup();

	void createCrossHairs();
	void showCrossHairs(bool bShow);
	void paintROIs(QMatrix4x4& modelData );

	QPoint fromWidget2OriginalImagePt(const QPointF & pt);
	QSizeF  fromWidget2OriginalImageSize(const QSize & sz);
	QPointF fromOrigImage2WidgetPt(const QPoint & pt);

	QMatrix4x4 getModelViewProjMatrix(void);

	void updateInternalLayers();
	void updateInternalScrolls();

	void wheelEvent(QWheelEvent* event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	bool eventFilter(QObject* obj, QEvent* event) override;

	GLfloat _glScale = 1.0;
	GLfloat _imageScaleRatio = 1.0;
	GLfloat _actualPixelsScaleRatio = 1.0;
	GLfloat _imageRatio = 1.0;

	GLfloat _offsetX = 0.0;
	GLfloat _offsetY = 0.0;

	QOpenGLTexture* _texture;
	QOpenGLShaderProgram *_program;
	QOpenGLBuffer _vbo;

	QSize _imageSize;
	//QSize _startWidgetSize;
	
	bool _hasImage = false;

	int _drawnImageLeft = 0;
	int _drawnImageWidth = 0;
	int _drawnImageTop = 0;
	int _drawnImageHeight = 0;

	// rubberband values
	QPoint _origin;
	QRubberBand * _rubberBand = nullptr;

	QScrollBar *	_horizontalScrollbar = nullptr;
	QScrollBar *	_verticalScrollbar = nullptr;
	
	moveableLayerWidget * _i2sCross = nullptr;;
	QVector<moveableLayerWidget *> _c2cCrosses;
	QVector<moveableLayerWidget *> _allCrossesArray;;

	QRect _i2sROIRect;
	QVector<QRect> _c2cROIRects;

	int	_i2sMarginX = 0;
	int	_i2sMarginY = 0;
	int	_c2cMarginX = 0;
	int	_c2cMarginY = 0;
	int _c2cCircleDiameter = 0;

	static QSize _maxTextureSize;
	QSize _initialWidgetSize = {};

};