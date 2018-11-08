#include "roiParamWidget.h"
#include <QToolTip>
#include <QComboBox>
#include <QAction>
#include <QLineEdit>


using namespace LandaJune;
using namespace Parameters;

roiParamWidget::roiParamWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	_controlBox = ui.controlBox;

	connect ( ui.applyButt, &QPushButton::clicked, this, &roiParamWidget::onApply );
	connect ( ui.cancelButt, &QPushButton::clicked, this, &roiParamWidget::onCancel );
}

roiParamWidget::~roiParamWidget()
{
}

void roiParamWidget::enableControls(bool bEnable)
{
	setEnabled(bEnable);
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

void roiParamWidget::setProcessParameters(LandaJune::ProcessParametersPtr params)
{
	_params = params;
	connect(_params.get(), &BaseParameters::updateCalculated, this, &roiParamWidget::onParamsUpdated );
}


QWidget* roiParamWidget::addControl(QString strParamName, QString labelText, bool bSwitchableLabel, bool bEditable, QString strToolTip)
{
	QWidget* retVal = nullptr;
	if (!_params )
		return retVal;

	QVariant varVal = _params->getParamProperty(strParamName);
	if (varVal.isNull())
		return retVal;

	if (bEditable)
	{
		if ( varVal.type() == QVariant::Double  )
		{
			retVal = addDoubleSpinBox( varVal.toDouble(), labelText, strParamName, bSwitchableLabel);
		}
	}
	else
	{
		retVal = addReadOnlyEdit( varVal, labelText, strParamName);
	}

	if ( retVal && !strToolTip.isEmpty())
		retVal->setToolTip(strToolTip);
	
	return retVal;
}

QDoubleSpinBox * roiParamWidget::addDoubleSpinBox(double currentValue, QString labelText, QString propertyName, bool bSwitchableLabel)
{
	QDoubleSpinBox * targetWidget = nullptr;

	targetWidget = new QDoubleSpinBox(this);
	targetWidget->setMinimum(-DBL_MAX);
	targetWidget->setMaximum(DBL_MAX);
	targetWidget->setValue(currentValue);
	targetWidget->setSingleStep(0.1);
	targetWidget->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	targetWidget->setMinimumHeight(_controlHeight);
	targetWidget->setMaximumHeight(_controlHeight);
	targetWidget->setProperty("ownerProperty", propertyName);
	connect(targetWidget, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &roiParamWidget::onDoubleSpinnerValChanged);

	if ( !labelText.isEmpty())
	{
		if ( bSwitchableLabel )
		{
			const auto unitLabel = new unitSwitchLabel(_controlBox, labelText, unitSwitchLabel::MM);
			_switchLabelList.push_back(unitLabel);
			addWidget (unitLabel);
		}
		else
		{
			addWidget (new QLabel(labelText, _controlBox));
		}
	}
	
	_spinBoxMap[propertyName] = targetWidget;
	
	addWidget(targetWidget);
	return targetWidget;
}

QComboBox* roiParamWidget::addComboBox(QString labelText)
{
	QComboBox * targetWidget = new QComboBox(this);
	targetWidget->setMinimumHeight(_controlHeight);
	targetWidget->setMaximumHeight(_controlHeight);

	if ( !labelText.isEmpty())
	{
		addWidget (new QLabel(labelText, _controlBox));
	}
	
	addWidget(targetWidget);
	return targetWidget;
}

QCheckBox* roiParamWidget::addCheckBox(QString labelText)
{
	QCheckBox * targetWidget = new QCheckBox(this);
	targetWidget->setMinimumHeight(_controlHeight);
	targetWidget->setMaximumHeight(_controlHeight);

	if ( !labelText.isEmpty())
	{
		targetWidget->setText(labelText);
	}
	
	addWidget(targetWidget);
	return targetWidget;
}

QLineEdit * roiParamWidget::addReadOnlyEdit( QVariant value, QString labelText, QString propertyName )
{
	QLineEdit * targetWidget = new QLineEdit(this);
	targetWidget->setMinimumHeight(_controlHeight);
	targetWidget->setMaximumHeight(_controlHeight);
	targetWidget->setProperty("ownerProperty", propertyName);

	if ( !labelText.isEmpty())
	{
		addWidget (new QLabel(labelText, _controlBox));
	}

	targetWidget->setReadOnly(true);
	targetWidget->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	_readonlyEditMap[propertyName] = targetWidget;
	
	targetWidget->setText(getValueString(value));

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

void roiParamWidget::onApply()
{
	emit done(true);
}

void roiParamWidget::onCancel()
{
	emit done(false);
}

void roiParamWidget::onParamsUpdated()
{
	for ( auto ctrl : _spinBoxMap )
	{
		auto const ownProp = ctrl->property("ownerProperty").toString();
		auto valVar = _params->getParamProperty(ownProp);
		if (!valVar.isNull())
		{
			disconnect(ctrl, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &roiParamWidget::onDoubleSpinnerValChanged);
			ctrl->setValue(valVar.toDouble());
			connect(ctrl, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &roiParamWidget::onDoubleSpinnerValChanged);
		}
	}

	for ( auto ctrl : _readonlyEditMap )
	{
		auto const ownProp = ctrl->property("ownerProperty").toString();
		auto valVar = _params->getParamProperty(ownProp);
		ctrl->setText(getValueString(valVar));
	}
}

QString roiParamWidget::getValueString (const QVariant& varVal) const
{
	QString valtext;
	switch( varVal.type() )
	{
		case QVariant::Double :	valtext = QString::number(varVal.toDouble(), 'g', 3); break;
		case QVariant::Int :	valtext = QString::number(varVal.toInt()); break;
		case QVariant::String :	valtext = varVal.toString(); break;
		default: ;
	}

	return valtext;
}
