#include "moveableLayerWidget.h"
#include <QPainter>
#include <QMouseEvent>

#include "applog.h"

#define SPOT_RADIUS 9

moveableLayerWidget::moveableLayerWidget(QWidget* parent, CROSS_TYPE crossType, int width, int height, QPoint startPt)
	: QWidget(parent)
	, _crossType(crossType)
{
	setCursor(Qt::OpenHandCursor);
	setGeometry(100, 100, width, height);

	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_NoSystemBackground);
	//setAttribute(Qt::WA_TransparentForMouseEvents);
		
	move(startPt);
	
	_topLeftOnOriginalImage = startPt;
	_originalSize = geometry().size();

	showCross(true);
}

void moveableLayerWidget::showCross(bool bShow)
{
	if (bShow)
	{
		raise();
		show();
	}
	else
	{
		hide();
	}
}

moveableLayerWidget::~moveableLayerWidget()
{
}


QPoint moveableLayerWidget::getCenterPoint(QPoint topLeft) const
{
	return QPoint(topLeft.x() + rect().width() / 2, topLeft.y() + rect().height() / 2);
}

void moveableLayerWidget::paintI2SCross(QPainter& painter)
{
	auto const ratio =  static_cast<float>(rect().width()) / static_cast<float>(_originalSize.width());
	const auto leftOffset = (_originalSize.width() / 4 ) * ratio;
	const auto topOffset =  (_originalSize.height() / 4 ) * ratio;

	int penWidth = 1;
	if ( ratio > 1.5 && ratio < 3 )
	{
		penWidth = 2;
	}
	else if  ( ratio > 3 )
	{
		penWidth = 3;
	}

	//painter.fillRect(rect(), QColor(0,0,255, 120));
	QPen pen ((QBrush
			(QColor(255,200,0,255))
		)
		, penWidth
	);

	painter.setPen (pen);

	painter.drawLine(
			QPoint(leftOffset, rect().height() / 2), 
			QPoint(rect().right() - leftOffset, rect().height() / 2)
	);

	painter.drawLine(
				QPoint(rect().width() / 2, topOffset), 
				QPoint(rect().width() / 2, rect().bottom() - topOffset)
		);

	pen.setStyle(Qt::DotLine);
	painter.setPen (pen);

	painter.drawLine(
			QPoint(0, rect().height() / 2), 
			QPoint(leftOffset, rect().height() / 2)
	);

	painter.drawLine(
				QPoint(rect().width() / 2, 0), 
				QPoint(rect().width() / 2, topOffset)
	);
}


void moveableLayerWidget::paintC2CCross(QPainter& painter)
{
	auto const ratio =  static_cast<float>(rect().width()) / static_cast<float>(_originalSize.width());

	const auto leftOffset = (static_cast<float>(_originalSize.width()) / 4 ) * ratio;
	const auto topOffset =  (static_cast<float>(_originalSize.height()) / 4 ) * ratio;

	int penWidth = 1;
	if ( ratio > 1.5 && ratio < 3 )
	{
		penWidth = 2;
	}
	else if  ( ratio > 3 )
	{
		penWidth = 3;
	}

	QPen pen ((QBrush
			(QColor(255,0,0,255))
		)
		, penWidth
	);

	painter.setPen (pen);

	painter.drawLine(
			QPoint(leftOffset, rect().height() / 2), 
			QPoint(rect().right() - leftOffset, rect().height() / 2)
	);

	painter.drawLine(
				QPoint(rect().width() / 2, topOffset), 
				QPoint(rect().width() / 2, rect().bottom() - topOffset)
	);

	painter.drawEllipse( { static_cast<float>(rect().width()) / 2, static_cast<float>(rect().height()) / 2 }, SPOT_RADIUS * ratio, SPOT_RADIUS * ratio );
}

void moveableLayerWidget::paintEvent(QPaintEvent* event)
{
	auto const ratio = _originalSize.width() / rect().width();
	QPainter painter (this);

	painter.setRenderHint(QPainter::HighQualityAntialiasing, true);

	if (_crossType == CROSS_I2S )
	{
		paintI2SCross(painter);
	}
	else
	{
		paintC2CCross(painter);
	}
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
		setCursor(Qt::OpenHandCursor);

		_topLeftOnActualImage = geometry().topLeft();
		
		emit crossMoved(geometry().topLeft(), getCenterPoint(geometry().topLeft()));
	}

}
