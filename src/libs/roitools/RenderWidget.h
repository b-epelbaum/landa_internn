#pragma once

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QRubberBand>
#include <QVector>

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

	void assignScrollBars(QScrollBar *horz, QScrollBar *vert) { _horz = horz; _vert = vert; }

	void zoomIn() ;
	void zoomOut();

	void setImage(const QString& file);
	void update();

signals:

	void cursorPos(QPoint pt);

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

	void wheelEvent(QWheelEvent* event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	GLfloat _scale;
	QOpenGLTexture* _texture;
	QOpenGLShaderProgram *_program;
	QOpenGLBuffer _vbo;

	QSize _imageSize;

	QPoint _origin;
	QRubberBand * _rubberBand = nullptr;

	QScrollBar *_horz = nullptr;
	QScrollBar *_vert = nullptr;

};