#include "imagescrollarea.h"
#include <QWheelEvent>
#include <QDebug>
#include "RenderWidget.h"

imageScrollArea::imageScrollArea(QWidget *parent)
	: QScrollArea(parent)
{
}

imageScrollArea::~imageScrollArea()
{
}

void imageScrollArea::wheelEvent(QWheelEvent* event)
{
	event->angleDelta().y() < 0 ? zoomOut() : zoomIn();



	//if ( event->modifiers() & Qt::ControlModifier )
	//	event->accept();
	//else
		event->accept();
}

void imageScrollArea::zoomOut()
{
	if (_widget == nullptr )
		_widget = static_cast<RenderWidget*>(widget());

	_widget->ZoomOut();
}

void imageScrollArea::zoomIn()
{
	if (_widget == nullptr )
		_widget = static_cast<RenderWidget*>(widget());

	_widget->ZoomIn();
}
