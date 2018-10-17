#include "RenderWidget.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include <QMouseEvent>

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
{
	setMouseTracking(true);
	setCursor(Qt::CrossCursor);

	//QWidget * frame = new QWidget(this);
	//frame->setMaximumSize(200,200);
	//frame->setMinimumSize(200,200);

	//frame->setStyleSheet("QWidget { background-color : red; }");
	//frame->setGeometry(20,20, 200, 200);
	//frame->raise();
	//frame->show();
}

RenderWidget::~RenderWidget()
{
	makeCurrent();
	if (_vbo.bufferId()) { _vbo.destroy(); }
	if (_texture) { delete _texture; }
	if (_program) { delete _program; }
	doneCurrent();
}

void RenderWidget::ZoomIn()
{
	//setMaximumSize(maximumSize() * 2);
	//setMinimumSize(minimumSize() * 2);

	if (_zoomFactor > 4.74 )
		return;

	_zoomFactor += 0.25;
	setMaximumSize(_imageSize * _zoomFactor);
	setMinimumSize(_imageSize * _zoomFactor);
}

void RenderWidget::ZoomOut()
{
	if (_zoomFactor <= 0.25 )
		return;

	_zoomFactor -= 0.25;
	setMaximumSize(_imageSize * _zoomFactor);
	setMinimumSize(_imageSize * _zoomFactor);
}

void RenderWidget::SetImage(const QString& file)
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
		setMaximumSize(_imageSize);
		setMinimumSize(_imageSize);
		_zoomFactor = 1;
	}
	doneCurrent();
	repaint();
}

void RenderWidget::initializeGL()
{
	initializeOpenGLFunctions();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

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

	auto vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
	const auto vsrc =
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

	auto* fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
	const auto fsrc =
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
	glClearColor(0, 0.7, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	QMatrix4x4 m;
	m.ortho(-1, 1, -1, 1, -1, 1);
	m.scale(_scale, _scale, 1.0f);

	_vbo.bind();
	_program->bind();
	_program->setUniformValue("matrix", m);
	_program->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
	_program->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
	_program->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
	_program->setAttributeBuffer(PROGRAM_TEXCOORD_ATTRIBUTE, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));

	if (_texture) {
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

void RenderWidget::update() {
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