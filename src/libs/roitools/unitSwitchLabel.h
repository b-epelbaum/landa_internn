#pragma once

#include <QWidget>
#include "ui_unitSwitchLabel.h"

class unitSwitchLabel : public QWidget
{
	Q_OBJECT

public:
	enum LABEL_UNITS { MM, UM, PX, NO_UNITS };

	explicit unitSwitchLabel(QWidget *parent) : QWidget(parent)
	{
		ui.setupUi(this);
	}

	explicit unitSwitchLabel(QWidget *parent, QString strText, LABEL_UNITS labelUnit = MM) : QWidget(parent)
	{
		ui.setupUi(this);
		ui.valueLabel->setText(strText);
	}
	~unitSwitchLabel() = default;

	void setUnits(LABEL_UNITS lUnits)
	{
		ui.unitLabel->setText(LABEL_UNITS_NAMES[lUnits]);
	}

	void setData(QString strName, LABEL_UNITS lUnits)
	{
		ui.valueLabel->setText(strName);
		ui.unitLabel->setText(LABEL_UNITS_NAMES[lUnits]);
	}

private:
	Ui::unitSwitchLabel ui{};

	inline static QString LABEL_UNITS_NAMES [] =
	{
		  " [mm]"
		, " [um]"
		, " [px]"
		, ""
	};
};
