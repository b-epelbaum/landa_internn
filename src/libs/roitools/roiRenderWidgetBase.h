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

QT_FORWARD_DECLARE_CLASS(QScrollBar)
QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

struct JGL_COLOR
{
	float _r = 0;
	float _g = 0;
	float _b = 0;
	float _alpha = 0;
};

static const JGL_COLOR JGL_NO_COLOR  = {};

static const QColor		I2S_COLOR =				{255,0,0,255};
static const JGL_COLOR	I2S_COLOR_FRAME =		{1.0,	0.0,	0.0,	1.0};
static const QColor		C2C_COLOR =				{169,	168,	0,		255};
static const JGL_COLOR	C2C_COLOR_FRAME =		{0.55,	0.55,	0.0,	1.0};
static const QColor		STRIP_MARGIN_COLOR =	{0,		0,		255,	255};
static const JGL_COLOR	STRIP_COLOR_FILL =		{0.0,	0.0,	1.0,	0.2};
static const QColor		PAGE_OFFSET_COLOR =		{255,	255,	0,		255};

static const Qt::PenStyle SOLID_PEN =		Qt::SolidLine;
static const Qt::PenStyle DOT_PEN =			Qt::DotLine;



using glDrawFunc = std::function<void(const QRect& rc, 
					bool bDrawRect,			
					JGL_COLOR rectColor,
					bool bFillBG,
					JGL_COLOR)>;

class roiRenderWidgetBase : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	roiRenderWidgetBase(QWidget *parent = nullptr);
	virtual ~roiRenderWidgetBase();

	bool hasImage() const { return _hasImage; }
	
	virtual void redrawAll();

	void setAlias( const QString strAlias ) { _alias = strAlias; }
	void assignScrollBars(QScrollBar *horz, QScrollBar *vert);

	void updateScaleFromExternal( double glScale, double imageScale );

	void zoomIn() ;
	void zoomOut();

	void showActualPixels ();
	void showFitOnScreen ();
	void setZoom ( int zoom  );

	virtual LandaJune::CORE_ERROR setImage(const QString& file);
	QSize getImageSize () const { return _hasImage ? _imageSize : QSize{}; }

signals:

	void cursorPos(QPoint pt, QSize rectSize);
	void scaleChanged( double newGLScale, double newImageScale );
	void doubleClick( QPoint pos );

public slots:

	void onCrossMovingOver( QPoint pt );
	void onCrossMoving( QPoint centerPos );
	void onROIControlPointMoved( QPoint topLeft, QPoint centerPos );

protected:

	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

	void mouseDoubleClickEvent(QMouseEvent* event) override;

	virtual void cleanup();
	virtual void createCrossHairs( float creationScale ) {}
	virtual void paintROIRects( glDrawFunc func ) {}
	virtual void GLpaintROIs(QMatrix4x4& modelData );
	virtual QVector<QPoint> gatherROICenterPoints();

	QPoint		fromWidget2OriginalImagePt(const QPointF & pt);
	QSizeF		fromWidget2OriginalImageSize(const QSize & sz);
	QPointF		fromOrigImage2WidgetPt(const QPoint & pt);
	QMatrix4x4	getModelViewProjMatrix(void);

	virtual void updateInternalLayers();
	virtual void updateInternalScrolls();

	virtual void handleROIControlPointMoved( moveableLayerWidget* sender, QPoint topLeft, QPoint centerPos ) {}

	void wheelEvent(QWheelEvent* event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	bool eventFilter(QObject* obj, QEvent* event) override;

	QString _alias;
	GLfloat _glScale = 1.0;
	GLfloat _imageScaleRatio = 1.0;
	GLfloat _actualPixelsScaleRatio = 1.0;
	GLfloat _imageRatio = 1.0;

	GLfloat _offsetX = 0.0;
	GLfloat _offsetY = 0.0;

	QOpenGLTexture* _texture = nullptr;
	QOpenGLShaderProgram *_program = nullptr;
	QOpenGLBuffer _vbo;

	QSize _imageSize = {};
	bool _hasImage = false;
	
	// rubberband values
	QPoint _origin = {};
	QRubberBand * _rubberBand = nullptr;

	QScrollBar *	_horizontalScrollbar = nullptr;
	QScrollBar *	_verticalScrollbar = nullptr;
	
	QVector<moveableLayerWidget *> _allCrossesArray;;
	static QSize _maxTextureSize;
	QSize _initialWidgetSize = {};

};