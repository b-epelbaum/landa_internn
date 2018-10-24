#include "RenderWidget.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include <QMouseEvent>
#include <QScrollBar>
#include <QPainter>

#include "roiRectWidget.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>


#ifdef _DEBUG
#pragma comment(lib, "opencv_world342d.lib")
#else
#pragma comment(lib, "opencv_world342.lib")
#endif

#define MAX_SCALE 5.0
#define MIN_SCALE 0.20

#define ZOOM_STEP 1.2

bool double_equals(double a, double b, double epsilon = 0.001)
{
    return std::abs(a - b) < epsilon;
}

RenderWidget::RenderWidget(QWidget *parent)
	: QOpenGLWidget(parent)
	, _glScale(1.0f)
	, _texture(nullptr)
	, _program(nullptr)
	, _imageSize(1, 1)
{
	setMouseTracking(true);
	parentWidget()->installEventFilter(this);

	_triangleCross = new moveableLayerWidget(this);
	_triangleCross->setGeometry(10, 10, 30, 30);
	_triangleCross->raise();
	_triangleCross->show();
}

RenderWidget::~RenderWidget()
{
	makeCurrent();
	if (_vbo.bufferId())
	{
		_vbo.destroy();
	}

	if (_texture)
	{
		delete _texture;
	}

	if (_program)
	{
		delete _program;
	}

	doneCurrent();
}

void RenderWidget::assignScrollBars(QScrollBar* horz, QScrollBar* vert)
{
	_horizontalScrollbar = horz; _verticalScrollbar = vert;
}

void RenderWidget::setScales(float glScale, float imageScale)
{
	if(!_hasImage )
		return;
	
	_imageScale = imageScale;
	_glScale = glScale;
	updateScroll();
}

void RenderWidget::setScrolls(int hScrollVal, int vScrollVal)
{
	if(!_hasImage )
		return;

	if ( hScrollVal != _horizontalScrollbar->value() )
		_horizontalScrollbar->setValue(hScrollVal);
	if ( vScrollVal != _verticalScrollbar->value() )
		_verticalScrollbar->setValue(vScrollVal);
	update();
}

void RenderWidget::showActualPixels ()
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

	_imageScale = 1.0;
	updateScroll();
	update();
	emit scaleChanged (_glScale, _imageScale);
}
	
void RenderWidget::showFitOnScreen ()
{
	if(!_hasImage )
		return;

	if ( _imageRatio < 1 ) // vertical image
	{
		_imageScale = static_cast<double>(height()) /  static_cast<double>(_imageSize.height());
	}
	else
	{
		_imageScale =  static_cast<double>(width()) /  static_cast<double>(_imageSize.width());
	}

	_glScale = 1.0;
	updateScroll();
	update();
	emit scaleChanged (_glScale, _imageScale);
}
	
void RenderWidget::setZoom ( int zoom  )
{
	if(!_hasImage )
		return;
}

void RenderWidget::zoomIn()
{
	if(!_hasImage )
		return;

	if (double_equals(_imageScale, MAX_SCALE ) )
		return;

	const auto newScale = _imageScale * ZOOM_STEP;
	if (newScale >= MAX_SCALE )
	{
		_imageScale = MAX_SCALE;
		return;
	}
	
	_imageScale = newScale;
	qDebug() << " **** SCALE : " << _imageScale;
	_glScale =  _glScale * ZOOM_STEP;
	
	emit scaleChanged(_glScale, _imageScale);
	updateScroll();
}
	
void RenderWidget::zoomOut()
{
	if(!_hasImage )
		return;

	if (double_equals(_imageScale, MIN_SCALE ) )
		return;

	const auto newScale = _imageScale / ZOOM_STEP;
	if (newScale <= MIN_SCALE )
	{
		_imageScale = MIN_SCALE;
		return;
	}
	
	_imageScale = newScale;
	qDebug() << " **** SCALE : " << _imageScale;
	_glScale =  _glScale / ZOOM_STEP;
	emit scaleChanged(_glScale, _imageScale);
	updateScroll();
}


bool RenderWidget::setImage(const QString& file)
{
	makeCurrent();

	cv::Mat cvImage = cv::imread(file.toStdString());
	cv::cvtColor(cvImage, cvImage, CV_BGR2RGB);

	QImage img(static_cast<unsigned char*>(cvImage.data), cvImage.cols, cvImage.rows, cvImage.step, QImage::Format_RGB888);
	if (_texture)
	{
		delete _texture;
	}
	_texture = new QOpenGLTexture(img);
	_imageSize = img.size();
	

	_imageScale = 1.0;
	_imageRatio = (double)_imageSize.width() / (double)_imageSize.height();

	if ( _imageRatio < 1 ) // vertical image
	{
		_glScale = (double)_imageSize.height() /  (double)height();
	}
	else
	{
		_glScale =  (double)_imageSize.width() /  (double)width();
	}

	_hasImage = true;
	_startWidgetSize = size();
	setCursor(Qt::CrossCursor);

	addBox({14, 20, 100, 100});

	doneCurrent();
	repaint();

	emit scaleChanged(_glScale, _imageScale);
	return true;
}

