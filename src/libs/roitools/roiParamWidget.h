#pragma once

#include <QWidget>
#include "unitSwitchLabel.h"
#include "ui_roiParamWidget.h"
#include <QDoubleSpinBox>
#include "ProcessParameters.h"


class roiParamWidget : public QWidget
{
	Q_OBJECT

public:
	roiParamWidget(QWidget *parent = Q_NULLPTR);
	~roiParamWidget();

	void clear() const;

	void setProcessParameters (LandaJune::ProcessParametersPtr params) { _params = params; }
	void setControlsHeight ( int controlHeight ) { _controlHeight = controlHeight; }
	int getControlsHeight() const { return _controlHeight; }

	void setControlsTextAlignment ( Qt::Alignment align ) { _textAlignH = align; }
	Qt::Alignment getControlsTextAlignment () const { return _textAlignH; }

	QWidget * addControl( QString strParamName, QString labelText, bool bSwitchableLabel );
	QDoubleSpinBox * addDoubleSpinBox(double currentValue, QString labelText, QString propertyName, bool bSwitchableLabel);
	QComboBox * addComboBox(QString labelText);

	void addSpacer( int w, int h) const
	{
		const auto spacwr = new QSpacerItem(w,h, QSizePolicy::Ignored, QSizePolicy::Maximum); _controlBox->layout()->addItem(spacwr); 
	}
	
	void addFinalSpacer() const
	{
		const auto spacwr = new QSpacerItem(10,100, QSizePolicy::Ignored, QSizePolicy::Expanding ); _controlBox->layout()->addItem(spacwr); 
	}

	void addWidget(QWidget * widget) const;

	unitSwitchLabel::LABEL_UNITS currentUnits () const { return _currentUnits; }
	void switchUnits (unitSwitchLabel::LABEL_UNITS units);

signals:

	void unitsChanged (unitSwitchLabel::LABEL_UNITS oldUnits, unitSwitchLabel::LABEL_UNITS newUnits );
	void propertyChanged( QString propertyName, QVariant newVal );

private slots:

	void onDoubleSpinnerValChanged( double newValue);

private:

	void translateUnits(QDoubleSpinBox* spinBox, unitSwitchLabel::LABEL_UNITS oldUnits, unitSwitchLabel::LABEL_UNITS newUnits );

	Ui::roiParamWidget ui;
	QList<unitSwitchLabel*> _switchLabelList;
	Qt::Alignment _textAlignH = Qt::AlignHCenter;
	int _controlHeight = 30;

	unitSwitchLabel::LABEL_UNITS _currentUnits = unitSwitchLabel::MM;
	QWidget * _controlBox = nullptr;
	QMap<QString, QDoubleSpinBox*> _spinBoxMap;

	LandaJune::ProcessParametersPtr _params;
};
