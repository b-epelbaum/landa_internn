#include "roiRectWidget.h"
#include <QPainter>

roiRectWidget::roiRectWidget(QWidget *parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_TransparentForMouseEvents);
	setAttribute ( Qt::WA_NoSystemBackground );

}

roiRectWidget::~roiRectWidget()
{
}

void roiRectWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter (this);
	QPen pen ((QBrush
			(QColor(255,0,0,255))
		)
		, 10.0
	);

	painter.setPen (pen);

	painter.drawRect(rect());
}
