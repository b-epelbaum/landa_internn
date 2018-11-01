#include "roiRenderWidget.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include <QMouseEvent>
#include <QScrollBar>
#include <QPainter>


#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include "applog.h"


#ifdef _DEBUG
#pragma comment(lib, "opencv_world342d.lib")
#else
#pragma comment(lib, "opencv_world342.lib")
#endif

#define MAX_SCALE 5.0
#define MIN_SCALE 0.20

#define ZOOM_STEP 1.2

using namespace LandaJune;

bool double_equals(double a, double b, double epsilon = 0.001)
{
    return std::abs(a - b) < epsilon;
}

QSize roiRenderWidget::_maxTextureSize = {0,0};

roiRenderWidget::roiRenderWidget(QWidget *parent)
	: QOpenGLWidget(parent)
	, _glScale(1.0f)
	, _texture(nullptr)
	, _program(nullptr)
	, _imageSize(1, 1)
{
	setMouseTracking(true);
	//parentWidget()->installEventFilter(this);
}

roiRenderWidget::~roiRenderWidget()
{
	makeCurrent();
	cleanup();

	if (_vbo.bufferId())
	{
		_vbo.destroy();
	}

	if (_program)
	{
		delete _program;
	}
	doneCurrent();
}

void roiRenderWidget::cleanup()
{
	if (_texture)
	{
		delete _texture;
	}
	_texture = nullptr;

	if (_i2sCross != nullptr )
		delete _i2sCross;

	_i2sCross = nullptr;

	if (!_c2cCrosses.isEmpty())
		qDeleteAll(_c2cCrosses);

	_c2cCrosses.clear();
	_allCrossesArray.clear();


	_glScale = 1.0;
	_imageScaleRatio = 1.0;
	_actualPixelsScaleRatio = 1.0;

	_offsetX = 0.0;
	_offsetY = 0.0;

	_imageSize = {};
	_imageRatio = 1.0;
	_hasImage = false;

	_drawnImageLeft = 0;
	_drawnImageWidth = 0;
	_drawnImageTop = 0;
	_drawnImageHeight = 0;

	_origin = {};
	_initialWidgetSize = {};
}


CORE_ERROR roiRenderWidget::setImage(const QString& file)
{
	cleanup ();
	makeCurrent();

	cv::Mat cvImage = cv::imread(file.toStdString());
	if (cvImage.dims != 2)
		{
		return CORE_ERROR::UI_ROITOOL_INVALID_IMAGE;
	}

	if ( cvImage.cols > _maxTextureSize.width() || cvImage.rows > _maxTextureSize.height())
	{
		return CORE_ERROR::UI_ROITOOL_IMAGE_TOO_BIG;
	}

	cv::cvtColor(cvImage, cvImage, CV_BGR2RGB);

	QImage img(static_cast<unsigned char*>(cvImage.data), cvImage.cols, cvImage.rows, cvImage.step, QImage::Format_RGB888);
	
	_texture = new QOpenGLTexture(img);
	_imageSize = img.size();

	_imageRatio = static_cast<double>(_imageSize.width()) / static_cast<double>(_imageSize.height());

	if ( _imageRatio < 1 ) // vertical image
	{
		_glScale = static_cast<double>(_imageSize.height()) /  static_cast<double>(height());
	}
	else
	{
		_glScale =  static_cast<double>(_imageSize.width()) /  static_cast<double>(width());
	}
	_imageScaleRatio = _glScale;
	_actualPixelsScaleRatio = _glScale;

	qDebug() << " **** INITIAL SCALE : " << _glScale;

	_hasImage = true;
	setCursor(Qt::CrossCursor);

	doneCurrent();
	createCrossHairs();
	repaint();

	emit scaleChanged(_glScale, _glScale / _imageScaleRatio );
	return RESULT_OK;
}