QPoint RenderWidget::toImageC(const QPoint & pt) 
{
	if(!_hasImage )
		return pt;

	QMatrix4x4 m = getModelViewProjMatrix().inverted();
	QSize widgetSize = size();
	GLfloat xn = 2 * pt.x() / GLfloat(widgetSize.width()) - 1;
	GLfloat yn = 2 * (widgetSize.height() - pt.y()) / GLfloat(widgetSize.height()) - 1;
	QVector4D r = m * QVector4D(xn, yn, 0, 1);
	int x = int((r.x() + 1) * _imageSize.width() / 2);
	int y = int((1 - r.y()) * _imageSize.height() / 2);
	QPoint result(x, y);
	return std::move(result);
}

QSize RenderWidget::toImageC(const QSize & sz) 
{
	if(!_hasImage )
		return sz;

		QMatrix4x4 m = getModelViewProjMatrix().inverted();
	QSize widgetSize = size();
	GLfloat xn = 2 * sz.width() / GLfloat(widgetSize.width()) - 1;
	GLfloat yn = 2 * (widgetSize.height() - sz.height()) / GLfloat(widgetSize.height()) - 1;
	QVector4D r0 = m * QVector4D(-1, 1, 0, 1);
	QVector4D r1 = m * QVector4D(xn, yn, 0, 1);
	int x0 = int((r0.x() + 1) * _imageSize.width() / 2);
	int y0 = int((1 - r0.y()) * _imageSize.height() / 2);
	int x1 = int((r1.x() + 1) * _imageSize.width() / 2);
	int y1 = int((1 - r1.y()) * _imageSize.height() / 2);
	QSize result(x1 - x0, y1 - y0);
	return std::move(result);
}

void RenderWidget::initializeGL()
{
	initializeOpenGLFunctions();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	GLint dims[2];
	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, dims);

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

void RenderWidget::paintGL()
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

	// TODO: replace it with modern approach
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(m.data());

	glLineWidth(3.0);
	glColor3d(1, 0, 0);

	for (QRect rc : _boxes) 
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
	}
	// 

	/*
	glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

	painter.endNativePainting();
	painter.fillRect(10,10, 100, 100, QColor(Qt::darkBlue));
	*/
}


QMatrix4x4 RenderWidget::getModelViewProjMatrix(void) 
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


void RenderWidget::updateLayers()
{
	if(!_hasImage )
		return;

	auto geom = _triangleCross->geometry();
	auto rc = _triangleCross->rect();

	_triangleCross->move(_triangleCross->geometry().topLeft() * _imageScale);
}

void RenderWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
}

void RenderWidget::updateHScroll( int hVal)
{
	if(!_hasImage )
		return;

	update();
}

void RenderWidget::updateVScroll( int vVal)
{
	if(!_hasImage )
		return;

	update();
}

void RenderWidget::updateScroll() 
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
	updateLayers();
}

void RenderWidget::mouseMoveEvent(QMouseEvent* event)
{
	if(!_hasImage )
		return;

	auto pos = toImageC(event->pos());
	QSize sentSize;

	if (_rubberBand && _rubberBand->isVisible())
	{
		_rubberBand->setGeometry(QRect(_origin, event->pos()).normalized());
		sentSize = _rubberBand->geometry().size();
		sentSize = sentSize / _imageScale;
		pos = toImageC(_rubberBand->geometry().topLeft()) ;
	}
	emit cursorPos(pos, sentSize);
	//emit cursorPos(event->pos() / _imageScale, sentSize / _imageScale);
}

void RenderWidget::mousePressEvent(QMouseEvent *event)
{
	if(!_hasImage )
		return;
	
	_origin = event->pos();

    if (!_rubberBand)
        _rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    _rubberBand->setGeometry(QRect(_origin, QSize()));
    _rubberBand->show();
}

void RenderWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if(!_hasImage )
		return;
    _rubberBand->hide();
}


bool RenderWidget::eventFilter(QObject* obj, QEvent* event)
{
	if(!_hasImage )
		return QObject::eventFilter(obj, event);

	if (event->type() == QEvent::Resize) 
	{
		auto newSize = size();
		qDebug() << " **** SIZE : " << newSize;
		if (_imageRatio < 1 ) // vertical
		{
			_imageScale = _imageScale * static_cast<float>(newSize.height()) / static_cast<float>(_startWidgetSize.height());
		}
		else
		{
			_imageScale = _imageScale * static_cast<float>(newSize.width()) / static_cast<float>(_startWidgetSize.width());
		}
		_startWidgetSize = newSize;
		emit scaleChanged(_glScale, _imageScale);
		updateScroll();
	}
	return QObject::eventFilter(obj, event);
}


void RenderWidget::wheelEvent(QWheelEvent* event)
{
	if(!_hasImage )
		return;

	if(event->modifiers().testFlag(Qt::ControlModifier))
	{
     	event->angleDelta().y() < 0 ? zoomOut() : zoomIn();
	}
	else
	{
		auto const val = event->angleDelta().y() < 0 ? _verticalScrollbar->singleStep() : -_verticalScrollbar->singleStep();
		_verticalScrollbar->setValue(_verticalScrollbar->value() + val) ;
		update();
	}
}
