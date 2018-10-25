#pragma once

#include <QWidget>

class moveableLayerWidget : public QWidget
{
	Q_OBJECT

public:
	moveableLayerWidget(QWidget *parent);
	~moveableLayerWidget();

signals:

	 void dragStopped( QPoint topLeft, QPoint center );

protected:

	QPoint getCenterPoint (QPoint topLeft) const;

	void paintEvent(QPaintEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	QPoint _dragPosition { -1, -1 };
	QPoint _lastDropPosition { -1, -1 };
};