void roiRenderWidget::createCrossHairs()
{		
	delete _i2sCross;

	if (!_i2sROIRect.isEmpty())
	{
		_i2sCross = new moveableLayerWidget(this , moveableLayerWidget::CROSS_I2S, _i2sMarginX * 2, _i2sMarginY * 2, _c2cCircleDiameter, _i2sROIRect.topLeft(), _glScale);
		connect (_i2sCross, &moveableLayerWidget::crossMoved, this, &roiRenderWidget::onI2SCrossMoved );
	}
	
	if (!_c2cCrosses.isEmpty())
		qDeleteAll(_c2cCrosses);

	if (_c2cROIRects.isEmpty())
	{
		return;
	}

	auto c2cCross = new moveableLayerWidget(this , moveableLayerWidget::CROSS_SPOT_FIRST, _c2cMarginX*2 + _c2cCircleDiameter, _c2cMarginY*2 + _c2cCircleDiameter, _c2cCircleDiameter, _c2cROIRects[0].topLeft(), _glScale);
	c2cCross->setProperty("idx", 0);
	_c2cCrosses.push_back(c2cCross);


	for (auto i = 1; i < _c2cROIRects.size(); i++ )
	{
		auto c2c = new moveableLayerWidget(this , moveableLayerWidget::CROSS_SPOT_FIRST, _c2cMarginX*2 + _c2cCircleDiameter, _c2cMarginY*2 + _c2cCircleDiameter, _c2cCircleDiameter, _c2cROIRects[i].topLeft(), _glScale);
		c2c->setProperty("idx", i);
		_c2cCrosses.push_back(c2c);
	}

	for ( auto const c2c : _c2cCrosses)
	{
		connect (c2c, &moveableLayerWidget::crossMoved, this, &roiRenderWidget::onC2CrossMoved );
	}

	_allCrossesArray << _i2sCross << _c2cCrosses;

	for ( auto elem : _allCrossesArray)
	{
		connect (elem, &moveableLayerWidget::crossMoving, this, &roiRenderWidget::onCrossMoving );
		connect (elem, &moveableLayerWidget::movingOver, this, &roiRenderWidget::onCrossMovingOver );
	}
	updateInternalLayers();
}

void roiRenderWidget::assignScrollBars(QScrollBar* horz, QScrollBar* vert)
{
	_horizontalScrollbar = horz; _verticalScrollbar = vert;
}

void roiRenderWidget::setInitialROIs(const QRect& is2sRc, const QVector<QRect>& c2cRects, QSize i2sMargins, QSize c2cMargins, int c2cCircleDiameter )
{
	_i2sROIRect = is2sRc;
	_c2cROIRects = c2cRects;

	_i2sMarginX = i2sMargins.width();
	_i2sMarginY = i2sMargins.height();

	_c2cMarginX = c2cMargins.width();
	_c2cMarginY = c2cMargins.height();

	_c2cCircleDiameter = c2cCircleDiameter;

	if(_hasImage )
		update();
}


void roiRenderWidget::updateROIs(const QRect& is2sRc, const QVector<QRect>& c2cRects)
{
	//qDebug () << " ---- CALCULATED I2S RECT : " << is2sRc;

	//_i2sROIRect = is2sRc;
	//_c2cROIRects = c2cRects;
	
	//if(_hasImage )
	//	update();
}

void roiRenderWidget::redrawAll()
{
	updateInternalLayers();
	update();
}

void roiRenderWidget::showCrossHairs(bool bShow)
{
	if (_i2sCross )
		_i2sCross->showCross(bShow);

	for ( auto const& cross : _c2cCrosses )
	{
		if ( cross )
			cross->showCross(bShow);
	}
}

void roiRenderWidget::updateScaleFromExternal(double glScale, double imageScale)
{
	if(!_hasImage )
		return;

	_glScale = glScale;
	updateInternalScrolls();

	/*
	PRINT_WARNING << "Target Vscroll data : \r\n" 
	<< "\trange : " << _verticalScrollbar->minimum() << " ==> " << _verticalScrollbar->maximum()
	<< "\tpage step : " << _verticalScrollbar->pageStep() 
	<< "\tsingle step : " << _verticalScrollbar->singleStep()
	<< "\tvalue : " << _verticalScrollbar->value();
	*/
}

void roiRenderWidget::showActualPixels ()
{
	if(!_hasImage )
		return;

	if ( _imageRatio < 1 ) // vertical image
	{
		_glScale = static_cast<double>(_imageSize.height()) /  static_cast<double>(height());
	}
	else
	{
		_glScale =  static_cast<double>(_imageSize.width()) /  static_cast<double>(width());
	}
	updateInternalScrolls();
	emit scaleChanged(_glScale, _glScale / _imageScaleRatio );
}
	
void roiRenderWidget::showFitOnScreen ()
{
	if(!_hasImage )
		return;

	_glScale = 1.0;
	updateInternalScrolls();
	emit scaleChanged(_glScale, _glScale / _imageScaleRatio );
}
	
void roiRenderWidget::setZoom ( int zoom  )
{
	if(!_hasImage )
		return;

	_glScale = _imageScaleRatio * (float)zoom / 100;
	updateInternalScrolls();
	emit scaleChanged(_glScale, _glScale / _imageScaleRatio );
}

