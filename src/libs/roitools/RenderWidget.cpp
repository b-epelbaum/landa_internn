#include "RenderWidget.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include <QMouseEvent>
#include <QScrollBar>

#ifdef _DEBUG
#pragma comment(lib, "opencv_world342d.lib")
#else
#pragma comment(lib, "opencv_world342.lib")
#endif

RenderWidget::RenderWidget(QWidget *parent)
	: QOpenGLWidget(parent)
	, _scale(1.0f)
	, _texture(nullptr)
	, _program(nullptr)
	, _imageSize(1, 1)
{
	setMouseTracking(true);
	setCursor(Qt::CrossCursor);
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

void RenderWidget::zoomIn()
{
	QRect curRect = geometry();
	auto w = curRect.width() * 1.25;
	auto h = curRect.height() * 1.25;

	setMinimumSize(w, h);
	setMaximumSize(w, h);
	
	//_scale = std::min(20.0, _scale + 0.5); 
	//update(); 
}
	
void RenderWidget::zoomOut()
{
	_scale = std::max(0.5, _scale - 0.5); 
	update();
}


void RenderWidget::setImage(const QString& file)
{
	makeCurrent();
	QImage img;
	if (img.load(file)) 
	{
		if (_texture)
		{
			delete _texture;
		}
		_texture = new QOpenGLTexture(img);
		_imageSize = img.size();
	}

	auto imgRatio = (double)_imageSize.width() / (double)_imageSize.height();
	QRect parentRect = parentWidget()->geometry();
	QRect thisRect { 0,0,0,0};

	if ( imgRatio < 1 ) // vertical image
	{
		thisRect.setWidth(parentRect.height() *imgRatio );
		thisRect.setHeight(parentRect.height());
	}

	setMinimumSize(thisRect.width(), thisRect.height());
	setMaximumSize(thisRect.width(), thisRect.height());

	doneCurrent();
	repaint();
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

	ratioX *= _scale; 
	ratioY *= _scale;

	GLfloat offsetY = 0;
	if (_horz && ratioY > 1.0f) 
	{
		offsetY = (2 * _horz->value() / GLfloat(_horz->maximum()) - 1) * (ratioY - 1.0f);
	}
	GLfloat offsetX = 0;
	if (_vert && ratioX > 1.0f) 
	{
		offsetX = (2 * _vert->value() / GLfloat(_vert->maximum()) - 1) * (ratioX - 1.0f);
	}

	QMatrix4x4 m;
	m.ortho(-1, 1, -1, 1, -1, 1);
	m.translate(-offsetX, offsetY, 0.0f);
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
}

void RenderWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
}

void RenderWidget::update() 
{
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

	ratioX *= _scale; ratioY *= _scale;

	if (_horz) 
	{
		int total = _imageSize.width();
		if (ratioY > 1.0f) 
		{
			int page = int(total / _scale);
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

	repaint();
}

void RenderWidget::mouseMoveEvent(QMouseEvent* event)
{
	emit cursorPos(event->pos());
	if (_rubberBand && _rubberBand->isVisible())
		_rubberBand->setGeometry(QRect(_origin, event->pos()).normalized());
}

void RenderWidget::mousePressEvent(QMouseEvent *event)
{
    _origin = event->pos();
    if (!_rubberBand)
        _rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    _rubberBand->setGeometry(QRect(_origin, QSize()));
    _rubberBand->show();
}

void RenderWidget::mouseReleaseEvent(QMouseEvent *event)
{
    _rubberBand->hide();
    // determine selection, for example using QRect::intersects()
    // and QRect::contains().
}

void RenderWidget::wheelEvent(QWheelEvent* event)
{
	event->angleDelta().y() < 0 ? zoomOut() : zoomIn();
}