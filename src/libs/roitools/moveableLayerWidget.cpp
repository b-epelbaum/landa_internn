#include "moveableLayerWidget.h"
#include <QPainter>
#include <QMouseEvent>

#include "applog.h"

#define SPOT_RADIUS 9

moveableLayerWidget::moveableLayerWidget(QWidget* parent
			, CROSS_TYPE crossType
			, int width
			, int height
			, int circleDiameter
			, QPoint startPt
			, QColor lineColor
			, Qt::PenStyle penStyle
			, float initScale)
	: QWidget(parent)
	, _initialScale(initScale)
	, _crossType(crossType)
	, _circleDiameter(circleDiameter)
	, _lineColor(lineColor)
	, _penStyle(penStyle)
{
	setMouseTracking(true);

	if (crossType == CROSS_EDGE )
	{
		setCursor(Qt::SplitHCursor);
		setGeometry(0, 0, width, height);
	}
	else
	{
		setCursor(Qt::OpenHandCursor);
		setGeometry(100, 100, width, height);
	}

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
= default;


QPoint moveableLayerWidget::getCenterPoint() const
{
	return getCenterPoint(mapToParent(geometry().topLeft()));
}

QPoint moveableLayerWidget::getCenterPoint(QPoint topLeft) const
{
	return QPoint(topLeft.x() + rect().width() / 2, topLeft.y() + rect().height() / 2);
}

void moveableLayerWidget::paintI2SCross(QPainter& painter) const
{
	//auto const ratio =  static_cast<double>(rect().width()) / static_cast<double>(_originalSize.width());

	const auto leftOffset = (static_cast<float>(_originalSize.width()) / 4 ) * _scaleRatio;
	const auto topOffset =  (static_cast<float>(_originalSize.height()) / 4 ) * _scaleRatio;

	const auto penWidth = _scaleRatio * 2;
	QPen pen ((QBrush
			(_lineColor)
		)
		, penWidth
	);
	pen.setStyle(_penStyle);
	painter.setPen (pen);

	painter.drawLine(
			QPointF(static_cast<float>(leftOffset), static_cast<float>(rect().height()) / 2), 
			QPointF(static_cast<float>(rect().right()) - leftOffset, static_cast<float>(rect().height()) / 2)
	);

	painter.drawLine(
				QPointF(static_cast<float>(rect().width()) / 2, static_cast<float>(topOffset)), 
				QPointF(static_cast<float>(rect().width()) / 2, static_cast<float>(rect().bottom()) - topOffset)
		);

	pen.setStyle(Qt::DotLine);
	painter.setPen (pen);

	//painter.fillRect(rect(), QColor(0,0,255,128));

	painter.drawLine(
			QPointF(0, static_cast<float>(rect().height()) / 2), 
			QPointF(static_cast<float>(leftOffset), static_cast<float>(rect().height()) / 2)
	);


	painter.drawLine(
				QPointF(static_cast<float>(rect().width()) / 2, 0), 
				QPointF(static_cast<float>(rect().width()) / 2, static_cast<float>(topOffset))
	);
}


void moveableLayerWidget::paintC2CCross(QPainter& painter) const
{
	const auto leftOffset = static_cast<float>(_originalSize.width()) / 4 * _scaleRatio;
	const auto topOffset =  static_cast<float>(_originalSize.height()) / 4 * _scaleRatio;

	const auto penWidth = _scaleRatio * 2;

	QPen pen ((QBrush
			(_lineColor)
		)
		, penWidth
	);
	pen.setStyle(_penStyle);

	painter.setPen (pen);

	painter.drawLine(
			QPointF(leftOffset, static_cast<float>(rect().height()) / 2), 
			QPointF(static_cast<float>(rect().right()) - leftOffset, static_cast<float>(rect().height()) / 2)
	);

	painter.drawLine(
				QPointF(static_cast<float>(rect().width()) / 2, static_cast<float>(topOffset)), 
				QPointF(static_cast<float>(rect().width()) / 2, static_cast<float>(rect().bottom()) - static_cast<float>(topOffset))
	);

	//painter.fillRect(rect(), QColor(0,0,255,128));
	painter.drawEllipse( 
						{ 
							static_cast<float>(rect().width()) / 2
						  , static_cast<float>(rect().height()) / 2 
						}, 
						static_cast<float>(_circleDiameter) / 2 * _scaleRatio, 
						static_cast<float>(_circleDiameter) / 2 * _scaleRatio );
}

void moveableLayerWidget::paintEdgeCross(QPainter& painter) const
{
	const auto leftOffset = static_cast<float>(_originalSize.width()) / static_cast<float>(2) * _scaleRatio;
	const auto imHeight =  _originalSize.height() * _scaleRatio;

	const auto penWidth = _scaleRatio * 2;
	
	//painter.fillRect(rect(), QColor(0,0,255, 120));
	QPen pen ((QBrush
			(_lineColor)
		)
		, penWidth
	);
	pen.setStyle(_penStyle);

	painter.setPen (pen);
	painter.drawLine(
			QPointF( leftOffset - penWidth / 2, 0 ), 
			QPointF( leftOffset - penWidth / 2, imHeight )
	);
		
	//painter.fillRect(rect(), QColor(0,0,255,128));

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
	else if (_crossType == CROSS_EDGE )
	{
		paintEdgeCross(painter);
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
		if (_crossType == CROSS_EDGE )
		{
			_lastDropPosition.setY(0);
		}
		else if (_crossType == CROSS_SPOT_OTHER && !_canChangeX )
		{
			_lastDropPosition.setX(_staticROIXValue);
		}
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
		_staticROIXValue = frameGeometry().topLeft().x();
		_dragPosition = event->globalPos() - frameGeometry().topLeft();
	}
	if (_crossType != CROSS_EDGE )
	{
		setCursor(Qt::BlankCursor);
	}
	event->accept();
}

void moveableLayerWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		_dragPosition = QPoint(-1, -1);
		event->accept();

		if (_crossType == CROSS_EDGE )
		{
			setCursor(Qt::SplitHCursor);
		}
		else
			setCursor(Qt::OpenHandCursor);

		_topLeftOnActualImage = geometry().topLeft();
		
		emit crossMoved(geometry().topLeft(), getCenterPoint(geometry().topLeft()));
	}

}