void roiRenderWidget::zoomIn()
{
	if(!_hasImage )
		return;

	if (double_equals(_glScale/_imageScaleRatio, 5) )
		return;


	_glScale =  _glScale * ZOOM_STEP;
	if ( _glScale / _imageScaleRatio > 5.0)
	{
		_glScale = _imageScaleRatio * 5.0;
	}

	qDebug() << " **** SCALE : " << _glScale;
	emit scaleChanged(_glScale, _glScale / _imageScaleRatio );
	updateInternalScrolls();

	PRINT_WARNING << "Source Vscroll data : \r\n" 
	<< "\trange : " << _verticalScrollbar->minimum() << " ==> " << _verticalScrollbar->maximum()
	<< "\tpage step : " << _verticalScrollbar->pageStep() 
	<< "\tsingle step : " << _verticalScrollbar->singleStep()
	<< "\tvalue : " << _verticalScrollbar->value();
}
	
void roiRenderWidget::zoomOut()
{
	if(!_hasImage )
		return;

	if (double_equals(_glScale/_imageScaleRatio, 0.1) )
		return;

	_glScale =  _glScale / ZOOM_STEP;
	if ( _glScale / _imageScaleRatio < 0.1)
	{
		_glScale = _imageScaleRatio * 0.1;
	}

	qDebug() << " **** SCALE : " << _glScale;
	emit scaleChanged(_glScale, _glScale / _imageScaleRatio );
	updateInternalScrolls();

	PRINT_WARNING << "Source Vscroll data : \r\n" 
	<< "\trange : " << _verticalScrollbar->minimum() << " ==> " << _verticalScrollbar->maximum()
	<< "\tpage step : " << _verticalScrollbar->pageStep() 
	<< "\tsingle step : " << _verticalScrollbar->singleStep()
	<< "\tvalue : " << _verticalScrollbar->value();
}


QPoint roiRenderWidget::fromWidget2OriginalImagePt(const QPointF & pt) 
{
	if(!_hasImage )
		return pt.toPoint();

	QSizeF widgetSize = size();
	GLfloat xn = 2 * pt.x() / GLfloat(widgetSize.width()) - 1;
	GLfloat yn = 2 * (widgetSize.height() - pt.y()) / GLfloat(widgetSize.height()) - 1;
	QMatrix4x4 m = getModelViewProjMatrix().inverted();
	QVector4D r = m * QVector4D(xn, yn, 0, 1);
	const auto x = (r.x() + 1) * (float)_imageSize.width() / 2;
	const auto y = (1 - r.y()) *  (float)_imageSize.height() / 2;
	
	QPoint result(lround(x), lround(y));
	return std::move(result);
}

QSizeF roiRenderWidget::fromWidget2OriginalImageSize(const QSize & sz) 
{
	if(!_hasImage )
		return sz;

	QSizeF widgetSize = size();
	GLfloat xn = 2 * sz.width() / GLfloat(widgetSize.width()) - 1;
	GLfloat yn = 2 * (widgetSize.height() - sz.height()) / GLfloat(widgetSize.height()) - 1;
	QMatrix4x4 m = getModelViewProjMatrix().inverted();
	QVector4D r0 = m * QVector4D(-1, 1, 0, 1);
	QVector4D r1 = m * QVector4D(xn, yn, 0, 1);
	float x0 = (r0.x() + 1) * (float)_imageSize.width() / 2;
	float y0 = (1 - r0.y()) * (float)_imageSize.height() / 2;
	float x1 = (r1.x() + 1) * (float)_imageSize.width() / 2;
	float y1 = (1 - r1.y()) * (float)_imageSize.height() / 2;
	QSizeF result(x1 - x0, y1 - y0);
	return std::move(result);
}

// backward
QPointF roiRenderWidget::fromOrigImage2WidgetPt(const QPoint & pt)
{
	if(!_hasImage )
		return pt;

	QSize widgetSize = size();
	GLfloat rx = 2 * pt.x() / GLfloat(_imageSize.width()) - 1;
	GLfloat ry = 1 - 2 * pt.y() / GLfloat(_imageSize.height());
	QMatrix4x4 m = getModelViewProjMatrix();
	QVector4D rn = m * QVector4D(rx, ry, 0, 1);
	float xw = (1 + rn.x()) * widgetSize.width() / 2;
	float yw = widgetSize.height() - (1 + rn.y()) * widgetSize.height() / 2;

	QPointF result(xw, yw);
	return std::move(result);
}

