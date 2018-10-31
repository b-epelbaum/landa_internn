#include "roiParamWidget.h"
#include <QToolTip>
#include <QComboBox>

roiParamWidget::roiParamWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	_controlBox = ui.controlBox;

	connect (ui.unitsCombo, qOverload<int>(&QComboBox::currentIndexChanged), this
			, [this](int iIndex)
			{
				switchUnits(static_cast<unitSwitchLabel::LABEL_UNITS>(iIndex) );
			}
	);
}

roiParamWidget::~roiParamWidget()
{
}

void roiParamWidget::clear() const
{
	qDeleteAll(ui.controlBox->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly));
	const auto layout = ui.controlBox->layout();
	for (auto i = 0; i < layout->count(); ++i) 
	{
		auto layoutItem = layout->itemAt(i);
		if (layoutItem->spacerItem()) 
		{
			layout->removeItem(layoutItem);
            delete layoutItem;
		}
        --i;
    }
}


QWidget* roiParamWidget::addControl(QString strParamName, QString labelText, bool bSwitchableLabel)
{
	if (!_params )
		return nullptr;

	QVariant varVal = _params->getParamProperty(strParamName);
	if (varVal.isNull())
		return nullptr;

	if (varVal.canConvert<double>())
	{
		return addDoubleSpinBox( varVal.toDouble(), labelText, strParamName, bSwitchableLabel);
	}

	return nullptr;
}

QDoubleSpinBox * roiParamWidget::addDoubleSpinBox(double currentValue, QString labelText, QString propertyName, bool bSwitchableLabel)
{
	QDoubleSpinBox * targetWidget = nullptr;

	targetWidget = new QDoubleSpinBox(this);
	targetWidget->setMinimum(-DBL_MAX);
	targetWidget->setMaximum(DBL_MAX);
	targetWidget->setValue(currentValue);
	targetWidget->setSingleStep(1);
	targetWidget->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	targetWidget->setMinimumHeight(_controlHeight);
	targetWidget->setMaximumHeight(_controlHeight);
	targetWidget->setProperty("ownerProperty", propertyName);
	connect(targetWidget, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &roiParamWidget::onDoubleSpinnerValChanged);
	//QToolTip::showText(QCursor::pos(), "<img src=':/icon.png'>Message", this, QRect(), 5000);

	if ( !labelText.isEmpty())
	{
		if ( bSwitchableLabel )
		{
			const auto unitLabel = new unitSwitchLabel(_controlBox, labelText, unitSwitchLabel::MM);
			_switchLabelList.push_back(unitLabel);
			addWidget (unitLabel);
			_spinBoxMap[propertyName] = targetWidget;
		}
		else
		{
			addWidget (new QLabel(labelText, _controlBox));
		}
	}
	addWidget(targetWidget);
	return targetWidget;
}

QComboBox* roiParamWidget::addComboBox(QString labelText)
{
	QComboBox * targetWidget = nullptr;

	targetWidget = new QComboBox(this);
	targetWidget->setMinimumHeight(_controlHeight);
	targetWidget->setMaximumHeight(_controlHeight);

	if ( !labelText.isEmpty())
	{
		addWidget (new QLabel(labelText, _controlBox));
	}
	
	addWidget(targetWidget);
	return targetWidget;
}

void roiParamWidget::addWidget(QWidget * widget) const
{
	widget->setParent(_controlBox);
	_controlBox->layout()->addWidget(widget);
	auto className = widget->metaObject()->className();
}

void roiParamWidget::onDoubleSpinnerValChanged(double newValue)
{
	emit propertyChanged(sender()->property("ownerProperty").toString(), newValue );
}

void roiParamWidget::translateUnits(QDoubleSpinBox* spinBox, unitSwitchLabel::LABEL_UNITS oldUnits,
	unitSwitchLabel::LABEL_UNITS newUnits)
{

}

void roiParamWidget::switchUnits(unitSwitchLabel::LABEL_UNITS units)
{
	_currentUnits = units;
}
