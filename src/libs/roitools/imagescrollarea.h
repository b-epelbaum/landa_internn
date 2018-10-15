#pragma once

#include <QScrollArea>

class RenderWidget;

class imageScrollArea : public QScrollArea
{
	Q_OBJECT

public:
	imageScrollArea(QWidget *parent);
	~imageScrollArea();

 protected:

    void wheelEvent(QWheelEvent* event) override;
	void zoomOut();
	void zoomIn();

	RenderWidget * _widget = nullptr;

};
