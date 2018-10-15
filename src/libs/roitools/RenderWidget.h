#pragma once

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QRubberBand>
#include <QVector>

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

	void ZoomIn();
	void ZoomOut();
	void SetImage(const QString& file);

signals:

	void cursorPos(QPoint pt);

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

	void update();

	void mouseMoveEvent(QMouseEvent *event) override;
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);

	GLfloat _scale;
	QOpenGLTexture* _texture;
	QOpenGLShaderProgram *_program;
	QOpenGLBuffer _vbo;

	double _zoomFactor = 1.0f;
	QSize _imageSize;

	QPoint _origin;
	QRubberBand * _rubberBand = nullptr;

};