#include "moveableLayerWidget.h"
#include <QPainter>
#include <QMouseEvent>

#include "applog.h"

#define SPOT_RADIUS 9

moveableLayerWidget::moveableLayerWidget(QWidget* parent, CROSS_TYPE crossType, int width, int height, int circleDiameter, QPoint startPt, float initScale)
	: QWidget(parent)
	, _crossType(crossType)
	, _initialScale(initScale)
	, _circleDiameter(circleDiameter)
{
	setMouseTracking(true);
	setCursor(Qt::OpenHandCursor);
	setGeometry(100, 100, width, height);

	//setAttribute(Qt::WA_TranslucentBackground);
	//setAttribute(Qt::WA_NoSystemBackground);
			
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


QPoint moveableLayerWidget::getCenterPoint() const
{
	return getCenterPoint(mapToParent(geometry().topLeft()));
}

QPoint moveableLayerWidget::getCenterPoint(QPoint topLeft) const
{
	return QPoint(topLeft.x() + rect().width() / 2, topLeft.y() + rect().height() / 2);
}

void moveableLayerWidget::paintI2SCross(QPainter& painter)
{
	//auto const ratio =  static_cast<double>(rect().width()) / static_cast<double>(_originalSize.width());

	const auto leftOffset = (_originalSize.width() / 4 ) * _scaleRatio;
	const auto topOffset =  (_originalSize.height() / 4 ) * _scaleRatio;

	int penWidth = _scaleRatio * 2;
	/*
	if ( _scaleRatio > 1.5 && _scaleRatio < 3 )
	{
		penWidth = 2;
	}
	else if  ( _scaleRatio > 3 )
	{
		penWidth = _scaleRatio * 2;
	}
	*/
	//painter.fillRect(rect(), QColor(0,0,255, 120));
	QPen pen ((QBrush
			(QColor(255,50,0,255))
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

	//painter.fillRect(rect(), QColor(0,0,255,128));

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
	const auto leftOffset = (static_cast<double>(_originalSize.width()) / 4 ) * _scaleRatio;
	const auto topOffset =  (static_cast<double>(_originalSize.height()) / 4 ) * _scaleRatio;

	const int penWidth = _scaleRatio * 2;

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

	//painter.fillRect(rect(), QColor(0,0,255,128));
	painter.drawEllipse( { static_cast<double>(rect().width()) / 2, static_cast<double>(rect().height()) / 2 }, _circleDiameter / 2 * _scaleRatio, _circleDiameter / 2 * _scaleRatio );
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
		emit crossMoving(getCenterPoint(geometry().topLeft()));
	}
	else
	{
		movingOver(event->globalPos());
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
