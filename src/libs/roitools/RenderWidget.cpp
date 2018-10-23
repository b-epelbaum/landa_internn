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

	auto roiRect = new roiRectWidget(this);
	roiRect->setGeometry(0, 0, 100, 100);
	roiRect->raise();
	roiRect->show();
	_roiRectArray.push_back(roiRect);


	_triangleCross = new moveableLayerWidget(this);
	_triangleCross->setGeometry(0, 110, 100, 100);
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
	_horz = horz; _vert = vert;

	auto hMin = _horz->minimum();
	auto hMax = _horz->maximum();
}

void RenderWidget::setScales(float glScale, float imageScale)
{
	if (_hasImage)
	{
		_imageScale = imageScale;
		_glScale = glScale;
		updateScroll();
	}
}

void RenderWidget::setScrolls(int hScrollVal, int vScrollVal)
{
	if (_hasImage)
	{
		if ( hScrollVal != _horz->value() )
			_horz->setValue(hScrollVal);
		if ( vScrollVal != _vert->value() )
			_vert->setValue(vScrollVal);
		update();
	}
}

void RenderWidget::showActualPixels ()
{
	
}
	
void RenderWidget::showFitOnScreen ()
{
	
}
	
void RenderWidget::setZoom ( int zoom  )
{
	
}

void RenderWidget::zoomIn()
{
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
	doneCurrent();
	repaint();

	emit scaleChanged(_glScale, _imageScale);
	return true;
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
	if (_horz && ratioY > 1.0f) 
	{
		_offsetY = (2 * _horz->value() / GLfloat(_horz->maximum()) - 1) * (ratioY - 1.0f);
	}
	_offsetX = 0;
	if (_vert && ratioX > 1.0f) 
	{
		_offsetX = (2 * _vert->value() / GLfloat(_vert->maximum()) - 1) * (ratioX - 1.0f);
	}
	
	

	QMatrix4x4 m;
	m.ortho(-1, 1, -1, 1, -1, 1);
	m.translate(-_offsetX, _offsetY, 0.0f);
	m.scale(ratioX, ratioY, 1.0f);


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
	glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

	painter.endNativePainting();

	painter.fillRect(10,10, 100, 100, QColor(Qt::darkBlue));

	_drawnImageLeft = _vert->value() * _imageSize.width() / (_vert->maximum() + _vert->pageStep());
	_drawnImageWidth = _vert->pageStep() * _imageSize.width() / (_vert->maximum() + _vert->pageStep());

	qDebug() << " --- PIC LEFT : " << _drawnImageLeft;
	qDebug() << " --- PIC WIDTH : " << _drawnImageWidth;
}

void RenderWidget::updateLayers()
{
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
	for ( const auto  rcWidget : _roiRectArray )
	{
		rcWidget->move(-hVal, 0);
	}
	update();
	//updateScroll();
}

void RenderWidget::updateVScroll( int vVal)
{
	for ( const auto  rcWidget : _roiRectArray )
	{
		rcWidget->move(0, -vVal * _horz->pageStep());
	}
	update();
	//updateScroll();
}

void RenderWidget::updateScroll() 
{
	if(!_hasImage )
	{
		_horz->setEnabled(false);
		_vert->setEnabled(false);
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

	if (_horz) 
	{
		int total = _imageSize.width();
		if (ratioY > 1.0f) 
		{
			int page = int(total / _glScale);
			_horz->setMinimum(0);
			_horz->setMaximum(total - page);
			_horz->setPageStep(page);
			_horz->setEnabled(true);
		}
		else 
		{
			_horz->setMinimum(0);
			_horz->setMaximum(total);
			_horz->setPageStep(total);
			_horz->setEnabled(false);
		}
	}

	if (_vert) 
	{
		int total = _imageSize.height();
		if (ratioX > 1.0f) 
		{
			int page = int(total / ratioX);
			_vert->setMinimum(0);
			_vert->setMaximum(total - page);
			_vert->setPageStep(page);
			_vert->setEnabled(true);
		}
		else 
		{
			_vert->setMinimum(0);
			_vert->setMaximum(total);
			_vert->setPageStep(total);
			_vert->setEnabled(false);
		}
	}
	update();
	updateLayers();
}

void RenderWidget::mouseMoveEvent(QMouseEvent* event)
{
	if(!_hasImage )
		return;

	QSize sentSize;
	
	if (_rubberBand && _rubberBand->isVisible())
	{
		_rubberBand->setGeometry(QRect(_origin, event->pos()).normalized());
		sentSize = _rubberBand->geometry().size();
	}

	emit cursorPos(event->pos() / _imageScale, sentSize / _imageScale);
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
    // determine selection, for example using QRect::intersects()
    // and QRect::contains().
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
			auto heightRatio = (float)newSize.height() / (float)_startWidgetSize.height();
			_imageScale = _imageScale * heightRatio;
			_startWidgetSize = newSize;
		}
	}
	return QObject::eventFilter(obj, event);
}


void RenderWidget::wheelEvent(QWheelEvent* event)
{
	if(!_hasImage )
		return;

	event->angleDelta().y() < 0 ? zoomOut() : zoomIn();
}
