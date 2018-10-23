#pragma once

#include <QWidget>

class moveableLayerWidget : public QWidget
{
	Q_OBJECT

public:
	moveableLayerWidget(QWidget *parent);
	~moveableLayerWidget();

protected:

	void paintEvent(QPaintEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	QPoint _dragPosition { -1, -1 };
};