void roiRenderWidget::onCrossMovingOver(QPoint pt)
{
	emit cursorPos(fromWidget2OriginalImagePt(mapFromGlobal(pt)), QSize{});
}

void roiRenderWidget::onCrossMoving(QPoint centerPos)
{
	auto targetWidget = dynamic_cast<moveableLayerWidget*>(sender());

	auto newCenterPos = fromWidget2OriginalImagePt(centerPos);
	emit cursorPos(newCenterPos, QSize{});
}

void roiRenderWidget::onI2SCrossMoved(QPoint topLeft, QPoint centerPos )
{
	auto targetWidget = dynamic_cast<moveableLayerWidget*>(sender());

	// transpose from widget coordinates to original image coordinates 
	const auto newI2SCenter = fromWidget2OriginalImagePt(centerPos);
	qDebug () << " ---- LOCAL I2S CENTER PT : " << newI2SCenter;
	
	const auto newLeftToPos = fromWidget2OriginalImagePt(topLeft);

	targetWidget->setTopLeftOnOriginalImage(newLeftToPos);

	_i2sROIRect = QRect(newLeftToPos.x(), newLeftToPos.y(), _i2sROIRect.width(), _i2sROIRect.height());

	qDebug () << " ---- LOCAL I2S RECT : " << _i2sROIRect;
	emit i2sPosChanged(newI2SCenter);
	update();
}

void roiRenderWidget::onC2CrossMoved( QPoint topLeft, QPoint centerPos )
{
	auto targetWidget = dynamic_cast<moveableLayerWidget*>(sender());

	auto newC2CPos = fromWidget2OriginalImagePt(centerPos);
	const auto newLeftToPos = fromWidget2OriginalImagePt(topLeft);

	targetWidget->setTopLeftOnOriginalImage(newLeftToPos);

	auto idx = targetWidget->property("idx").toInt();

	_c2cROIRects[idx] = QRect(newLeftToPos.x(), newLeftToPos.y(), _c2cROIRects[idx].width(), _c2cROIRects[idx].height());

	qDebug () << " ---- NEW C2C RECT [idx=" << idx << "]: " << _c2cROIRects[idx] ;
	
	emit c2cPosChanged(idx, newC2CPos);
	update();
}

void roiRenderWidget::initializeGL()
{
	initializeOpenGLFunctions();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	if (_maxTextureSize.isEmpty())
	{
		GLint dims[2];
		glGetIntegerv(GL_MAX_VIEWPORT_DIMS, dims);
		_maxTextureSize.setWidth(dims[0]);
		_maxTextureSize.setHeight(dims[1]);
	}

	static const GLfloat vertData[] = {
		-1, -1, 0, 0, 1,
		+1, +1, 0, 1, 0,
		-1, +1, 0, 0, 0,

		-1, -1, 0, 0, 1,
		+1, -1, 0, 1, 1,
		+1, +1, 0, 1, 0
	};

	_vbo.create();
	_vbo.bind();
	_vbo.allocate(vertData, sizeof(vertData));

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

	QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
	const char *vsrc =
		"uniform mediump mat4 matrix;\n"
		"attribute highp vec3 vertex;\n"
		"attribute mediump vec2 texCoord;\n"
		"varying mediump vec2 texc;\n"
		"void main(void)\n"
		"{\n"
		"    gl_Position = matrix * vec4(vertex, 1);\n"
		"    texc = texCoord;\n"
		"}\n";

	if (!vshader->compileSourceCode(vsrc)) {
		throw (vshader->log());
	}

	QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
	const char *fsrc =
		"uniform sampler2D texture;\n"
		"varying mediump vec2 texc;\n"
		"void main(void)\n"
		"{\n"
		"    gl_FragColor = texture2D(texture, texc.st);\n"
		"}\n";
	if (!fshader->compileSourceCode(fsrc)) {
		throw (fshader->log());
	}

	_program = new QOpenGLShaderProgram;
	_program->addShader(vshader);
	_program->addShader(fshader);
	_program->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
	_program->bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);
	if (!_program->link()) {
		throw (_program->log());
	}

	_program->bind();
	_program->setUniformValue("texture", 0);
	_program->release();

}

