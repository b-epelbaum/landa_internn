#pragma once

#include <QWidget>

class roiRectWidget : public QWidget
{
	Q_OBJECT

public:
	roiRectWidget(QWidget *parent);
	~roiRectWidget();

protected:

	void paintEvent(QPaintEvent* event) override;
};
