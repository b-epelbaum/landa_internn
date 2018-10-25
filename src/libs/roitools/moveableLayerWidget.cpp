#include "moveableLayerWidget.h"
#include <QPainter>
#include <QMouseEvent>

moveableLayerWidget::moveableLayerWidget(QWidget *parent)
	: QWidget(parent)
{
	//setAttribute(Qt::WA_TranslucentBackground);
	//setAttribute ( Qt::WA_NoSystemBackground );
	setCursor(Qt::OpenHandCursor);
}

moveableLayerWidget::~moveableLayerWidget()
{
}


QPoint moveableLayerWidget::getCenterPoint(QPoint topLeft) const
{
	return QPoint(topLeft.x() + rect().width() / 2, topLeft.y() + rect().height() / 2);
}

void moveableLayerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter (this);
	QPen pen ((QBrush
			(QColor(255,0,0,255))
		)
		, 3
	);

	painter.setPen (pen);

	painter.drawLine(QPoint(0, rect().height() / 2), QPoint(rect().right(), rect().height() / 2));
	painter.drawLine(QPoint(rect().width() / 2, 0), QPoint(rect().width() / 2, rect().bottom()));
}


void moveableLayerWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::LeftButton && _dragPosition.x() != -1 )
	{
		_lastDropPosition = event->globalPos() - _dragPosition;
		move(_lastDropPosition);
		event->accept();
	}
}

void moveableLayerWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		_dragPosition = event->globalPos() - frameGeometry().topLeft();
	}
	setCursor(Qt::BlankCursor);
	event->accept();
}

void moveableLayerWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		_dragPosition = QPoint(-1, -1);
		event->accept();
		setCursor(Qt::BlankCursor);
		setCursor(Qt::OpenHandCursor);
		emit dragStopped(geometry().topLeft(), getCenterPoint(geometry().topLeft()));
	}

}