void roiRenderWidget::paintGL()
{
	if(!_hasImage )
		return;

	QPainter painter (this);
	painter.beginNativePainting();
	glClearColor(0, 0, 0.0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	QMatrix4x4 m = getModelViewProjMatrix();

	_vbo.bind();
	_program->bind();
	_program->setUniformValue("matrix", m);
	_program->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
	_program->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
	_program->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
	_program->setAttributeBuffer(PROGRAM_TEXCOORD_ATTRIBUTE, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));

	if (_texture) 
	{
		_texture->bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		_texture->release();
	}

	_program->release();
	_vbo.release();

	paintROIs(m);

	/*
	glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

	painter.endNativePainting();
	painter.fillRect(10,10, 100, 100, QColor(Qt::darkBlue));
	*/
}

void roiRenderWidget::paintROIs(QMatrix4x4& modelData )
{
	// TODO: replace it with modern approach
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelData.data());

	auto imageScale = _glScale / _imageScaleRatio;
//	if ( imageScale > 1 && imageScale < 2 )
	
	glLineWidth(imageScale * 2);
	glColor3d(1, 0, 0);

	auto drawLambda = [this](const QRect& rc)
	{
		GLfloat l = 2 * rc.left() / GLfloat(_imageSize.width()) - 1;
		GLfloat t = 1 - 2 * rc.top() / GLfloat(_imageSize.height());
		GLfloat r = 2 * rc.right() / GLfloat(_imageSize.width()) - 1;
		GLfloat b = 1 - 2 * rc.bottom() / GLfloat(_imageSize.height());

		glBegin(GL_LINE_LOOP);
		glVertex3f(l, t, 0.1);
		glVertex3f(r, t, 0.1);
		glVertex3f(r, b, 0.1);
		glVertex3f(l, b, 0.1);
		glEnd();
	};

	drawLambda(_i2sROIRect);
	for (QRect rc : _c2cROIRects) 
	{
		drawLambda(rc);
	}
}


QMatrix4x4 roiRenderWidget::getModelViewProjMatrix(void) 
{
	QSize widgetSize = size();
	
	GLfloat ratioX = _imageSize.width() / GLfloat(widgetSize.width());
	GLfloat ratioY = _imageSize.height() / GLfloat(widgetSize.height());
	
	if (ratioX < ratioY)
	{
		ratioX /= ratioY; 
		ratioY = 1;
	}
	else
	{
		ratioY /= ratioX; ratioX = 1;
	}

	ratioX *= _glScale; 
	ratioY *= _glScale;

	_offsetY = 0;
	if (_verticalScrollbar && ratioY > 1.0f) 
	{
		_offsetY = (2 * _verticalScrollbar->value() / GLfloat(_verticalScrollbar->maximum()) - 1) * (ratioY - 1.0f);
	}
	_offsetX = 0;
	if (_horizontalScrollbar && ratioX > 1.0f) 
	{
		_offsetX = (2 * _horizontalScrollbar->value() / GLfloat(_horizontalScrollbar->maximum()) - 1) * (ratioX - 1.0f);
	}

	QMatrix4x4 m;
	m.ortho(-1, 1, -1, 1, -1, 1);
	m.translate(-_offsetX, _offsetY, 0.0f);
	m.scale(ratioX, ratioY, 1.0f);
	return std::move(m);
}


void roiRenderWidget::updateInternalLayers()
{
	if(!_hasImage )
		return;

	std::for_each(std::begin(_allCrossesArray), std::end(_allCrossesArray), 
			[this](auto w)
			{
				if (w)
					w->mapOnActualImage(_glScale, fromOrigImage2WidgetPt(w->topLeftOnOriginalImage()));
			}
	);
}

void roiRenderWidget::resizeGL(int w, int h)
{
	if ( !_hasImage)
	{
		_initialWidgetSize = {w,h};
		glViewport(0, 0, w, h );
	}
	else
	{
		glViewport(0, 0, _initialWidgetSize.width(), _initialWidgetSize.height());
//		auto newScale = static_cast<float>(w) / static_cast<float>(_initialWidgetSize.width()) * _glScale;
//		_glScale = newScale;
//		qDebug() << " **** SCALE : " << _glScale;

	}
//	updateScroll();
}

/*
void roiRenderWidget::updateScrollFromExternal( int hScrollVal, int vScrollVal )
{
	if(!_hasImage )
		return;
	updateLayers();

	PRINT_WARNING << "Source Vscroll data : \r\n" 
		<< "\trange : " << _verticalScrollbar->minimum() << " ==> " << _verticalScrollbar->maximum() 
		<< "\tpage step : " << _verticalScrollbar->pageStep() 
		<< "\tsingle step : " << _verticalScrollbar->singleStep()
		<< "\tvalue : " << _verticalScrollbar->;

}
*/

