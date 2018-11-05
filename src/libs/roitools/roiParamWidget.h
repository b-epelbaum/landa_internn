#pragma once

#include <QWidget>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>

#include "unitSwitchLabel.h"
#include "ui_roiParamWidget.h"
#include "ProcessParameters.h"

class roiParamWidget : public QWidget
{
	Q_OBJECT

public:
	roiParamWidget(QWidget *parent = Q_NULLPTR);
	~roiParamWidget();

	void enableControls( bool bEnable );
	void clear() const;

	void setProcessParameters (LandaJune::ProcessParametersPtr params) { _params = params; }
	void setControlsHeight ( int controlHeight ) { _controlHeight = controlHeight; }
	int getControlsHeight() const { return _controlHeight; }

	void setControlsTextAlignment ( Qt::Alignment align ) { _textAlignH = align; }
	Qt::Alignment getControlsTextAlignment () const { return _textAlignH; }

	QWidget * addControl( QString strParamName, QString labelText, bool bSwitchableLabel );
	QDoubleSpinBox * addDoubleSpinBox(double currentValue, QString labelText, QString propertyName, bool bSwitchableLabel);
	QComboBox * addComboBox(QString labelText);
	QCheckBox * addCheckBox(QString labelText);

	void addSpacer( int w, int h) const
	{
		const auto spacwr = new QSpacerItem(w,h, QSizePolicy::Ignored, QSizePolicy::Maximum); _controlBox->layout()->addItem(spacwr); 
	}
	
	void addFinalSpacer() const
	{
		const auto spacwr = new QSpacerItem(10,100, QSizePolicy::Ignored, QSizePolicy::Expanding ); _controlBox->layout()->addItem(spacwr); 
	}

	void addWidget(QWidget * widget) const;

signals:

	void propertyChanged( QString propertyName, QVariant newVal );
	void done( bool bApply);

private slots:

	void onDoubleSpinnerValChanged( double newValue);
	void onApply();
	void onCancel();

private:

	Ui::roiParamWidget ui;
	QList<unitSwitchLabel*> _switchLabelList;
	Qt::Alignment _textAlignH = Qt::AlignHCenter;
	int _controlHeight = 30;

	QWidget * _controlBox = nullptr;
	QMap<QString, QDoubleSpinBox*> _spinBoxMap;

	LandaJune::ProcessParametersPtr _params;
};
