#include "imagescrollarea.h"
#include <QWheelEvent>
#include <QDebug>
#include "roiRenderWidget.h"

imageScrollArea::imageScrollArea(QWidget *parent)
	: QScrollArea(parent)
{
}

imageScrollArea::~imageScrollArea()
{
}

void imageScrollArea::wheelEvent(QWheelEvent* event)
{
}