void roiRenderWidget::updateInternalScrolls() 
{
	if(!_hasImage )
	{
		_horizontalScrollbar->setEnabled(false);
		_verticalScrollbar->setEnabled(false);
		return;
	}

	QRect qs = geometry();

	QSize vp = size();
	GLfloat ratioX = _imageSize.width() / GLfloat(vp.width());
	GLfloat ratioY = _imageSize.height() / GLfloat(vp.height());
	if (ratioX < ratioY)
	{
		ratioX /= ratioY; ratioY = 1;
	}
	else
	{
		ratioY /= ratioX; ratioX = 1;
	}

	ratioX *= _glScale; ratioY *= _glScale;

	if (_verticalScrollbar) 
	{
		const auto total = _imageSize.width();
		if (ratioY > 1.0f) 
		{
			const auto page = int(total / _glScale);
			_verticalScrollbar->setMinimum(0);
			_verticalScrollbar->setMaximum(total - page);
			_verticalScrollbar->setPageStep(page);
			_verticalScrollbar->setEnabled(true);
		}
		else 
		{
			_verticalScrollbar->setMinimum(0);
			_verticalScrollbar->setMaximum(total);
			_verticalScrollbar->setPageStep(total);
			_verticalScrollbar->setEnabled(false);
		}
	}

	if (_horizontalScrollbar) 
	{
		const auto total = _imageSize.height();
		if (ratioX > 1.0f) 
		{
			const auto page = int(total / ratioX);
			_horizontalScrollbar->setMinimum(0);
			_horizontalScrollbar->setMaximum(total - page);
			_horizontalScrollbar->setPageStep(page);
			_horizontalScrollbar->setEnabled(true);
		}
		else 
		{
			_horizontalScrollbar->setMinimum(0);
			_horizontalScrollbar->setMaximum(total);
			_horizontalScrollbar->setPageStep(total);
			_horizontalScrollbar->setEnabled(false);
		}
	}
	update();
	updateInternalLayers();
}

void roiRenderWidget::mouseMoveEvent(QMouseEvent* event)
{
	if(!_hasImage )
		return;

	auto pos = fromWidget2OriginalImagePt(event->pos());
	QSize sentSize;

	if (_rubberBand && _rubberBand->isVisible())
	{
		_rubberBand->setGeometry(QRect(_origin, event->pos()).normalized());
		sentSize = fromWidget2OriginalImageSize(_rubberBand->geometry().size()).toSize();
		//sentSize = sentSize / _imageScale;
		pos = fromWidget2OriginalImagePt(_rubberBand->geometry().topLeft()) ;
	}
	
	emit cursorPos(pos, sentSize);
	//emit cursorPos(event->pos() / _imageScale, sentSize / _imageScale);
}

void roiRenderWidget::mousePressEvent(QMouseEvent *event)
{
	if(!_hasImage )
		return;
	
	_origin = event->pos();

    if (!_rubberBand)
        _rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    _rubberBand->setGeometry(QRect(_origin, QSize()));
    _rubberBand->show();
}

void roiRenderWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if(!_hasImage )
		return;
    _rubberBand->hide();
}


bool roiRenderWidget::eventFilter(QObject* obj, QEvent* event)
{
	/*
	if(!_hasImage )
		return QObject::eventFilter(obj, event);

	if (event->type() == QEvent::Resize) 
	{
		auto newSize = size();
		qDebug() << " **** SIZE : " << newSize;
		if (_imageRatio < 1 ) // vertical
		{
			_imageScale = _imageScale * static_cast<double>(newSize.height()) / static_cast<double>(_startWidgetSize.height());
		}
		else
		{
			_imageScale = _imageScale * static_cast<double>(newSize.width()) / static_cast<double>(_startWidgetSize.width());
		}
		_startWidgetSize = newSize;
		emit scaleChanged(_glScale, _glScale / _imageScaleRatio );
		updateScroll();
	}
	*/
	return QObject::eventFilter(obj, event);
}


void roiRenderWidget::wheelEvent(QWheelEvent* event)
{

	if(event->modifiers().testFlag(Qt::ControlModifier))
	{
     	event->angleDelta().y() < 0 ? zoomOut() : zoomIn();
	}
	else
	{
		auto const val = event->angleDelta().y() < 0 ? _verticalScrollbar->pageStep() : -_verticalScrollbar->pageStep();
		_verticalScrollbar->setValue(_verticalScrollbar->value() + val / 2) ;
		update();
	}
}
